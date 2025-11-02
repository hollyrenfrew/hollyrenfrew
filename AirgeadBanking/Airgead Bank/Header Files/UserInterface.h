#pragma once
#ifndef USER_INTERFACE_HEADER
#define USER_INTERFACE_HEADER

#include "DataCalculation.h"

class UserInterface {
public:
	static InvestmentData UserInput(InvestmentData inputInvestment);
	static void inputError(string input);
	static bool ContinueQuestion();
};

#endif