#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <GLFW\glfw3.h>
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <algorithm> // For std::sort
#include <time.h>

#define STB_EASY_FONT_IMPLEMENTATION
#include "stb_easy_font.h"
#include <fstream>
#include <set>




using namespace std;

const float DEG2RAD = 3.14159 / 180;

void processInput(GLFWwindow* window);

enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE };
enum ONOFF { ON, OFF };
bool isGameOver = false;
bool spacePressed = false;

// creating paddle x location
float paddleX = 0.0f;

int score = 0;
bool allBricksCleared = false;
bool hitReset = false;

enum GAME_STATE { STATE_MENU, STATE_PLAYING, STATE_HIGHSCORE };
GAME_STATE gameState = STATE_MENU;

int menuIndex = 0;
const vector<string> menuItems = { "Play Game", "High Scores", "Exit" };
bool enterPressedInHighScore = false;
bool enterReleased = true;

class Brick {
public:
	float red, green, blue;
	float x, y, width;
	int hitCount;
	int maxHits;
	ONOFF onoff;

	Brick(float xx, float yy, float ww)
		: x(xx), y(yy), width(ww), hitCount(0), maxHits(3), onoff(ON) {
		updateColor();
	}

	void updateColor() {
		switch (hitCount) {
		case 0: red = 0.0f; green = 1.0f; blue = 0.0f; break; // Green
		case 1: red = 1.0f; green = 1.0f; blue = 0.0f; break; // Yellow
		case 2: red = 1.0f; green = 0.0f; blue = 0.0f; break; // Red
		default: red = green = blue = 0.0f; break;
		}
	}

	

	bool hit() {
		static int bricksBroken = 0;
		hitCount++;

		if (hitReset == true) {
			bricksBroken = 0;
			hitReset = false;
		}

		if (hitCount >= maxHits) {
			onoff = OFF;
			extern int score;
			score += 10;

			bricksBroken++;
			if (bricksBroken >= 3) {
				bricksBroken = 0;
				return true;  // signal to spawn new ball
			}
			return false;
		}
		else {
			updateColor();  // only change color if brick is still active
			return false;
		}
	}



	void drawBrick() {
		if (onoff == ON) {
			double halfside = width / 2;

			// Filled brick
			glColor3d(red, green, blue);
			glBegin(GL_POLYGON);
			glVertex2d(x + halfside, y + halfside);
			glVertex2d(x + halfside, y - halfside);
			glVertex2d(x - halfside, y - halfside);
			glVertex2d(x - halfside, y + halfside);
			glEnd();

			// Outline
			glColor3f(0.0f, 0.0f, 0.0f); // black outline
			glLineWidth(2.0f);          // optional: thicker line
			glBegin(GL_LINE_LOOP);
			glVertex2d(x + halfside, y + halfside);
			glVertex2d(x + halfside, y - halfside);
			glVertex2d(x - halfside, y - halfside);
			glVertex2d(x - halfside, y + halfside);
			glEnd();
		}
	}

};


vector<Brick> bricksBackup; // store original layout
vector<Brick> bricks;

class Circle
{
public:
	bool isWaiting = false; // new ball, waiting to be launched
	float red, green, blue;
	float radius;
	float x;
	float y;
	float speed = 0.03;
	float vx, vy;
	bool hasHitBrickThisFrame = false;
	int collisionCooldown = 0;


	Circle(double xx, double yy, double rad, float velx, float vely, float r, float g, float b)
	{
		x = xx;
		y = yy;
		radius = rad;
		vx = velx;
		vy = vely;
		red = r;
		green = g;
		blue = b;
	}

	void CheckCollisionWithOtherBall(Circle& other) {
		if (this == &other || isWaiting || other.isWaiting) return; // Skip self and waiting balls

		float dx = other.x - x;
		float dy = other.y - y;
		float distSq = dx * dx + dy * dy;
		float radiusSum = radius + other.radius;

		if (distSq < radiusSum * radiusSum) {
			// Normalize the collision vector
			float dist = sqrt(distSq);
			float nx = dx / dist;
			float ny = dy / dist;

			// Relative velocity
			float dvx = vx - other.vx;
			float dvy = vy - other.vy;

			// Dot product of relative velocity and normal vector
			float impactSpeed = dvx * nx + dvy * ny;
			if (impactSpeed > 0) return; // They're moving away

			// Simple elastic collision (same mass)
			float impulse = 2 * impactSpeed / 2;
			vx -= impulse * nx;
			vy -= impulse * ny;
			other.vx += impulse * nx;
			other.vy += impulse * ny;

			// Slight separation to prevent sticking
			float overlap = 0.5f * (radiusSum - dist + 0.001f);
			x -= overlap * nx;
			y -= overlap * ny;
			other.x += overlap * nx;
			other.y += overlap * ny;
		}
	}


	void setFixedSpeed(float targetSpeed = 0.03f) {
		float angle = atan2(vy, vx);

		// Normalize angle to prevent near-horizontal motion
		float minSin = 0.25f; // increase this to force steeper angle (~14.5 degrees)
		if (fabs(sin(angle)) < minSin) {
			angle = (vy >= 0 ? 1 : -1) * asin(minSin);
			angle = (vx < 0 ? M_PI - angle : angle);
		}

		vx = cos(angle) * targetSpeed;
		vy = sin(angle) * targetSpeed;
	}




	bool CheckCollision(Brick* brk) {
		if (collisionCooldown > 0 || brk->onoff == OFF || hasHitBrickThisFrame) {
			return false;
		}

		float halfW = brk->width / 2;
		float halfH = brk->width / 4;

		if ((x + radius > brk->x - halfW && x - radius < brk->x + halfW) &&
			(y + radius > brk->y - halfH && y - radius < brk->y + halfH))
		{
			bool shouldAddBall = brk->hit();

			if (abs(y - brk->y) > abs(x - brk->x)) {
				vy = -vy;
			}
			else {
				vx = -vx;
			}

			collisionCooldown = 5;
			setFixedSpeed();
			hasHitBrickThisFrame = true;
			return shouldAddBall;
		}
		return false;
	}




	int GetRandomDirection(){
		return (rand() % 8) + 1;
	}

	bool isOffBottom = false;

	void MoveOneStep() {
		const int subSteps = 4;
		for (int step = 0; step < subSteps; ++step) {
			x += vx / subSteps;
			y += vy / subSteps;

			if (collisionCooldown > 0)
				collisionCooldown--;

			// Wall collision
			if (x - radius < -1) { x = -1 + radius; vx = fabs(vx); }
			else if (x + radius > 1) { x = 1 - radius; vx = -fabs(vx); }

			if (y + radius > 1) { y = 1 - radius; vy = -fabs(vy); }

			// Paddle collision
			const float paddleY = -0.95f;
			const float paddleHeight = 0.05f;
			const float paddleWidth = 0.3f;

			if (vy < 0 &&
				y - radius <= paddleY + paddleHeight + 0.015f &&
				y - radius >= paddleY - 0.015f &&
				x >= paddleX - paddleWidth / 2 &&
				x <= paddleX + paddleWidth / 2)
			{
				y = paddleY + paddleHeight + radius;
				float angle = (x - paddleX) * 1.5f; // tweak angle
				vx = angle;
				vy = fabs(vy);
			}

			setFixedSpeed(); // Always normalize speed after motion & collisions

			if (y - radius < -1) {
				isOffBottom = true;
				break;
			}
		}
	}




	void DrawCircle()
	{
		glColor3f(red, green, blue);
		glBegin(GL_POLYGON);
		for (int i = 0; i < 360; i++) {
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y);
		}
		glEnd();
	}
};


vector<Circle> world;
void renderCenteredMultilineText(float cx, float cy, float scale, const char* line1, const char* line2) {
	static char buf1[99999], buf2[99999];
	int q1 = stb_easy_font_print(0, 0, (char*)line1, nullptr, buf1, sizeof(buf1));
	int q2 = stb_easy_font_print(0, 0, (char*)line2, nullptr, buf2, sizeof(buf2));

	float unscaledW1 = stb_easy_font_width((char*)line1);
	float unscaledW2 = stb_easy_font_width((char*)line2);
	float w1 = unscaledW1 * scale;
	float w2 = unscaledW2 * scale;
	float hLine = 13.0f * scale;
	float totalHeight = hLine * 2.0f + 5.0f;

	glMatrixMode(GL_PROJECTION);
	glPushMatrix(); glLoadIdentity();
	glOrtho(0, 480, 480, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix(); glLoadIdentity();

	// Line 1
	glPushMatrix();
	glTranslatef(cx - w1 / 2.0f, cy - totalHeight / 2.0f, 0);
	glScalef(scale, scale, 1);
	glColor3f(1, 0, 0);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 16, buf1);
	glDrawArrays(GL_QUADS, 0, q1 * 4);
	glDisableClientState(GL_VERTEX_ARRAY);
	glPopMatrix();

	// Line 2
	glPushMatrix();
	glTranslatef(cx - w2 / 2.0f, cy - totalHeight / 2.0f + hLine + 5, 0);
	glScalef(scale, scale, 1);
	glColor3f(1, 0, 0);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 16, buf2);
	glDrawArrays(GL_QUADS, 0, q2 * 4);
	glDisableClientState(GL_VERTEX_ARRAY);
	glPopMatrix();

	glPopMatrix();
	glMatrixMode(GL_PROJECTION); glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void saveHighScores(const vector<int>& scores) {
	vector<int> sorted = scores;
	std::sort(sorted.begin(), sorted.end(), std::greater<int>());
	if (sorted.size() > 3) sorted.resize(3);  // keep top 3

	std::ofstream file("highscores.txt");
	for (int s : sorted) file << s << "\n";
}


vector<int> loadHighScores() {
	vector<int> scores;
	std::ifstream file("highscores.txt");
	int s;
	while (file >> s) {
		if (s > 0) scores.push_back(s);  // allow duplicates
	}
	std::sort(scores.begin(), scores.end(), std::greater<int>());
	while (scores.size() < 3) scores.push_back(0);  // Fill up to 3
	return scores;
}


vector<int> highScores = loadHighScores();



void drawPaddle() {
	float width = 0.3f, height = 0.05f;
	glColor3f(0.9f, 0.9f, 0.2f);
	glBegin(GL_QUADS);
	glVertex2f(paddleX - width / 2, -0.95f);
	glVertex2f(paddleX + width / 2, -0.95f);
	glVertex2f(paddleX + width / 2, -0.90f);
	glVertex2f(paddleX - width / 2, -0.90f);
	glEnd();
}

void initGame(vector<Brick>& bricks, vector<Circle>& world, const vector<Brick>& bricksBackup) {
	bricks.clear();
	for (const auto& b : bricksBackup)
		bricks.emplace_back(b.x, b.y, b.width);

	world.clear();
	Circle ball(paddleX, -0.85f, 0.05f, 0, 0, 1, 1, 1);
	ball.isWaiting = true;
	world.push_back(ball);
}

int main(void) {
	srand(time(NULL));

	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = glfwCreateWindow(480, 480, "8-2 Assignment", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glfwSwapInterval(1);

	// Add initial waiting ball
	Circle initialBall(paddleX, -0.85f, 0.05f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	initialBall.isWaiting = true;
	world.push_back(initialBall);
	for (int row = 0; row < 3; ++row) {
		for (int col = -3; col <= 3; ++col) {
			float x = col * 0.3f + 0.1f * (row % 2);
			float y = 0.6f - row * 0.25f;
			float red = row * 0.3f, green = 1 - row * 0.3f, blue = col * 0.1f + 0.3f;
			BRICKTYPE type = (row == 0 ? REFLECTIVE : DESTRUCTABLE);
			bricks.emplace_back(x, y, 0.28f);
			bricksBackup.push_back(bricks.back());
		}
	}


	while (!glfwWindowShouldClose(window)) {

		processInput(window);
		//Setup View
		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);


		if (gameState == STATE_HIGHSCORE) {
			glClear(GL_COLOR_BUFFER_BIT);
			renderCenteredMultilineText(240, 120, 2.5f, "H I G H   S C O R E S", "");
			for (int i = 0; i < highScores.size(); ++i) {
				char buf[32];
				snprintf(buf, sizeof(buf), "%d. %d", i + 1, highScores[i]);
				renderCenteredMultilineText(240, 180 + i * 40, 2.0f, buf, "");
			}
			renderCenteredMultilineText(240, 340, 1.8f, "Press ENTER to return", "");

			if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && enterReleased) {
				enterReleased = false;
				gameState = STATE_MENU;
			}
			if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
				enterReleased = true;
			}

			else {
				if (enterPressedInHighScore) {
					gameState = STATE_MENU;
					enterPressedInHighScore = false;
				}
			}

			glfwSwapBuffers(window);
			glfwPollEvents();
			continue;
		}





		if (gameState == STATE_MENU) {
			glClear(GL_COLOR_BUFFER_BIT);
			for (int i = 0; i < menuItems.size(); ++i) {
				float y = 180 + i * 50;
				float scale = (i == menuIndex) ? 1.5f : 1.0f;
				glColor3f((i == menuIndex) ? 1.0f : 0.6f, 1.0f, 1.0f);
				renderCenteredMultilineText(240, y, scale, menuItems[i].c_str(), "");
			}


			glfwSwapBuffers(window);
			glfwPollEvents();
			continue;
		}

		//Movement
		bool allBallsGone = true;

		for (int i = 0; i < world.size(); ) {
			if (world[i].isWaiting) {
				world[i].x = paddleX;
				world[i].DrawCircle();
				i++;
				continue;
			}

			world[i].hasHitBrickThisFrame = false;
			for (auto& brk : bricks) {
				if (world[i].CheckCollision(&brk)) {
					// Spawn new ball after 3 bricks broken
					float paddleTopY = -0.90f;
					float ballY = paddleTopY + 0.05f;
					Circle newBall(paddleX, ballY, 0.05f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
					newBall.isWaiting = true;
					world.push_back(newBall);
					break;
				}
				if (world[i].hasHitBrickThisFrame) break;
			}


			world[i].MoveOneStep();
			world[i].DrawCircle();

			for (size_t j = 0; j < world.size(); ++j) {
				if (i != j) {
					world[i].CheckCollisionWithOtherBall(world[j]);
				}
			}


			if (world[i].isOffBottom) {
				world.erase(world.begin() + i);
			}
			else {
				allBallsGone = false;
				i++;
			}
		}
		// Move this to JUST AFTER processing world[i] loop
		bool anyBricksLeft = false;
		for (const auto& brick : bricks) {
			if (brick.onoff == ON) {
				anyBricksLeft = true;
				break;
			}
		}

		if (!anyBricksLeft) {
			// Clear all previous balls and bricks
			world.clear();
			bricks.clear();

			// Reload original bricks
			for (const auto& b : bricksBackup)
				bricks.emplace_back(b.x, b.y, b.width);

			// Spawn waiting ball
			Circle newBall(paddleX, -0.85f, 0.05f, 0, 0, 1, 1, 1);
			newBall.isWaiting = true;
			world.push_back(newBall);

			allBricksCleared = false;
		}



		if (world.empty() && !allBricksCleared) {
			isGameOver = true;
			allBricksCleared = false;
		}


		for (auto& brick : bricks) {
			brick.drawBrick();
		}
		

		drawPaddle();

		// RENDER SCORE
		char scoreDisplay[64] = { 0 };
		snprintf(scoreDisplay, sizeof(scoreDisplay), "Score: %d", score);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		renderCenteredMultilineText(240, 50, 1.0f, scoreDisplay, "");
		
		// GAME OVER TEXT
		if (isGameOver) {
			renderCenteredMultilineText(240, 240, 2.0f, "Game Over!", "Press ENTER to return to menu");
		}



		glMatrixMode(GL_PROJECTION);
		glPushMatrix(); glLoadIdentity();
		glOrtho(0, 480, 480, 0, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix(); glLoadIdentity();

		renderCenteredMultilineText(240, 50, 1.0f, scoreDisplay, "");

		glPopMatrix();
		glMatrixMode(GL_PROJECTION); glPopMatrix();
		glMatrixMode(GL_MODELVIEW);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate;
	exit(EXIT_SUCCESS);
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	static bool menuKeyPressed = false;

	// Handle MENU navigation
	if (gameState == STATE_MENU) {
		if (!menuKeyPressed) {
			if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
				menuIndex = (menuIndex + 1) % menuItems.size();
				menuKeyPressed = true;
			}
			else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
				menuIndex = (menuIndex + menuItems.size() - 1) % menuItems.size();
				menuKeyPressed = true;
			}
			else if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
				switch (menuIndex) {
				case 0:
					initGame(bricks, world, bricksBackup);
					gameState = STATE_PLAYING;
					isGameOver = false;
					break;
				case 1:
					gameState = STATE_HIGHSCORE;
					enterReleased = false; // prevent immediate exit
					break;
				case 2:
					glfwSetWindowShouldClose(window, true);
					break;
				}
				menuKeyPressed = true;
			}
		}

		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE &&
			glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE &&
			glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
			menuKeyPressed = false;
		}

		return; // Skip rest while in menu
	}

	if (isGameOver) {
		if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && enterReleased) {
			enterReleased = false;

			// Load existing high scores from file
			vector<int> existingScores = loadHighScores();
			existingScores.push_back(score);
			std::sort(existingScores.begin(), existingScores.end(), std::greater<int>());
			if (existingScores.size() > 3) existingScores.resize(3);  // only keep top 3
			saveHighScores(existingScores);
			highScores = existingScores;  // update in-memory vector

			score = 0;
			isGameOver = false;
			hitReset = true;
			initGame(bricks, world, bricksBackup);
			gameState = STATE_MENU;
		}
		if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
			enterReleased = true;
		}

		return;
	}


	// Handle gameplay input
	if (gameState == STATE_PLAYING) {
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			if (!spacePressed) {
				spacePressed = true;

				for (auto& c : world) {
					if (c.isWaiting) {
						float angle = rand() % 90 + 45;
						float speed = 0.02f;
						c.vx = cos(angle * DEG2RAD) * speed;
						c.vy = fabs(sin(angle * DEG2RAD) * speed);
						c.isWaiting = false;
						break;
					}
				}
			}
		}
		else {
			spacePressed = false;
		}

		const float paddleHalfWidth = 0.15f;

		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
			paddleX -= 0.02f;
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			paddleX += 0.02f;
		}

		// Clamp paddle position
		if (paddleX - paddleHalfWidth < -1.0f) {
			paddleX = -1.0f + paddleHalfWidth;
		}
		if (paddleX + paddleHalfWidth > 1.0f) {
			paddleX = 1.0f - paddleHalfWidth;
		}
	}
}
