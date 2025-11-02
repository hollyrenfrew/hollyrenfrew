#include <iostream>
using namespace std;

#include "UserInterface.h"

void UserInterface::inputError(string input) {
	cerr << "Wrong input... Please enter a valid " << input << endl;
	cin.clear();
	while (cin.get() != '\n');
}

InvestmentData UserInterface::UserInput(InvestmentData inputInvestment) {
	int years = 0;
	int amount = 0;
	bool validator = false;
	char symbol = ' ';
	cout << string(42, '*') << endl;
	cout << string(15, '*') << " Data Input " << string(15, '*') << endl;

	/* Get currency symbol from user */
	while (!validator) {
		cout << "Currency Symbol: ";
		if (cin >> symbol) {
			inputInvestment.SetCurrency(symbol); //set input as initial investment
			validator = true;
		}
		else {
			UserInterface::inputError("symbol");
		}
	}
	// clear any extra input
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	validator = false;

	/* Get initial investment from user.*/
	while (!validator) {
		cout << "Initial Investment Amount: ";
		if (cin >> amount) {
			inputInvestment.SetInitialInvestment(amount); //set input as initial investment
			validator = true;
		}
		else {
			UserInterface::inputError("number");
		}
	}
	// clear any extra input
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	validator = false;

	/* Get monthly deposit from user */
	while (!validator) {
		cout << "Monthly Deposit: ";
		if (cin >> amount) {
			inputInvestment.SetMonthlyDeposit(amount); //set input as monthly deposit
			validator = true;
		}
		else {
			UserInterface::inputError("number");
		}
	}
	// clear any extra input
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	validator = false;

	/* Get annual interest from user */
	while (!validator) {
		cout << "Annual Interest (%): ";
		if (cin >> amount) {
			inputInvestment.SetAnnualInterest(amount); //set input as annual interest
			validator = true;
		}
		else {
			UserInterface::inputError("number");
		}
	}
	// clear any extra input
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	validator = false;

	/* Get number of years from user */
	while (!validator) {
		cout << "Years: ";
		if (cin >> years) {
			inputInvestment.SetYears(years); //set input as number of years
			validator = true;
		}
		else {
			UserInterface::inputError("number");
		}
	}
	// clear any extra input
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	validator = false;
	cout << endl;
	return inputInvestment;
}


bool UserInterface::ContinueQuestion() {
	bool user_answer = true;
	char answer = ' ';
	cout << endl;
	while (answer != 'y' && answer != 'Y' && answer != 'n' && answer != 'N') {
		cout << "Would you like to continue? Enter y or n." << endl;
		cin >> answer;
		while (cin.get() != '\n');
	}
	if (answer == 'n' || answer == 'N') {
		cout << endl;
		user_answer = false;
	}
	// clear any extra input
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	return user_answer;
};