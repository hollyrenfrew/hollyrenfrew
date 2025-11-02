/*
 This program calculates based on entered initial investment amount,
 monthly deposit, annual interest (%), and number of years what the 
 year end balance and year end earned interest would be each year.

 Author: Holly Renfrew
 Date:   April 7, 2024
*/

#include <iostream>
using namespace std;

#include "InvestmentData.h"
#include "DataCalculation.h"
#include "UserInterface.h"

int main() { 
	InvestmentData inputInvestment;
	bool userContinue = true;
	while (userContinue) {
		// save all user input to inputInvestment
		inputInvestment = UserInterface::UserInput(inputInvestment);
		
		// add some space to make it look prettier
		cout << endl;
		system("pause");
		cout << endl;
		cout << endl;
		
		// calculate and show requested data based on input
		DataCalculation::DataCalculation(inputInvestment);

		//do you want to do it again? y or n function
		userContinue = UserInterface::ContinueQuestion();
	}
	return 0; 
}