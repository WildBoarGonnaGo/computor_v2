//
// Created by WildBoarGonnaGo on 01.04.2023.
//

#ifndef MATRIXCALC_H
#define MATRIXCALC_H

#include "Matrix.h"

class MatrixCalc {
	/*Variable structure. It has either matrix
	or regular equation. It also has state
	value: 0 - it's operation, 1 - it's matrix,
	2 - it's regular expression, 3 - it's token,
	 4 - it's error*/
	struct value {
		int state;
		Matrix matrix;
		std::string eq;
	};

	//Set of functions. Reference value
	std::map<std::string, Func> &funcs;
	//Set of values. Reference value
	std::map<std::string, Matrix> &matricies;
	//Infix notation of equation
	std::string infixEq;
	//Error message
	std::string error;
	//Set of base matrix operations
	std::string baseOpers;
	//Set of base functions for regular expressionss
	std::set<std::string> baseFuncsReg;
	//Set of base functions for matrix
	std::set<std::string> baseFuncsMatrix;
	//Set of operation priority
	std::map<std::string, int> operPriority;
	//Postfix queue of values, operations, functions and matricies
	std::list<value> postfixQueue;
	//Token in case of function
	std::string token;
	//Result string that strores your calculation result
	std::string calcResult;
	//bool variable, that checks calculation errors
	bool calcError;
	//Is token a matrix. 0 - not defined, 1 - matrix,
	//2 - regular variable
	int tokenIsMatrix;
	//Calculated value
	value finCalc;

	//Making a postfix string for further calculations
	void ProcessPostfix();
	//Seeking implicit multiplication in expression
	void StrMultiplySearch(std::string &src, const std::string &token);
	//Get double number from string
	std::string	GetStringNumber(const std::string &str, int &i);
	//Parse matrix in source string. If error, return 0
	int ParseMatrix(int &i);
	//Marks error place
	std::string MarkError(const std::string &src, int &i);
	//Private method. It executes base and user defined functions
	value funcExecute(const std::string &oper, const value &var);
	//Return l1norm value result
	value L1norm(const Matrix &matrix);
	//Return l2norm value result
	value L2norm(const Matrix &matrix);
	//Return result of matrix transpose
	value Transpose(const Matrix &matrix);
	//Determinant of matrix
	value Det(const Matrix &matrix);
	//Adjoint matrix
	value Adj(const Matrix &src);
	//Inverse matrix
	value Inv(const Matrix &src);
	//Expose function and replace token
	std::string funcExpose(const Func &src, const std::string &forReplace);
public:
	//Default constructor
	MatrixCalc() = delete;
	//Parametrized constructor
	MatrixCalc(std::map<std::string, Func> &funcsSrc,
			   std::map<std::string, Matrix> &matriciesSrc,
			   const std::string &src,
			   const std::string &tokenSrc = std::string());
	//Calculation of postfix equation
	std::string CalcIt();
	//Get error string
	const std::string &getError() const;
	//Get final value
	const value getFinValue() const;
};


#endif //MATRIXCALC_H
