#include <iostream>
#include <iomanip>
#include <sstream>
using namespace std;

#include "DataCalculation.h"



DataCalculation::DataCalculation(InvestmentData investmentData) {
	int months = investmentData.GetYears() * 12;
	double openingAmount = investmentData.GetIntialInvestment();
	double annualInterest = investmentData.GetAnnualInterest();
	double monthlyInterest = (annualInterest / 100) / 12;
	double depositAmount = investmentData.GetMonthlyDeposit();

	cout << setw(totalWidth) << "Balance and Interest Without Monthly Deposits" << endl;
	cout << string(totalWidth, '=') << endl;
	DataCalculation::YearlyBalances(months, openingAmount, 0, monthlyInterest, investmentData.GetCurrency());
	
	cout << endl;
	cout << endl;
	cout << setw(totalWidth) << "Balance and Interest With Monthly Deposits" << endl;
	cout << string(totalWidth, '=') << endl;
	DataCalculation::YearlyBalances(months, openingAmount, depositAmount, monthlyInterest, investmentData.GetCurrency());
}

void DataCalculation::YearlyBalances(int months, double openingAmount, double depositAmount, double monthlyInterest, char symbol) {
	int i = 1;
	double total = 0;
	double totalInterest = 0;
	double closingBalance = 0;
	int year = 0;
	double interestTotal = 0;
	cout << setw(yearWidth) << "Year";
	cout << setw(interestWidth) << "Yearly Interest";
	cout << setw(balanceWidth) << "Year End Balance";
	cout << endl;
	for (i; i <= months; i++) {
		total = openingAmount + depositAmount;
		totalInterest = total * monthlyInterest;
		closingBalance = total + totalInterest;
		interestTotal = totalInterest + interestTotal;

		if (i % 12 == 0) {
			// create strings to print to screen from doubles and the symbol character
			stringstream closingStream;
			closingStream << fixed << setprecision(2) << closingBalance;
			string closing = symbol + closingStream.str();
			stringstream interestStream;
			interestStream << fixed << setprecision(2) << interestTotal;
			string interest = symbol + interestStream.str();

			year = i / 12;
			totalInterest = totalInterest * 12;
			cout << setw(yearWidth) << year;
			cout << setw(interestWidth) << interest;
			cout << setw(balanceWidth) << closing;
			cout << endl;
			interestTotal = 0;
		}
		openingAmount = closingBalance;
	}
}
