#ifndef MATRIX_H
#define MATRIX_H

#include "RevPolNotation.h"
#include <vector>
#include <string>

class Matrix {
	//Number of rows
	int row;
	//Number of columns
	int column;
	//Matrix values
	std::vector<std::string> values;
	//Matrix string
	std::string matrix;
	//error string
	std::string error;
	//Set of functions. Reference value
	std::map<std::string, Func> funcs;
	//Set of values. Reference value
	//std::map<std::string, std::string> &vars;
	//Set of matricies. Reference value
	std::map<std::string, Matrix> matricies;
	//Result string for output
	std::string output;
	//Token string
	std::string token;
	//Is token a matrix 0 - not defined, 1 - matrix,
	//2 - regular variable
	int tokenIsMatrix;

	//Parsing source string
	int Parse();
	//Private method. It generates error string and mark error
	//position in source string
	int RetError(const std::string &errMsg,
				 const int &i, const std::string &src);
public:
	Matrix();
	//Parametrized constructor
	Matrix(const std::string &src,
		   std::map<std::string, Func> &funcSrc,
		   std::map<std::string, Matrix> &matrixSrc,
		   const std::string &tokenSrc = std::string(),
		   const int &tokenIsMatrixSrc = 0);
	//Parametrized constructor. It takes values and generate matrix
	//string, and output string
	Matrix(const std::vector<std::string> &src,
		   const int &rowSrc, const int &columnSrc,
		   const std::string &tokenSrc = std::string(),
		   const int &tokenIsMatrixSrc = 0);
	//Get error string
	const std::string &getError() const;
	//Get result string. If there is some error
	//return error string instead
	const std::string &toString() const;
	//Get number of rows
	const int &getRow() const;
	//Get number of columns
	const int &getColumn() const;
	//Get matrix infix notation
	const std::string &getMatrix() const;
	//Set matrix infix notation
	void setMatrix(const std::string &src,
				   std::map<std::string, Func> &funcSrc,
				   std::map<std::string, Matrix> &matrixSrc);
	//Set token value
	void setToken(const std::string &token);
	//Set tokenIsMatrix value
	void setTokenIsMatrix(const int &val);
	//Get tokenIsMatrix value
	const int &getTokenIsMatrix() const;
	//Get vector of values
	const std::vector<std::string> &getValues() const;
};


#endif //COMPUTOR_V2_MATRIX_H
