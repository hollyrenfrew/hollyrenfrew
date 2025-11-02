#pragma once
#ifndef DATA_CALCULATIONS_HEADER
#define DATA_CALCULATIONS_HEADER

#include "InvestmentData.h"

class DataCalculation {
public:
	DataCalculation(InvestmentData investmentData);
	void YearlyBalances(int months, double openingAmount, double depositAmount, double monthlyInterest, char symbol);
private: 
	int yearWidth = 6;
	int balanceWidth = 24;
	int interestWidth = 30;
	int totalWidth = yearWidth + balanceWidth + interestWidth;

};

#endif