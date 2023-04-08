#include "Matrix.h"

//Default constructor
Matrix::Matrix() { }

//Parsing source string
int Matrix::Parse() {
	//brace control
	int brace = 0;
	//prompt runner
	int i = 0;
	//polish notation parser
	RevPolNotation pol(funcs);
	//Column calculation for first time
	bool columnCalc = false;

	//Set token for regular expression solver
	pol.setToken(token);
	//Pass whitespaces
	while (std::isspace(matrix[i])) ++i;
	//Check if symbol is a left squarebrace. If it's
	//not, error returns;
	if (matrix[i] != '[')
		return RetError("\nerror: there should be left squarebrace", i, matrix);
	++brace;
	for (i = 0; i < matrix.size(); ++i) {
		//Passing whitespaces
		while (std::isspace(matrix[i])) ++i;
		//Parsing row
		if (matrix[i] == '[' and brace == 1) {
			//number of elements in row
			int rowElems = 0;
			++brace;
			++i;
			++row;
			while (matrix[i] != ']') {
				//Auxiliary result string
				std::string aux;
				//previous position
				int prev = i;

				//Pass whitespaces
				while (std::isspace(matrix[i])) ++i;
				//find position of next non-digit
				i = matrix.find(",]", i);
				//If there is no comma, return error
				if (i == std::string::npos)
					return RetError("\nerror: there should be comma character or right squarebrace", prev + 1, matrix);
				//aux = matrix.substr(prev, i);
				//If character is squarebrace
				if (matrix[i] == ']') {
					//Parse number
					pol.setInfixExpr(matrix.substr(prev, i));
					//Check is string contain any information
					//If it doesn't, return error
					if (pol.getPosfixExpr().empty())
						RetError("\nerror: there is no information whatsoever", i, matrix);
					--brace;
					++rowElems;
					//If there was no elements in a row, return error
					//if (!rowElems)
					//	return RetError("\nerror: there is no elements in a row", i, matrix);
					//If it's first row set number of columns
					if (!columnCalc) {
						column = rowElems;
						columnCalc = true;
					}
					//Else, we check number of elemnts
					//in further rows. If number is not equal
					//to number of elements on the first row
					//return error
					if (column != rowElems)
						return RetError("\nerror: number of columns are not the same in current matrix", i, matrix);
					break ;
				}
				//aux = matrix.substr(prev, i);
				//Parse number
				pol.setInfixExpr(matrix.substr(prev, i));
				//Check is string contain any information
				//If it doesn't, return error
				if (pol.getPosfixExpr().empty())
					RetError("\nerror: there is no information whatsoever", i, matrix);
				//If there is error, stop parsing and return it
				if (!pol.getErrMsg().empty())
					{ error = pol.getErrMsg(); return 0; }
				//Push number to vector
				values.push_back(pol.CalcIt());
				//Pass whitespaces
				//while (std::isspace(matrix[i])) ++i;
				//Check if it's comma. If it's not, return error
				++i;
				++rowElems;
			}
			--brace;
			//Pass whitespaces
			while (std::isspace(matrix[i])) ++i;
			//If character is not semicolon or right squarebrace, return error
			if (matrix[i] != ';' || matrix[i] != ']')
				return RetError("\nerror: there should be semicolon or right squarebrace", i, matrix);
			//If we deal with final right squarebrace, break the loop
			else if (matrix[i] == ']') {
				--brace;
				break ;
			}
		}
		//If number of left squarebraces is greater than 2
		//return error
		else if (matrix[i] == '[' && brace == 2)
			return RetError("\nerror: found extra squarebrace", i, matrix);
		//If number of right squarebraces is greater then number
		//of left squarebraces return error
		else if (matrix[i] == ']' && !brace)
			return RetError("\nerror: number right squarebraces is greater than left", i, matrix);
		//If there is any other character return error
		else
			return RetError("\nerror: some obscure character found", i, matrix);
	}
	return 1;
}

//Private method. It generates error string and mark error
//position in source string
int Matrix::RetError(const std::string &errMsg,
			 const int &i, const std::string &src) {
	error = src;
	error.push_back('\n');
	error.insert(error.size(), i - 1, ' ');
	error.push_back('^');
	error += errMsg;
	return 0;
}

//Parametrized constructor
Matrix::Matrix(const std::string &src,
	   std::map<std::string, Func> &funcSrc,
	   std::map<std::string, Matrix> &matrixSrc,
		const std::string &tokenSrc, const int &tokenIsMatrixSrc) :
	   matrix(src), row(0), column(0), funcs(funcSrc), matricies(matrixSrc),
	   token(tokenSrc), tokenIsMatrix(tokenIsMatrixSrc) {
	//Set of basic matrix functions
	std::set<std::string> baseFuncsMatrix = { "inv", "transp", "l1norm", "l2norm" };

	//Let's check if token is matrix, if it's return error
	if (!token.empty() && matrix.find(token) != std::string::npos && tokenIsMatrix == 1) {
		RetError("\nerror: there is token and it's matrix", matrix.find(token), matrix);
		return ;
	}
	//If token state is not defined yet, set it to regular expression
	if (!token.empty() && matrix.find(token) != std::string::npos && !tokenIsMatrix)
		tokenIsMatrix = 1;
	//Let's check, is there some matrix, inside the matrix
	//If there is, return error
	for (std::map<std::string, Matrix>::iterator it = matricies.begin();
		it != matricies.end(); ++it) {
		int indx = matrix.find(it->first);
		if (indx != std::string::npos) {
			RetError("\nerror: matrix '" + it->first + "' inside the matrix", indx, matrix);
			return ;
		}
	}
	// Check if there is some matrix function in user defined functions.
	// If there is return error
	for (std::map<std::string, Func>::iterator it = funcs.begin();
		 it != funcs.end(); ++it) {
		int indx = matrix.find(it->first);
		if (indx != std::string::npos && it->second.isMatrix) {
			RetError("\nerror: matrix function '" + it->first + "' inside the matrix", indx, matrix);
			return ;
		}
	}
	// Check if there is some matrix function in base set of functions.
	for (std::set<std::string>::iterator it = baseFuncsMatrix.begin();
		it != baseFuncsMatrix.end(); ++it) {
		int indx = matrix.find(*it);
		if (indx != std::string::npos) {
			RetError("\nerror: matrix function '" + *it + "' inside the matrix", indx, matrix);
			return ;
		}
	}
	// If there is return error
	//Let's parse source string. If there is some error
	//assign error string to result and exit constructor.
	//Else, generate result string
	if (!Parse()) {
		values.clear();
		row = 0; column = 0;
		output = error;
		return ;
	}
	//Generating output string in other case
	for (int i = 0; i < row; ++i) {
		output.append("[ ");
		for (int j = 0; j < column; ++j) {
			output.append(values[i + j]);
			output.append((j == column - 1) ? " ]\n" : " , ");
		}
	}
}

Matrix::Matrix(const std::vector<std::string> &src,
		   const int &rowSrc, const int &columnSrc,
		   const std::string &tokenSrc,
		   const int &tokenIsMatrixSrc) : row(rowSrc), column(columnSrc),
		   values(src), token(tokenSrc), tokenIsMatrix(tokenIsMatrixSrc) {
	//matrix.push_back('['); output.push_back('[');
	matrix.append("[ ");
	for (int i = 0; i < row; ++i) {
		matrix.append("[ ");
		output.append("[ ");
		for (int j = 0; j < column; ++j) {
			matrix.append(values[i * row + column] + ((j == column - 1) ? " " : ", "));
			output.append(values[i * row + column] + ((j == column - 1) ? " " : " , "));
		}
		matrix.append(((i == row - 1) ? "] ]": "]; "));
		output.append("]\n");
	}
}

//Get error string
const std::string &Matrix::getError() const { return error; }

//Get result string. If there is some error
//return error string instead
const std::string &Matrix::toString() const { return output; }

//Get number of rows
const int &Matrix::getRow() const { return row; }

//Get number of columns
const int &Matrix::getColumn() const { return column; }

//Get matrix infix notation
const std::string & Matrix::getMatrix() const { return matrix; }

//Set matrix infix notation
void Matrix::setMatrix(const std::string &src,
			   std::map<std::string, Func> &funcSrc,
			   std::map<std::string, Matrix> &matrixSrc) {
	matrix = src; funcs = funcSrc; matricies = matrixSrc;
	//Let's check, is there some matrix, inside the matrix
	//If there is, return error
	for (std::map<std::string, Matrix>::iterator it = matricies.begin();
		 it != matricies.end(); ++it) {
		int indx = matrix.find(it->first);
		if (indx != std::string::npos) {
			RetError("\nerror: matrix '" + it->first + "' inside the matrix", indx, matrix);
			return ;
		}
	}
	//Let's parse source string. If there is some error
	//assign error string to result and exit constructor.
	//Else, generate result string
	if (!Parse()) {
		output = error;
		return ;
	}
	//Generating output string in other case
	for (int i = 0; i < row; ++i) {
		output.append("[ ");
		for (int j = 0; j < column; ++j) {
			output.append(values[i + j]);
			output.append((j == column - 1) ? " ]\n" : " , ");
		}
	}
}

//Set token value
void Matrix::setToken(const std::string &token) { this->token = token; }

//Set tokenIsMatrix value
void Matrix::setTokenIsMatrix(const int &val) { tokenIsMatrix = val; }

//Get tokenIsMatrix value
const int &Matrix::getTokenIsMatrix() const { return tokenIsMatrix; }

//Get vector of values
const std::vector<std::string> &Matrix::getValues() const { return values; }