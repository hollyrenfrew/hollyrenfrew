#include <iostream>
using namespace std;

#include "InvestmentData.h"

/* Default constructor */
InvestmentData::InvestmentData() {
	initial_investment = 0;
	monthly_deposit = 0;
	annual_interest = 0;
	number_of_years = 0;
	currency = '$';
}

/* Investment Construction */
void InvestmentData::SetInitialInvestment(double investment) {
	initial_investment = investment;
}

double InvestmentData::GetIntialInvestment() {
	return initial_investment;
}

/* Deposit Construction */
void InvestmentData::SetMonthlyDeposit(double deposit) {
	monthly_deposit = deposit;
}

double InvestmentData::GetMonthlyDeposit() {
	return monthly_deposit;
}

/* Interest Construction */
void InvestmentData::SetAnnualInterest(double interest) {
	annual_interest = interest;
}

double InvestmentData::GetAnnualInterest() {
	return annual_interest;
}

/* Year Construction */
void InvestmentData::SetYears(int years) {
	number_of_years = years;
}

int InvestmentData::GetYears() {
	return number_of_years;
}


/* Currency Construction */
void InvestmentData::SetCurrency(char curr) {
	currency = curr;
}

char InvestmentData::GetCurrency() {
	return currency;
}