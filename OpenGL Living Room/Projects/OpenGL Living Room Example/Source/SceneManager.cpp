///////////////////////////////////////////////////////////////////////////////
// shadermanager.cpp
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
//  EDITOR: Holly Renfrew - SNHU Student / Computer Science
//  DUE: 6/1/25                           DATE: 6/8/25
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>
#include <map>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager* pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationX * rotationY * rotationZ * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag){

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
			m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/***********************************************************
  *  LoadSceneTextures()
  *
  *  This method is used for preparing the 3D scene by loading
  *  the shapes, textures in memory to support the 3D scene
  *  rendering
  ***********************************************************/
void SceneManager::LoadSceneTextures()
{
	bool bReturn = false;
	bReturn = CreateGLTexture(
		"Textures/drywall.jpg",
		"drywall"); // 1
	bReturn = CreateGLTexture(
		"Textures/floor.jpg",
		"floor"); // 1
	bReturn = CreateGLTexture(
		"Textures/black_wood.jpg",
		"black_wood"); // 3
	bReturn = CreateGLTexture(
		"Textures/photo.jpg",
		"photo"); // 4
	bReturn = CreateGLTexture(
		"Textures/tv.jpg",
		"tv");// 5
	bReturn = CreateGLTexture(
		"Textures/black_plastic.jpg",
		"black_plastic"); //6
	bReturn = CreateGLTexture(
		"Textures/rug.jpg",
		"rug"); //7
	bReturn = CreateGLTexture(
		"Textures/table.jpg",
		"table"); //8 
	bReturn = CreateGLTexture(
		"Textures/plastic.png",
		"plastic");//9
	bReturn = CreateGLTexture(
		"Textures/tuffet.jpg",
		"tuffet");//10
	bReturn = CreateGLTexture(
		"Textures/glass_texture.png",
		"glass"); // 11
	bReturn = CreateGLTexture(
		"Textures/outside.jpg",
		"outside"); //12
	bReturn = CreateGLTexture(
		"Textures/white_plastic.jpg",
		"white_plastic"); //13

	// after the texture image data is loaded into memory, the
	// loaded textures need to be bound to texture slots - there
	// are a total of 16 available slots for scene textures
	BindGLTextures();
}

/***********************************************************
* DefineObjectMaterials()
 *
* This method is used for configuring the various material
* se�ngs for all of the objects in the 3D scene.
***********************************************************/
void SceneManager::DefineObjectMaterials()
{
	OBJECT_MATERIAL drywallMaterial;
	drywallMaterial.ambientColor = glm::vec3(0.5f, 0.5f, 0.5f);
	drywallMaterial.ambientStrength = 0.1f;
	drywallMaterial.diffuseColor = glm::vec3(0.5f, 0.5f, 0.5f);
	drywallMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
	drywallMaterial.shininess = 1.0f;
	drywallMaterial.tag = "drywall";
	m_objectMaterials.push_back(drywallMaterial);

	OBJECT_MATERIAL floorMaterial;
	floorMaterial.ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);
	floorMaterial.ambientStrength = 0.4f;
	floorMaterial.diffuseColor = glm::vec3(0.4f, 0.4f, 0.4f);
	floorMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
	floorMaterial.shininess = 8.0f;
	floorMaterial.tag = "floor";
	m_objectMaterials.push_back(floorMaterial);

	OBJECT_MATERIAL blackWoodMaterial;
	blackWoodMaterial.ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
	blackWoodMaterial.ambientStrength = 0.3f;
	blackWoodMaterial.diffuseColor = glm::vec3(0.2f, 0.2f, 0.2f);
	blackWoodMaterial.specularColor = glm::vec3(0.3f, 0.3f, 0.3f);
	blackWoodMaterial.shininess = 10.0f;
	blackWoodMaterial.tag = "black_wood";
	m_objectMaterials.push_back(blackWoodMaterial);

	OBJECT_MATERIAL photoMaterial;
	photoMaterial.ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);
	photoMaterial.ambientStrength = 0.2f;
	photoMaterial.diffuseColor = glm::vec3(0.7f, 0.7f, 0.7f);
	photoMaterial.specularColor = glm::vec3(1.0f, 1.0f, 1.0f);
	photoMaterial.shininess = 64.0f;
	photoMaterial.tag = "photo";
	m_objectMaterials.push_back(photoMaterial);

	OBJECT_MATERIAL tvMaterial;
	tvMaterial.ambientColor = glm::vec3(0.5f, 0.5f, 0.5f);
	tvMaterial.ambientStrength = 1.0f; 
	tvMaterial.diffuseColor = glm::vec3(0.5f, 0.5f, 0.5f); 
	tvMaterial.specularColor = glm::vec3(0.9f, 0.9f, 0.9f);
	tvMaterial.shininess = 64.0f;
	tvMaterial.tag = "tv";
	m_objectMaterials.push_back(tvMaterial);

	OBJECT_MATERIAL blackPlasticMaterial;
	blackPlasticMaterial.ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
	blackPlasticMaterial.ambientStrength = 0.3f;
	blackPlasticMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.3f);
	blackPlasticMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
	blackPlasticMaterial.shininess = 4.0f;
	blackPlasticMaterial.tag = "black_plastic";
	m_objectMaterials.push_back(blackPlasticMaterial);

	OBJECT_MATERIAL rugMaterial;
	rugMaterial.ambientColor = glm::vec3(0.25f, 0.25f, 0.3f);
	rugMaterial.ambientStrength = 0.3f;
	rugMaterial.diffuseColor = glm::vec3(0.35f, 0.35f, 0.4f);
	rugMaterial.specularColor = glm::vec3(0.1f, 0.1f, 0.1f);
	rugMaterial.shininess = 2.0f;
	rugMaterial.tag = "rug";
	m_objectMaterials.push_back(rugMaterial);

	OBJECT_MATERIAL tableMaterial;
	tableMaterial.ambientColor = glm::vec3(0.3f, 0.2f, 0.1f);
	tableMaterial.ambientStrength = 0.3f;
	tableMaterial.diffuseColor = glm::vec3(0.4f, 0.25f, 0.15f);
	tableMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
	tableMaterial.shininess = 6.0f;
	tableMaterial.tag = "table";
	m_objectMaterials.push_back(tableMaterial);

	OBJECT_MATERIAL plasticMaterial;
	plasticMaterial.ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);
	plasticMaterial.ambientStrength = 0.1f;
	plasticMaterial.diffuseColor = glm::vec3(0.4f, 0.4f, 0.4f);
	plasticMaterial.specularColor = glm::vec3(0.1f, 0.1f, 0.1f);
	plasticMaterial.shininess = 8.0f;
	plasticMaterial.tag = "plastic";
	m_objectMaterials.push_back(plasticMaterial);

	OBJECT_MATERIAL glassMaterial;
	glassMaterial.ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);   
	glassMaterial.ambientStrength = 0.05f;
	glassMaterial.diffuseColor = glm::vec3(0.1f, 0.1f, 0.1f);   
	glassMaterial.specularColor = glm::vec3(1.0f, 1.0f, 1.0f);
	glassMaterial.shininess = 128.0f;
	glassMaterial.tag = "glass";
	m_objectMaterials.push_back(glassMaterial);

	OBJECT_MATERIAL whitePlasticMaterial;
	whitePlasticMaterial.ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
	whitePlasticMaterial.ambientStrength = 0.1f;
	whitePlasticMaterial.diffuseColor = glm::vec3(0.15f, 0.15f, 0.15f);
	whitePlasticMaterial.specularColor = glm::vec3(0.8f, 0.8f, 0.8f);   
	whitePlasticMaterial.shininess = 32.0f;  
	whitePlasticMaterial.tag = "white_plastic";
	m_objectMaterials.push_back(whitePlasticMaterial);

}
void SceneManager::SetupSceneLights()
{
	m_pShaderManager->setBoolValue("bUseLighting", true);

	m_pShaderManager->setVec3Value("lightSources[0].position", -29.8f, 24.9f, 9.5f); 
	m_pShaderManager->setVec3Value("lightSources[0].ambientColor", 0.05f, 0.05f, 0.05f);
	m_pShaderManager->setVec3Value("lightSources[0].diffuseColor", 0.4f, 0.4f, 0.3f);
	m_pShaderManager->setVec3Value("lightSources[0].specularColor", 0.3f, 0.3f, 0.2f);
	m_pShaderManager->setFloatValue("lightSources[0].focalStrength", 8.0f);
	m_pShaderManager->setFloatValue("lightSources[0].specularIntensity", 1.0f);


	m_pShaderManager->setVec3Value("lightSources[1].position", 33.0f, 24.5f, -14.0f);
	m_pShaderManager->setVec3Value("lightSources[1].ambientColor", 0.05f, 0.05f, 0.05f);
	m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", 0.1f, 0.1f, 0.1f); 
	m_pShaderManager->setVec3Value("lightSources[1].specularColor", 0.2f, 0.2f, 0.2f);
	m_pShaderManager->setFloatValue("lightSources[1].focalStrength", 16.0);
	m_pShaderManager->setFloatValue("lightSources[1].specularIntensity", 0.8f);



	m_pShaderManager->setVec3Value("lightSources[2].position", 36.0f, 20.5f, 12.0f);
	m_pShaderManager->setVec3Value("lightSources[2].ambientColor", 0.05f, 0.05f, 0.05f);
	m_pShaderManager->setVec3Value("lightSources[2].diffuseColor", 0.1f, 0.1f, 0.1f);
	m_pShaderManager->setVec3Value("lightSources[2].specularColor", 0.2f, 0.2f, 0.2f);
	m_pShaderManager->setFloatValue("lightSources[2].focalStrength", 16.0);
	m_pShaderManager->setFloatValue("lightSources[2].specularIntensity", 0.8f);


}
/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	LoadSceneTextures();
	DefineObjectMaterials();
	SetupSceneLights();
	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene

	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadTaperedCylinderMesh();
	m_basicMeshes->LoadPrismMesh();
	m_basicMeshes->LoadConeMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadPyramid3Mesh();
	m_basicMeshes->LoadPyramid4Mesh();
	m_basicMeshes->LoadTorusMesh(0.8f);

	m_basicMeshes->LoadSphereMesh();

}
/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{

	DrawWalls();
	DrawTable(5.0f, -3.0f, 1.0f);
	DrawTVStand(0.0f, 1.0f, -12.0f);
	DrawPictureFrame(-5.0f, 18.0f, -14.9f);
	DrawTV(0.0f, 8.0f, -12.0f);
	DrawSquirtBottle(5.0f, -2.5f, 1.0f);
	DrawBookcase(-15.0f, 6.5f, -14.0f);
	DrawBookcase(29.0f, 6.5f, -14.0f);
	DrawTuffet(-25.0f, -6.65f, -7.5f);
	DrawLamp(33.0f, -8.4f, -15.0f);


}
void SceneManager::DrawWalls() {
	glm::vec3 scaleXYZ, positionXYZ;
	float XrotationDegrees, YrotationDegrees, ZrotationDegrees;
	/*** Drawing the Back Wall ***/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(36.0f, 1.0f, 30.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 15.0f, -18.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);


	SetShaderTexture("drywall");
	SetShaderMaterial("drywall");
	SetTextureUVScale(20.0f, 20.0f);
	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();


	/****************************************************************/
	/*** Drawing the Ceiling ***/
	/****************************************************************/

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(36.0f, 1.0f, 20.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 180.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 24.0f, 0);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("drywall");

	SetShaderMaterial("drywall");
	SetTextureUVScale(20.0f, 20.0f);
	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();



	/****************************************************************/
	/*** Drawing the Floor ***/
	/****************************************************************/

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(36.0f, 1.0f, 20.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, -8.5f, 0);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("floor");

	SetShaderMaterial("floor");
	SetTextureUVScale(20.0f, 20.0f);
	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();

	// the rug
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(25.0f, 1.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, -8.45f, 5.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("rug");
	SetTextureUVScale(1.0f, 1.0f);

	SetShaderMaterial("rug");
	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();


	/*** Drawing the Left Wall ***/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(16.5f, 1.0f, 7.0f);
	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-30.0f, 8.0f, -10.5f);
	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetShaderTexture("drywall");
	SetShaderMaterial("drywall");
	SetTextureUVScale(20.0f, 20.0f);
	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();

	/*** Drawing the Window Wall ***/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(16.5f, 1.0f, 12.0f);
	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-30.0f, 8.0f, 8.5f);
	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetShaderTexture("glass");
	SetShaderMaterial("glass");
	SetTextureUVScale(20.0f, 20.0f);
	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
	/*** Drawing the Outside ***/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(16.5f, 1.0f, 20.0f);
	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-30.2f, 8.0f, 0.0f);
	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetShaderTexture("outside");
	SetShaderMaterial("floor");
	SetTextureUVScale(1.0f, 1.0f);
	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
}
void SceneManager::DrawTVStand(float xPos, float yPos, float zPos) {
	glm::vec3 scaleXYZ, positionXYZ;
	float XrotationDegrees, YrotationDegrees, ZrotationDegrees;
	SetShaderTexture("black_wood");
	SetShaderMaterial("black_wood");

	/****************************************************************/
	/*** Drawing the Top Box TV Stand ***/
	/****************************************************************/

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(21.0f, 1.0f, 6.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(xPos, yPos, zPos);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetTextureUVScale(21.0f, 6.0f);
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();

	/****************************************************************/
	/*** Drawing the Left Box TV Stand ***/
	/****************************************************************/

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(10.0f, 1.0f, 6.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(xPos - 11.0f, yPos - 4.5f, zPos);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetTextureUVScale(10.0f, 6.0f);
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();


	/****************************************************************/
	/*** Drawing the Middle Shelf Box TV Stand ***/
	/****************************************************************/

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(21.0f, 1.0f, 6.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(xPos, yPos - 3.0f, zPos);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetTextureUVScale(20.0f, 6.0f);
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();


	/****************************************************************/
	/*** Drawing the Right Box TV Stand ***/
	/****************************************************************/

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(10.0f, 1.0f, 6.0f);
	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(xPos + 11.0f, yPos - 4.5f, zPos);
	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetTextureUVScale(10.0f, 6.0f);
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();


	/****************************************************************/
	/*** Drawing Top of Drawers For TV Stand ***/
	/****************************************************************/

	// Left Drawer
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(7.0f, 1.0f, 6.0f);
	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(xPos -7.0f, yPos - 4.0f, zPos);
	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetTextureUVScale(7.0f, 6.0f);
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();

	// Middle Drawer 
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(xPos, yPos - 4.0f, zPos);
	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetTextureUVScale(7.0f, 6.0f);
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();

	// Right Drawer 
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(xPos + 7.0f, yPos - 4.0f, zPos);
	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetTextureUVScale(7.0f, 6.0f);
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();


	/****************************************************************/
	/*** Drawing Left of Drawers For TV Stand ***/
	/****************************************************************/
	
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(4.0f, 1.0f, 5.0f);
	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	//Left Drawer
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(xPos - 10.0f, yPos - 6.5f, zPos + 0.5f);
	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetTextureUVScale(4.0f, 5.0f);
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();

	// Middle Drawer 
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(xPos - 3.0f, yPos - 6.5f, zPos + 0.5f);
	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetTextureUVScale(4.0f, 5.0f);
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();

	// Right Drawer 
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(xPos + 4.0f, yPos - 6.5f, zPos + 0.5f);
	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetTextureUVScale(4.0f, 5.0f);
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();

	/****************************************************************/
	/*** Drawing Right of Drawers For TV Stand ***/
	/****************************************************************/

	// Left Drawer
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(xPos - 4.0f, yPos - 6.5f, zPos + 0.5f);
	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetTextureUVScale(4.0f, 5.0f);
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();

	// Middle Drawer 
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(xPos + 4.0f, yPos - 6.5f, zPos + 0.5f);
	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetTextureUVScale(4.0f, 5.0f);
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();

	// Right Drawer 
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(xPos + 10.0f, yPos - 6.5f, zPos + 0.5f);
	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetTextureUVScale(4.0f, 5.0f);
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();

	/****************************************************************/
	/*** Drawing Bottom of Drawers For TV Stand ***/
	/****************************************************************/

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(7.0f, 1.0f, 6.0f);
	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// Left Drawer
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(xPos - 7.0f, yPos - 9.0f, zPos);
	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetTextureUVScale(7.0f, 6.0f);
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();

	// Middle Drawer 
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(xPos, yPos - 9.0f, zPos);
	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetTextureUVScale(7.0f, 6.0f);
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();

	// Right Drawer 
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(xPos + 7.0f, yPos - 9.0f, zPos);
	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetTextureUVScale(7.0f, 6.0f);
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();

	/****************************************************************/
	/*** Drawing Back TV Stand ***/
	/****************************************************************/

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(21.0f, 1.0f, 10.0f);
	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(xPos, yPos - 4.5f, zPos - 2.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetTextureUVScale(21.0f, 10.0f);
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();

	/****************************************************************/
	/*** Drawing Front of Drawers For TV Stand ***/
	/****************************************************************/

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(4.0f, 5.0f, 0.3f);
	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	// Left Drawer
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(xPos - 7.0f, yPos - 6.5f, zPos + 2.7f);
	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetShaderTexture("plastic");
	SetShaderMaterial("plastic");
	SetTextureUVScale(4.0f, 5.0f);
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();

	// Middle Drawer 
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(xPos, yPos - 6.5f, zPos + 2.7f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetTextureUVScale(4.0f, 5.0f);
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();

	// Right Drawer 
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(xPos + 7.0f, yPos - 6.5f, zPos + 2.7f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetTextureUVScale(4.0f, 5.0f);
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();

}
void SceneManager::DrawTV(float xPos, float yPos, float zPos) {
	glm::vec3 scaleXYZ, positionXYZ;
	float XrotationDegrees, YrotationDegrees, ZrotationDegrees;
	// === TV Parameters ===
	float photoWidth = 10.0f;
	float photoHeight = 5.625f;
	float frameThickness = 0.5f;
	float frameDepth = 0.2f;

	// === Photo (center rectangle) ===
	scaleXYZ = glm::vec3(photoWidth, 1.0f, photoHeight);
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(xPos,yPos,zPos);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("tv");
	SetShaderMaterial("tv");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawPlaneMesh();

	// === Back Panel ===
	scaleXYZ = glm::vec3(photoWidth*2 + frameThickness*2, photoHeight*2 + frameThickness*2, 0.5f);
	positionXYZ = glm::vec3(xPos, yPos, zPos - 0.30f);
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderTexture("black_plastic");

	SetShaderMaterial("black_plastic");
	SetTextureUVScale(photoWidth * 2, photoHeight * 2);
	m_basicMeshes->DrawBoxMesh();

	// === Top Frame === 
	scaleXYZ = glm::vec3(photoWidth * 2 + (frameThickness * 2), frameThickness, frameDepth);
	positionXYZ = glm::vec3(xPos, (photoHeight)+(frameThickness / 2) + yPos, zPos);
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawBoxMesh();

	// === Bottom Frame ===
	positionXYZ = glm::vec3(xPos, -photoHeight - frameThickness / 2 + yPos, zPos);
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	// === Left Frame ===
	scaleXYZ = glm::vec3(frameThickness, photoHeight * 2, frameDepth);
	positionXYZ = glm::vec3(-photoWidth - frameThickness/2 - xPos, yPos, zPos);
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	// === Right Frame ===
	positionXYZ = glm::vec3(photoWidth + frameThickness / 2 + xPos , yPos, zPos);
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	// === Right Leg ===
	scaleXYZ = glm::vec3(0.25f, 1.25f, 0.25f);
	XrotationDegrees = 75.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(xPos + 8.0f, yPos - 6.2f, zPos - 0.75f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	positionXYZ = glm::vec3(xPos + 8.0f, yPos - 6.2f, zPos + 0.25f);
	SetTransformations(scaleXYZ, -XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	// === Left Leg ===
	scaleXYZ = glm::vec3(0.25f, 1.25f, 0.25f);
	XrotationDegrees = 75.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(xPos - 8.0f, yPos - 6.2f, zPos - 0.75f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	positionXYZ = glm::vec3(xPos - 8.0f, yPos - 6.2f, zPos + 0.25f);
	SetTransformations(scaleXYZ, -XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	m_basicMeshes->DrawBoxMesh();


}
void SceneManager::DrawPictureFrame(float xPos, float yPos, float zPos) {
	glm::vec3 scaleXYZ, positionXYZ;
	float XrotationDegrees, YrotationDegrees, ZrotationDegrees;

	// === Frame Parameters ===
	float photoWidth = 3.6f;
	float photoHeight = 4.32f;
	float frameThickness = 0.5f;
	float frameDepth = 0.2f;

	// === Photo (center rectangle) ===
	scaleXYZ = glm::vec3(photoWidth, 1.0f, photoHeight);
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(xPos, yPos, zPos);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("photo");
	SetShaderMaterial("photo");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawPlaneMesh();

	// === Top Frame === 
	scaleXYZ = glm::vec3(photoWidth * 2 + (frameThickness * 2), frameThickness, frameDepth);
	positionXYZ = glm::vec3(xPos, yPos + photoHeight + (frameThickness / 2), zPos);
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderTexture("black_wood");
	SetShaderMaterial("black_wood");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawBoxMesh();

	// === Bottom Frame ===
	positionXYZ = glm::vec3(xPos, yPos - (photoHeight/2) - (frameThickness / 2)-2, zPos);
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	// === Left Frame ===
	scaleXYZ = glm::vec3(frameThickness, photoHeight * 2, frameDepth);
	positionXYZ = glm::vec3(xPos - (photoWidth/2) + frameThickness - 2.55f, yPos, zPos);
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	// === Right Frame ===
	positionXYZ = glm::vec3(xPos + (photoWidth/2) + (frameThickness / 2)+1.80 , yPos, zPos);
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	m_basicMeshes->DrawBoxMesh();


}
void SceneManager::DrawBookcase(float xPos, float yPos, float zPos) {
	glm::vec3 scaleXYZ, positionXYZ;
	float XrotationDegrees, YrotationDegrees, ZrotationDegrees;

	// right side
	scaleXYZ = glm::vec3(1.0f, 30.0f, 5.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(xPos, yPos, zPos);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("black_wood");
	SetShaderMaterial("black_wood");
	SetTextureUVScale(1.0f, 30.0f);
	m_basicMeshes->DrawBoxMesh();

	// left side
	scaleXYZ = glm::vec3(1.0f, 30.0f, 5.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(xPos - 14.0f, yPos, zPos);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("black_wood");
	SetShaderMaterial("black_wood");
	SetTextureUVScale(1.0f, 30.0f);
	m_basicMeshes->DrawBoxMesh();

	// top
	scaleXYZ = glm::vec3(1.0f, 14.0f, 5.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;
	positionXYZ = glm::vec3(xPos - 7.0f, yPos + 14.5f, zPos);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("black_wood");
	SetShaderMaterial("black_wood");
	SetTextureUVScale(1.0f, 14.0f);
	m_basicMeshes->DrawBoxMesh();

	// bottom
	ZrotationDegrees = 90.0f;
	positionXYZ = glm::vec3(xPos - 7.0f, yPos - 14.5f, zPos);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("black_wood");
	SetShaderMaterial("black_wood");
	SetTextureUVScale(5.0f, 14.0f);
	m_basicMeshes->DrawBoxMesh();

	float topY = yPos + 14.5f; // topmost Y
	float barHeight = 1.0f;
	float gap = 4.0f;

	for (int i = 0; i < 5; ++i) {
		float barCenterOffset = gap * (i + 1) + barHeight * i + barHeight / 2.0f;
		float barY = topY - barCenterOffset;

		positionXYZ = glm::vec3(xPos - 7.0f, barY, zPos);
		SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
		SetShaderTexture("black_wood");
		SetShaderMaterial("black_wood");
		SetTextureUVScale(5.0f, 14.0f);
		m_basicMeshes->DrawBoxMesh();
	}

	// back panel (plane)
	scaleXYZ = glm::vec3(7.0f, 1.0f, 15.0f);  // Width = 13 (from -14 to 0), Height = 30
	XrotationDegrees = 90.0f; // Rotate to align vertically
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(xPos - 7.0f, yPos, zPos); // slightly behind shelves
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("black_wood");
	SetShaderMaterial("black_wood");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawPlaneMesh();


// populating the shelves
	// ROW 1
	std::vector<glm::vec3> shelfScales1 = {
		{1.2f, 1.5f, 3.0f}, {1.4f, 1.5f, 3.0f}, {1.0f, 1.5f, 3.0f},
		{1.3f, 1.5f, 3.0f}, {0.9f, 1.5f, 3.0f}, {1.1f, 1.5f, 3.0f},
		{1.5f, 1.5f, 3.0f}, {0.8f, 1.5f, 3.0f}, {1.2f, 1.5f, 3.0f}, {1.1f, 1.5f, 3.0f}
	};
	std::vector<glm::vec3> shelfColors1 = {
		{0.95f, 0.4f, 0.2f}, {0.3f, 0.7f, 0.9f}, {0.6f, 0.9f, 0.4f},
		{0.5f, 0.3f, 0.9f}, {0.8f, 0.9f, 0.3f}, {0.4f, 0.1f, 0.7f},
		{0.2f, 0.8f, 0.8f}, {0.85f, 0.5f, 0.25f}, {0.65f, 0.65f, 0.65f}, {0.3f, 0.95f, 0.5f}
	};

	// ROW 2
	std::vector<glm::vec3> shelfScales2 = {
		{1.0f, 1.5f, 3.0f}, {0.7f, 1.5f, 3.0f}, {1.4f, 1.5f, 3.0f},
		{1.3f, 1.5f, 3.0f}, {1.1f, 1.5f, 3.0f}, {0.9f, 1.5f, 3.0f},
		{1.5f, 1.5f, 3.0f}, {0.8f, 1.5f, 3.0f}, {1.2f, 1.5f, 3.0f}, {1.1f, 1.5f, 3.0f}
	};
	std::vector<glm::vec3> shelfColors2 = {
		{0.1f, 0.9f, 0.3f}, {0.8f, 0.4f, 0.6f}, {0.5f, 0.7f, 0.9f},
		{0.6f, 0.6f, 0.2f}, {0.3f, 0.8f, 0.4f}, {0.7f, 0.2f, 0.8f},
		{0.9f, 0.6f, 0.3f}, {0.4f, 0.4f, 0.7f}, {0.1f, 0.6f, 0.9f}, {0.3f, 0.3f, 0.3f}
	};

	// ROW 3
	std::vector<glm::vec3> shelfScales3 = {
		{0.9f, 1.5f, 3.0f}, {1.2f, 1.5f, 3.0f}, {1.0f, 1.5f, 3.0f},
		{1.1f, 1.5f, 3.0f}, {1.4f, 1.5f, 3.0f}, {0.8f, 1.5f, 3.0f},
		{1.5f, 1.5f, 3.0f}, {1.3f, 1.5f, 3.0f}, {1.1f, 1.5f, 3.0f}, {1.2f, 1.5f, 3.0f}
	};
	std::vector<glm::vec3> shelfColors3 = {
		{0.7f, 0.7f, 0.1f}, {0.5f, 0.3f, 0.7f}, {0.6f, 0.4f, 0.9f},
		{0.2f, 0.8f, 0.6f}, {0.3f, 0.6f, 0.7f}, {0.9f, 0.3f, 0.5f},
		{0.8f, 0.9f, 0.1f}, {0.1f, 0.4f, 0.8f}, {0.5f, 0.9f, 0.2f}, {0.2f, 0.2f, 0.9f}
	};

	// ROW 4
	std::vector<glm::vec3> shelfScales4 = {
		{1.3f, 1.5f, 3.0f}, {1.1f, 1.5f, 3.0f}, {0.8f, 1.5f, 3.0f},
		{1.0f, 1.5f, 3.0f}, {1.2f, 1.5f, 3.0f}, {1.4f, 1.5f, 3.0f},
		{0.7f, 1.5f, 3.0f}, {1.3f, 1.5f, 3.0f}, {1.1f, 1.5f, 3.0f}, {1.1f, 1.5f, 3.0f}
	};
	std::vector<glm::vec3> shelfColors4 = {
		{0.6f, 0.1f, 0.6f}, {0.7f, 0.5f, 0.3f}, {0.4f, 0.9f, 0.7f},
		{0.3f, 0.2f, 0.8f}, {0.6f, 0.6f, 0.4f}, {0.7f, 0.3f, 0.9f},
		{0.2f, 0.7f, 0.5f}, {0.9f, 0.7f, 0.2f}, {0.1f, 0.9f, 0.7f}, {0.4f, 0.8f, 0.3f}
	};

	// ROW 5
	std::vector<glm::vec3> shelfScales5 = {
		{1.4f, 1.5f, 3.0f}, {1.3f, 1.5f, 3.0f}, {1.1f, 1.5f, 3.0f},
		{1.2f, 1.5f, 3.0f}, {1.5f, 1.5f, 3.0f}, {1.0f, 1.5f, 3.0f},
		{1.3f, 1.5f, 3.0f}, {1.0f, 1.5f, 3.0f}, {1.0f, 1.5f, 3.0f}
	};

	std::vector<glm::vec3> shelfColors5 = {
		{0.6f, 0.4f, 0.2f}, {0.7f, 0.5f, 0.8f}, {0.2f, 0.9f, 0.3f},
		{0.9f, 0.3f, 0.5f}, {0.5f, 0.6f, 0.1f}, {0.3f, 0.7f, 0.9f},
		{0.8f, 0.4f, 0.6f}, {0.7f, 0.7f, 0.2f}, {0.4f, 0.6f, 0.9f}
	};


	// ROW 6
	std::vector<glm::vec3> shelfScales6 = {
		{1.3f, 1.5f, 3.0f}, {1.2f, 1.5f, 3.0f}, {1.4f, 1.5f, 3.0f},
		{1.1f, 1.5f, 3.0f}, {1.0f, 1.5f, 3.0f}, {1.5f, 1.5f, 3.0f},
		{1.0f, 1.5f, 3.0f}, {1.1f, 1.5f, 3.0f}, {1.1f, 1.5f, 3.0f}
	};

	std::vector<glm::vec3> shelfColors6 = {
		{0.3f, 0.8f, 0.5f}, {0.6f, 0.2f, 0.9f}, {0.5f, 0.5f, 0.3f},
		{0.8f, 0.4f, 0.7f}, {0.2f, 0.9f, 0.4f}, {0.9f, 0.2f, 0.6f},
		{0.4f, 0.6f, 0.7f}, {0.7f, 0.3f, 0.9f}, {0.6f, 0.7f, 0.2f}
	};



	float baseYOffset[] = { 11.5f, 6.5f, 1.5f, -3.5f, -7.5f, -12.5};
	std::vector<std::vector<glm::vec3>> shelfScalesList = {
		shelfScales1, shelfScales2, shelfScales3, shelfScales4, shelfScales5, shelfScales6
	};
	std::vector<std::vector<glm::vec3>> shelfColorsList = {
		shelfColors1, shelfColors2, shelfColors3, shelfColors4, shelfColors5, shelfColors6
	};


	for (int row = 0; row < 6; ++row) {

		float currentX = 0.0f;
		for (size_t i = 0; i < shelfScalesList[row].size(); ++i) {
			glm::vec3 scaleXYZ = shelfScalesList[row][i];
			float width = scaleXYZ.x;
			float boxX = xPos - 13.0f + currentX + width / 2.0f;
			glm::vec3 positionXYZ(boxX, yPos + baseYOffset[row], zPos + 0.75f);

			SetTransformations(scaleXYZ, 90.0f, 0.0f, 0.0f, positionXYZ);
			SetShaderColor(shelfColorsList[row][i].r, shelfColorsList[row][i].g, shelfColorsList[row][i].b, 1.0f);
			SetShaderMaterial("white_plastic");
			m_basicMeshes->DrawBoxMesh();
			currentX += width + 0.1f;
		}
	}


}
void SceneManager::DrawTuffet(float xPos, float yPos, float zPos) {
	glm::vec3 scaleXYZ, positionXYZ;
	float XrotationDegrees, YrotationDegrees, ZrotationDegrees;
	
	scaleXYZ = glm::vec3(2.0f, 2.0f, 2.0f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(xPos, yPos, zPos);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("tuffet");
	SetShaderMaterial("rug");
	SetTextureUVScale(2.0f, 1.0f);
	m_basicMeshes->DrawTorusMesh();

	scaleXYZ = glm::vec3(0.6f, 1.2f, 0.6f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(xPos, yPos - 0.6f, zPos);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("tuffet");
	SetShaderMaterial("rug");
	SetTextureUVScale(0.3f, 0.6f);
	m_basicMeshes->DrawCylinderMesh(true, true, false);


	// back leg right
	scaleXYZ = glm::vec3(0.4f, 0.4f, 0.5f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(xPos + 2.0f, yPos - 1.4f, zPos - 2.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("table");
	SetShaderMaterial("table");
	SetTextureUVScale(2.0f, 1.0f);
	m_basicMeshes->DrawTorusMesh();

	scaleXYZ = glm::vec3(0.5f, 0.4f, 0.5f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(xPos + 2.0f, yPos - 1.85f, zPos - 2.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("table");
	SetShaderMaterial("table");
	SetTextureUVScale(0.3f, 0.6f);
	m_basicMeshes->DrawCylinderMesh();

	// back leg left
	scaleXYZ = glm::vec3(0.4f, 0.4f, 0.5f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(xPos - 2.0f, yPos - 1.4f, zPos - 2.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("table");
	SetShaderMaterial("table");
	SetTextureUVScale(2.0f, 1.0f);
	m_basicMeshes->DrawTorusMesh();

	scaleXYZ = glm::vec3(0.5f, 0.4f, 0.5f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(xPos - 2.0f, yPos - 1.85f, zPos - 2.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("table");
	SetShaderMaterial("table");
	SetTextureUVScale(0.3f, 0.6f);
	m_basicMeshes->DrawCylinderMesh();

	// front leg right
	scaleXYZ = glm::vec3(0.4f, 0.4f, 0.5f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(xPos + 2.0f, yPos - 1.4f, zPos + 2.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("table");
	SetShaderMaterial("table");
	SetTextureUVScale(2.0f, 1.0f);
	m_basicMeshes->DrawTorusMesh();

	scaleXYZ = glm::vec3(0.5f, 0.4f, 0.5f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(xPos + 2.0f, yPos - 1.85f, zPos + 2.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("table");
	SetShaderMaterial("table");
	SetTextureUVScale(0.3f, 0.6f);
	m_basicMeshes->DrawCylinderMesh();

	// front leg left
	scaleXYZ = glm::vec3(0.4f, 0.4f, 0.5f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(xPos - 2.0f, yPos - 1.4f, zPos + 2.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("table");
	SetShaderMaterial("table");
	SetTextureUVScale(2.0f, 1.0f);
	m_basicMeshes->DrawTorusMesh();

	scaleXYZ = glm::vec3(0.5f, 0.4f, 0.5f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(xPos - 2.0f, yPos - 1.85f, zPos + 2.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("table");
	SetShaderMaterial("table");
	SetTextureUVScale(0.3f, 0.6f);
	m_basicMeshes->DrawCylinderMesh();


}
void SceneManager::DrawTable(float xPos, float yPos, float zPos) {
	glm::vec3 scaleXYZ, positionXYZ;
	float XrotationDegrees, YrotationDegrees, ZrotationDegrees;

	/****************************************************************/
	/*** Drawing the Table ***/
	/****************************************************************/
	
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(20.0f, 1.0f, 5.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(xPos, yPos, zPos);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("table");
	SetShaderMaterial("table");
	SetTextureUVScale(20.0f, 5.0f);
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();

	/****************************************************************/
	/*** Drawing the Table Legs ***/
	/****************************************************************/

	// Back Left Leg
	scaleXYZ = glm::vec3(1.0f, 11.0f, 1.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(xPos - 9.5f, yPos - 5.5f, zPos - 2.0f);
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetShaderTexture("table");
	SetShaderMaterial("table");
	SetTextureUVScale(20.0f, 5.0f);
	m_basicMeshes->DrawBoxMesh();

	// Front Left Leg
	scaleXYZ = glm::vec3(1.0f, 11.0f, 1.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(xPos - 9.5f, yPos - 5.5f, zPos + 2.0f);
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetShaderTexture("table");
	SetShaderMaterial("table");
	SetTextureUVScale(20.0f, 5.0f);
	m_basicMeshes->DrawBoxMesh();

	// Back Right Leg
	scaleXYZ = glm::vec3(1.0f, 11.0f, 1.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(xPos + 9.5f, yPos - 5.5f, zPos - 2.0f);
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetShaderTexture("table");
	SetShaderMaterial("table");
	SetTextureUVScale(20.0f, 5.0f);
	m_basicMeshes->DrawBoxMesh();

	// Front Right Leg
	scaleXYZ = glm::vec3(1.0f, 11.0f, 1.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(xPos + 9.5f, yPos - 5.5f, zPos + 2.0f);
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetShaderTexture("table");
	SetShaderMaterial("table");
	SetTextureUVScale(20.0f, 5.0f);
	m_basicMeshes->DrawBoxMesh();



	// Table Shelf
	scaleXYZ = glm::vec3(20.0f, 1.0f, 5.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(xPos, yPos - 2.5f, zPos );
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetShaderTexture("table");
	SetShaderMaterial("table");
	SetTextureUVScale(20.0f, 5.0f);
	m_basicMeshes->DrawBoxMesh();
}
void SceneManager::DrawLamp(float xPos, float yPos, float zPos) {
	glm::vec3 scaleXYZ, positionXYZ;
	float XrotationDegrees, YrotationDegrees, ZrotationDegrees;
	
	/****************************************************************/
	/*** Drawing the Base of the Lamp ***/
	/****************************************************************/
	scaleXYZ = glm::vec3(2.0f, 0.2f, 2.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(xPos, yPos, zPos);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("white_plastic");
	SetShaderMaterial("white_plastic");
	SetTextureUVScale(2.0f, 2.0f);
	m_basicMeshes->DrawCylinderMesh();
	
	/****************************************************************/
	/*** Drawing the Pole of the Lamp ***/
	/****************************************************************/
	scaleXYZ = glm::vec3(0.2f, 25.2f, 0.2f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(xPos, yPos + 0.2f, zPos);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("white_plastic");
	SetShaderMaterial("white_plastic");
	SetTextureUVScale(0.2, 25.2f);
	m_basicMeshes->DrawCylinderMesh(false, false, true);

	/****************************************************************/
	/*** Drawing the Sconce of the Lamp ***/
	/****************************************************************/

	int tiers = 4;
	float initialScale = 0.2f;
	float initialY = yPos + 33.7;

	for (int i = 0; i < tiers; ++i) {
		initialScale = initialScale * 2; 
		initialY = initialY + initialScale;
		scaleXYZ = glm::vec3(initialScale, initialScale, initialScale);
		positionXYZ = glm::vec3(xPos, yPos + initialY, zPos);
		XrotationDegrees = 0.0f;
		YrotationDegrees = 0.0f;
		ZrotationDegrees = 180.0f;
		SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
		SetShaderTexture("plastic");
		SetShaderMaterial("plastic");
		SetTextureUVScale(initialScale, initialScale);
		m_basicMeshes->DrawTaperedCylinderMesh(false, false, true);

	}

	/****************************************************************/
	/*** Drawing the Second Pole of the Lamp ***/
	/****************************************************************/
	scaleXYZ = glm::vec3(0.2f, 5.2f, 0.2f);
	XrotationDegrees = 15.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = -50.0f;
	positionXYZ = glm::vec3(xPos, yPos + 17.1f, zPos);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("plastic");
	SetShaderMaterial("plastic");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawCylinderMesh(false, false, true);

	scaleXYZ = glm::vec3(0.4f, 0.4f, 0.4f);
	XrotationDegrees = 15.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 130.0f;
	positionXYZ = glm::vec3(xPos+4.25, yPos + 20.55, zPos + 0.95f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("plastic");
	SetShaderMaterial("plastic");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawTaperedCylinderMesh(true, true, true);

	scaleXYZ = glm::vec3(0.8f, 0.8f, 0.8f);
	XrotationDegrees = 15.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 130.0f;
	positionXYZ = glm::vec3(xPos + 4.85, yPos + 21.03, zPos + 1.08f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("plastic");
	SetShaderMaterial("plastic");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawTaperedCylinderMesh(true, true, true);

	scaleXYZ = glm::vec3(1.6f, 1.6f, 1.6f);
	XrotationDegrees = 15.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 130.0f;
	positionXYZ = glm::vec3(xPos + 6.1, yPos + 22.05, zPos + 1.35f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("plastic");
	SetShaderMaterial("plastic");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawTaperedCylinderMesh(true, true, true);


	



}
void SceneManager::DrawSquirtBottle(float xPos, float yPos, float zPos)
{
	glm::vec3 scaleXYZ, positionXYZ;
	float XrotationDegrees, YrotationDegrees, ZrotationDegrees;
	

	/****************************************************************/
	/*** Drawing the Body of the Bottle ***/
	/****************************************************************/
	scaleXYZ = glm::vec3(0.5f, 2.0f, 0.5f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(xPos, yPos, zPos);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("plastic");
	SetShaderMaterial("plastic");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawCylinderMesh(false, true, true);

	/****************************************************************/
	/*** Drawing the Bottle Shoulder ***/
	/****************************************************************/
	scaleXYZ = glm::vec3(0.5f, 0.5f, 0.50f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 5.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(xPos, yPos + 2.0f, zPos);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	m_basicMeshes->DrawTaperedCylinderMesh(false, false, true);

	/****************************************************************/
	/*** Drawing the Before the Screw Area ***/
	/****************************************************************/
	scaleXYZ = glm::vec3(0.5f, 0.3f, 0.50f);
	XrotationDegrees = 180.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(xPos, yPos + 2.8f, zPos);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	m_basicMeshes->DrawTaperedCylinderMesh(false, false, true);

	/****************************************************************/
	/*** Drawing the Taped Cylinder the Nozzle ***/
	/****************************************************************/
	scaleXYZ = glm::vec3(0.5f, 0.5f, 0.5f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(xPos, yPos + 2.8f, zPos);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_basicMeshes->DrawTaperedCylinderMesh();

	/****************************************************************/
	/*** Drawing the Pyramid 1 for Nozzle ***/
	/****************************************************************/
	scaleXYZ = glm::vec3(0.4f, 1.0f, 1.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = -90.0f;
	positionXYZ = glm::vec3(xPos + 0.5f, yPos + 3.3f, zPos);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_basicMeshes->DrawPyramid4Mesh();

	/****************************************************************/
	/*** Drawing the Pyramid 2 for Nozzle ***/
	/****************************************************************/
	scaleXYZ = glm::vec3(0.4f, 1.0f, 1.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;
	positionXYZ = glm::vec3(xPos - 0.5f, yPos + 3.3f, zPos);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_basicMeshes->DrawPyramid4Mesh();

	/****************************************************************/
	/*** Drawing the Tapered Cylinder for Nozzle ***/
	/****************************************************************/
	scaleXYZ = glm::vec3(0.2f, 0.3f, 0.2f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = -90.0f;
	positionXYZ = glm::vec3(xPos + 0.7f, yPos + 3.3f, zPos);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.0f, 0.35f, 0.8f, 1.0f);
	m_basicMeshes->DrawTaperedCylinderMesh();

	/****************************************************************/
	/*** Drawing the Cylinder for Trigger ***/
	/****************************************************************/
	scaleXYZ = glm::vec3(0.05f, 0.25f, 0.05f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = -125.0f;
	positionXYZ = glm::vec3(xPos + 0.3f, yPos + 3.2f, zPos);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_basicMeshes->DrawCylinderMesh();

	/****************************************************************/
	/*** Drawing the Pyramid for Trigger ***/
	/****************************************************************/
	scaleXYZ = glm::vec3(0.2f, 0.4f, 0.2f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 160.0f;
	positionXYZ = glm::vec3(xPos + 0.5f, yPos + 3.1f, zPos);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.0f, 0.35f, 0.8f, 1.0f);
	m_basicMeshes->DrawPyramid3Mesh();

	/****************************************************************/
	/*** Drawing the Prism for Trigger ***/
	/****************************************************************/
	scaleXYZ = glm::vec3(0.1f, 0.1f, 0.7f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = 45.0f;
	ZrotationDegrees = 90.0f;
	positionXYZ = glm::vec3(xPos + 0.6f, yPos + 2.8f, zPos);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.0f, 0.35f, 0.8f, 1.0f);
	m_basicMeshes->DrawPyramid3Mesh();
}