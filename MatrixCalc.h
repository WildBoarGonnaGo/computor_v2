//
// Created by WildBoarGonnaGo on 01.04.2023.
//

#ifndef MATRIXCALC_H
#define MATRIXCALC_H

#include "Matrix.h"
#include <stack>
#include <list>
#include <map>

class MatrixCalc {
	/*Variable structure. It has either matrix
	or regular equation. It also has state
	value: 0 - it's operation, 1 - it's matrix,
	2 - it's regular expression, 3 - it's token,
	 4 - it's error, 5 - it's brace, 6 - it's comma*/
	struct value {
		int state;
		Matrix matrix;
		std::string eq;
		//List of values in case of complex expressions
		std::list<value> lst;
		//Default constructor
		value() { }
		//Parametrized constructor with state only
		value(const int &s, const std::string &e = std::string(),
			  const Matrix &m = Matrix(),
			  const std::list<value> &v = std::list<value>()) :
			state(s), matrix(m), eq(e), lst(v) { }
		bool operator==(const value &s);
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
    //Executing functions with token parameter
    value funcExecuteTokenOrComplex(const std::string &oper, const value &var);
    //Expose user defined functions
    value ExposeUserDefFunc(const Func &f, const value &var);
	//Return norm of p degree
	void LPnorm();
    //Get matrix equation string and regular equation string
    //from source string
    void LpnormDev(const std::string &src, std::string &matEq, std::string &regEq,
                   const int &begin, const int &end);
	//Return infinite norm
	value LInfNorm(const Matrix &matrix);
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
	//std::string funcExpose(const Func &src, const std::string &forReplace);
	//Execute function block, no matter it's for matricies, regular expression
	//or it's some user defined
	value funcExprExec(const std::string &oper, std::stack<value> &nums);
	//Execute some base operands between two values
	value Execute(const std::string &oper, const value &first, const value &second);
    //Process both matricies
    value ProcessMatricies(const std::string &oper, const value &f, const value &s);
    //Process matrix and regular equation
    value ProcessMatrixAndRegEq(const std::string &oper, const value &f, const value &s);
    //Process regular equation and matrix
    value ProcessRegEqAndMatrix(const std::string &oper, const value &f, const value &s);
    //Process token and regular equation
    value ProcessTokenAndRegEq(const std::string &oper, const value &reg, const value &tokenVal);
    //Summing and subraction of regular equation and token
    value SumSubRegEqToken(const std::string &oper, const value &reg, const value &tokenVal,
                           const bool &isTokenFirst);
    //Complex summing or subtraction complex regular equation and token
    value SumSubComplexRegEqToken(const std::string &oper, const value &reg, const value &tokenVal,
                                  const bool &isTokenFirst);
    //Submethod for summing or subtraction regular equation and token, in case of token
    //is first value
    void SumSubComplexRegEqFirstToken(const std::string &oper, value &res, value &aux, bool &sign,
                                       bool &comp, bool &isToken);
    //Submethod for summing or subtraction regular equation and token, in case of token
    //is not first value
    void SumSubComplexRegEqIsNotFirstToken(const std::string &oper, value &res, value &aux, bool &sign,
                                       bool &comp, bool &isToken, const value &tokenVal);
    //Multiplication or divison of regex token
    value MultiDivRegExToken(const std::string &oper, const value &reg, const value &tok,
                             const bool &isTokenFirst);
    //Multiplication of complex regular expression and token
    value MultiComplexRegExToken(const value &reg, const value &tok, const bool &isTokenFirst);
    //Check and fix auxiliary value, due regular expression and token multiplication
    void CheckAndFixAuxValueToken(value &aux, const value &tok, const bool &isTokenFirst);
    //Power raising of token and regular equations
    value PowRaiseRegEqToken(const value &reg, const value &tok, const bool &isTokenFirst);
    //Computating both regular expression value
    value ProcessBothRegEq(const std::string &oper, const value &f, const value &s);
    //Computating first matrix variable and second token variable
    value ProcessMatrixAndToken(const std::string &oper, const value &f, const value &s);
    //Computating first token variable and second matrix variable
    value ProcessTokenAndMatrix(const std::string &oper, const value &f, const value &s);
    //Computating both token values
    value ProcessBothTokens(const std::string &oper, const value &f, const value &s);
	//Matricies summing and subtraction
	value MatriciesSumSub(const std::string &oper, const value &first, const value &second);
	//Matrix elements multiply results for socket
	value MatrixSocketMultiply(const Matrix &f, const Matrix &s, int row, int column);
	//Matricies multiplication
	value MatrixMulti(const value &f, const value &s);
	//Number and Matrix multiplication. 'f' is a regular expression, 's' is a matrix
	value MatrixNumMulti(const value &f, const value &s);
	//Matrix power raising. 'f' is a matrix, 's' should be integer number
	value MatrixPowerRaise(const value &f, const value &s);
	//Return Unit Matrix
	value UnitMatrix(const int &row, const int &column);
    //Complex matrix power raise. 'f' is a matrix, 's' should be integer number
    value ComplexMatrixPowerRaise(const value &f, const value &s);
	//Denomination elements for regular expression
	std::list<std::string> DenomElems(const std::string &src);
	//Generating string in case of error, or in case of result
	std::string FinResGenerate(const value &val, const bool &isError);
	//Multiplication of token and matrix
	value MultiTokenMatrix(const value &f, const value &s);
	//Add operation for token and matrix
	value AddOperTokenMatrix(const std::string &oper, const value &f,
							 const value &s);
	/*Multiplying complex matrix equation and regular expression
	 'm' represents matrix, 'r' represents regular expression*/
	value MatrixVectorNumMulti(const value &m, const value &r);
	//Summing or Subtracting of complex matrix and simple matrix
	value ComplexSimpleMatrixSumSub(const value &cm, const value &sm,
									const std::string &oper);
	//Summing or Subtracting of simple matrix and complex matrix
	value SimpleComplexMatrixSumSub(const value &sm, const value &cm,
									const std::string &oper);
	//Summing and subtracting in case when both values are complex matrix equations
	value ComplexComplexMatrixSumSub(const value &fcm, const value &scm,
									 const std::string &oper);
	//Multiplying of simple matrix and complex matrix equation
	value SimpleComplexMatrixMulti(const value &f, const value &s);
    //Multiplying of complex matrix equation and simple matrix
    value ComplexSimpleMatrixMulti(const value &f, const value &s);
    //Multiplying of complex matrix equations
    value ComplexComplexMatrixMulti(const value &f, const value &s);
    //Analizing and modifying complex source value
    value AnalizeModifyValue(value &&src);
	//Summing or subracting regular equations
	value SumSubRegEq(const std::string &oper, const value &f, const value &s);
	//Summing or subracting simple regular equation and
	//complex regular equation
	value SumSubSimpleComplexRegEq(const std::string &oper, const value &f, const value &s);
	//Summing or subtracting complex regular equation and
	//simple regular equation
	value SumSubComplexSimpleRegEq(const std::string &oper, const value &f, const value &s);
	//Summing or subtracting both complex regular expression
	value SumSubComplexComplexRegEq(const std::string &oper, const value &f, const value &s, const int &state);
	//Complex reg values values analizer for summing and subtraction
	value ComplexRegEqAnalyzerSumSub(const std::string &oper, const value &f, const value &s);
	//Iterate over the second value for summing and subtraction
	value IterateOverSecValueSumSub(const std::string &oper, const value &auxF, const value &s,
									const bool &signF, const bool &simF,
									std::list<std::list<value>::const_iterator> &auxLst,
									std::list<std::list<value>::const_iterator> &genLst,
									const value &res, const int &state);
	//Check value if it has zero values
	void CheckZero(value &src);
	//Iterate second value components to
	//generate final complex expression
	void IterateOverSecValueFinGen(value &res,
								   const std::list<std::list<value>::const_iterator> &genLst,
								   const std::string &oper, const value &s);
	//Multiplication of dividing regular equations
	value MultiDivRegEq(const std::string &oper, const value &f, const value &s);
	//Multiplicatoin and Division of simple regular equation and complex regular equation
	value MultiDivSimpleComplexRegEq(const std::string &oper, const value &f, const value &s);
	//Does complex value have multiple elements (with summing or subtraction operations)
	bool DoesComplexValHaveMultiple(const value &f);
	//Case of division simple value by complex value (both values are regular equations)
	value DivisionSimpleByComplexRegEq(const value &f, const value &s);
	//Case of multiplication or division of complex regular equations
	value MultiDivBothComplexRegEq(const std::string &oper, const value &f, const value &s);
	//Multiplication or Division of complex regular equation with multiple elements and complex
	//regular equation with one element
	value MultiDivComplexRegEqs(const std::string &oper, const value &f, const value &s,
								const bool &mf, const bool &ms);
	//Analyze element of regular equation for multiplications or division
	value AnalyzeForMultiDiv(const value &src, value &m, value &p, const int &state);
	//Equation simplifier and analyzer for source value
	value EqAnalyzeSimplify(value &&src);
    //regular equation analyzer and simplifier for
    //source value second iteration
    value RegEqAnalyzeSimplifySecondIt(value &src, std::list<value>::iterator &it,
                                       const int &state, const value &aux);
    //Matrix analyzer and simplifier for source value second
    //iteration
    value MatrixAnalyzeSimplifySecondIt(value &src, std::list<value>::iterator &it,
                                        const int &state, const value &aux);
	//Regular equation power raising
    value PowRaiseRegEq(const value &f, const value &s);
    //Multiplication of complex regular expression and simple matrix
    value MultiComplexRegEqSimpleMatrix(const value &r, const value &m);
    //Mutliplication of complex regular expression and complex matrix
    value MultiComplexRegEqComplexMatrix(const value &r, const value &m);
    //Check zero and unit values in complex equation
    void CheckZeroAndUnitsInComplexValue(value &src);
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
	//Get result of calculations as a string
	const std::string getCalcResult() const;
};


#endif //MATRIXCALC_H
