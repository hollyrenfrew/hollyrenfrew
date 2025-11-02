#pragma once
#ifndef INVESTMENT_DATA_HEADER
#define INVESTMENT_DATA_HEADER

class InvestmentData {
public:
	InvestmentData(); // constructor
	
	/* Mutators */
	void SetInitialInvestment(double investment);
	void SetMonthlyDeposit(double deposit);
	void SetAnnualInterest(double interest);
	void SetYears(int years);
	void SetCurrency(char curr);

	/* Accessors */
	double GetIntialInvestment();
	double GetMonthlyDeposit();
	double GetAnnualInterest();
	int GetYears();
	char GetCurrency();

private: // defining variables
	double initial_investment;
	double monthly_deposit;
	double annual_interest;
	int number_of_years;
	char currency;
};

#endif