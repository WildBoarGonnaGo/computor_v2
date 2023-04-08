#ifndef COMPUTOR_H
# define COMPUTOR_H


#include <string>
#include <vector>
#include <iostream>

class computor {
	//Max degree of equation
	int degree;
	//Number of corresponging polynoms
	std::vector<double> *polys;
	//Variable that signals if we deal with
	//the right or left side of equation
	int mirror;
	//Parsing iterator
	std::string::const_iterator it;
	//Source string with equation to parse
	const std::string &to_pass;
	//Result of equation solving
	std::string res;
	//Equation token to parse
	const std::string &token;
	//Error message string
	std::string errMsg;

	//Removing trailing zeros in string
	//represents number
	void del_trailer();
	//Get integer number of pow level. If number with a floating point
	//it returns error
	int get_int_number();
	//Get double value by parsing iterator
	double get_number();
	//Parsing source string, seeking errors and getting
	//corresponding polynominal values
	void fract();
	//Parse source and creating a new for result output
	void parse();
	//Passing whitespaces while passing source string
	void pass();
	//Returns syntax error if it occurs
	void syntax_err();
	//Adding solution to result string
	//in case of zero discriminant
	void solution1d();
	//Positive discriminant scenario
	void posdis(double dis);
	//Negative discriminant scenario
	void negdis(double dis);
	//Check equality
	void cheq();
	//Adding solution to result string
	//in case of non-zero discriminant
	void solution2d();
public:
	computor() = delete; //delete default constructor
	//Parametrized constructor. The parameters are
	//source string with equation and it's token
	computor(const std::string &src, const std::string &eqToken);
	//Return result as string
	const std::string& to_string();
	//Destructor
	~computor();
};

#endif //COMPUTOR_V2_COMPUTOR_H
