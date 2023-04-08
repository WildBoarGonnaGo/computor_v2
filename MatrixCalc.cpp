//
// Created by WildBoarGonnaGo on 01.04.2023.
//

#include "MatrixCalc.h"
#include <stack>
#include <cmath>

//Get double number from string
std::string	MatrixCalc::GetStringNumber(const std::string &str, int &i) {
	std::string 	res;

	for (; i < str.length(); ++i) {
		char	tmp;
		int		dot = 0;

		tmp = str[i];
		if (isdigit(tmp)) res.push_back(tmp);
		else if (tmp == '.' && !dot) { res.push_back(tmp); ++dot; }
		else if (tmp == '.' && dot) { res.clear(); return res; }
		else break ;
	}
	//move carriage
	--i;
	if (!res.compare(".")) res.clear();
	return res;
}

//Parse matrix in source string. If error, return 0
int MatrixCalc::ParseMatrix(int &i) {
	value matrixUnit;
	std::string res;
	int brace = 0, prev;

	prev = i;
	//Generating matrix string
	for ( ; i < infixEq.size(); ++i) {
		if (infixEq[i] == '[') ++brace;
		else if (infixEq[i] == ']') --brace;
		res.push_back(infixEq[i]);
		if (!brace) break;
	}
	//If wrong number of braces, return error
	if (brace) {
		error = MarkError(infixEq, prev) + "\nerror: wrong matrix syntax";
		return 0;
	}
	//If there is some error in matrix, return error
	matrixUnit.matrix.setMatrix(res, funcs, matricies);
	if (!matrixUnit.matrix.getError().empty()) {
		error = matrixUnit.matrix.getError();
		return 0;
	}
	matrixUnit.state = 1;
	postfixQueue.push_back(matrixUnit);
	return 1;
}

//Marks error place
std::string MatrixCalc::MarkError(const std::string &src, int &i) {
	std::string res = src;
	res.push_back('\n');
	res.insert(res.size(), i, ' ');
	res.push_back('^');
	return res;
}

//Making a postfix string for further calculations
void MatrixCalc::ProcessPostfix() {
	//Result string
	std::string				res;
	//Stack operators (string variables)
	std::stack<std::string>	oper;
	//Brace control
	int 					brace = 0;
	//function brace control
	int						funcBrace = 0;
	//Data queue control
	bool					number = false;
	//lambda function. It marks error place
	/*auto MarkError = [](const std::string &src, int i) {
		std::string res = src;
		res.push_back('\n');
		res.insert(res.size(), i, ' ');
		res.push_back('^');
		return res;
	};*/
	//Parsing string
	for (int i = 0; i < infixEq.size(); ++i) {
		//If character is digit
		if (std::isdigit(infixEq[i]) || infixEq[i] == '.') {
			//Initiate new number value
			value numberUnit;
			if (!number) number = true;
			else {
				error = MarkError(infixEq, i) + "\nerror: wrong order: there should be a number";
				postfixQueue.clear(); return ;
			}
			//res += GetStringNumber(infixEq,i) + " ";
			numberUnit.eq = GetStringNumber(infixEq, i);
			numberUnit.state = 2;
			if (numberUnit.eq.empty()) {
				//std::cerr << "wrong number syntax" << std::endl;
				error = MarkError(infixEq, i) + "\nerror: wrong number syntax";
				postfixQueue.clear(); return ;
			}
			postfixQueue.push_back(numberUnit);
		}
		//if character is a left squarebrace
		else if (infixEq[i] == '[')
			if (!ParseMatrix(i)) { postfixQueue.clear(); return ; }
		//if character is a left braced
		else if (infixEq[i] == '(') {
			//Push it into stack and announcing about a left brace
			std::string brace_str;
			//If there is nothing between left and right braces
			//there should be error
			if (infixEq[i + 1] == ')') {
				error = MarkError(infixEq, i) + "\nerror: there is no content between left and right braces";
				postfixQueue.clear(); return ;
			}
			brace_str.push_back(infixEq[i]);
			oper.push(brace_str); ++brace;
		}
			//Check a wrong right brace case
		else if (infixEq[i] == ')' && !brace) {
			//std::cerr << "extra braces found" << std::endl;
			error = MarkError(infixEq, i) + "\nerror: extra brace found";
			postfixQueue.clear(); return ;
		}
		//Check correct right brace case
		else if (infixEq[i] == ')') {
			//Check if ther was some function encounter in previous iteration
			//if there was error shoud be marked
			if (funcBrace == brace) {
				//non brace and space next character position
				int charPos = infixEq.find_first_not_of(")	 ", i);
				//check if we deal with end of the expression
				//if we do, just pass it, we check next character
				//is operator
				if (charPos != std::string::npos) {
					//If there no operator, we mark error, and stop infix expression processing
					if (auto search = baseOpers.find(infixEq[charPos]); search == std::string::npos) {
						error = MarkError(infixEq, charPos) + "\nerror: there should be some base operator";
						postfixQueue.clear(); return ;
					}
				}
				--funcBrace;
			}
			//Pop every operator and function until we stuck at left brace
			while (!oper.empty() && oper.top().compare("(")) {
				value operUnit;
				operUnit.eq = oper.top();
				operUnit.state = 0;
				//res += oper.top();
				postfixQueue.push_back(operUnit);
				oper.pop(); //res.push_back(' ');
			}
			oper.pop();
			--brace;
		}
		//if character is alphabetical
		else if (std::isalpha(infixEq[i]) || baseOpers.find(infixEq[i]) != std::string::npos) {
			//char	tmp = infixEq[i];
			//Assign character to 'c' variable
			char c = infixEq[i];
			//Generating string (tmp) while we deal with alphabetical characters
			std::string	tmp;
			while (std::isalpha(infixEq[i]))
				tmp.push_back(infixEq[i++]);

			//Move carriage back if there were alpha characters
			//If there weren't, we deal with binary operator
			//and it should be pushed into 'tmp' string. If there were alpha characters
			//check if we deal with function and there is a left brace
			//as a next character
			if (infixEq[i] != c) {
				//Search funcs in basic functions set
				auto search = operPriority.find(tmp);
				//Search func in user defined function set
				auto uDSearch = funcs.find(tmp);
				//Search matrix in matricies set
				auto matrixSearch = matricies.find(tmp);
				//lambda function check operator or function and operands order
				auto errorRet = [](bool &number, const std::string &errstr,
								   std::list<value> &lst, const value &add, int &indx) {
					if (!number) number = true;
					else {
						lst.clear();
						return errstr;
					}
					//res += add + " ";
					lst.push_back(add);
					--indx;
					return std::string();
				};
				//Passing whitespaces
				while (std::isspace(infixEq[i])) ++i;
				//If we got pi number, check operators and operands order
				if (tmp.size() == 2 && !tmp.compare("pi")) {
					//Generate value to push into postfix queue
					value piNumber; piNumber.state = 2; piNumber.eq = std::to_string(M_PI);
					//Check operation and operands order
					error = errorRet(number, "\nerror: wrong order: there should be operator or function", postfixQueue,
									  piNumber, i);
					//If error occured, mark place where error occured
					if (!error.empty()) error.insert(0, MarkError(infixEq, i));
					if (postfixQueue.empty()) return ;
					continue ;
				}
				//Check if paramater is a token
				else if (!token.empty() && tmp.size() == token.size() && !tmp.compare(token)) {
					//Generate value to push into postfix queue
					value tokenValue; tokenValue.state = 3; tokenValue.eq = token;
					//Check operations and operands order
					error = errorRet(number, "\nerror: wrong order: there should be operator or function", postfixQueue,
									 tokenValue, i);
					//If error occured, mark place where error occured
					if (!error.empty()) error.insert(0, MarkError(infixEq, i));
					if (postfixQueue.empty()) return ;
					continue ;
				}
				//Check if character is a complex number
				else if (tmp.size() == 1 && !tmp.compare("i")) {
					//Generate value to push into postfix queue
					value complex; complex.state = 2; complex.eq = tmp;
					//Check operand, operator order
					error = errorRet(number, "\nerror: wrong order: there should be operator or function", postfixQueue,
									 complex, i);
					//If error occured, mark place where error occured
					if (!error.empty()) error.insert(0, MarkError(infixEq, i));
					if (postfixQueue.empty()) return ;
					continue ;
				}
				//Check if parameter is a matrix
				else if (matrixSearch != matricies.end()) {
					//Generate value to push into postfix queue
					value matrix; matrix.state = 1; matrix.matrix = matricies[tmp];
					//Check operand, operator order
					error = errorRet(number, "\nerror: wrong order: there should be operator or function", postfixQueue,
									 matrix, i);
					//If error occured, mark place where error occured
					if (!error.empty()) error.insert(0, MarkError(infixEq, i));
					if (postfixQueue.empty()) return ;
					continue ;
				}
				//Check if we deal with user defined function and next character
				//is a left brace. If we deal with it, we push function into
				//operator stack, otherwise we check if we deal with base functions
				else if (uDSearch != funcs.end() && infixEq[i] == '(') {
					oper.push(tmp); --i; ++funcBrace; continue;
				}
				//Check if we deal with some base function and next character
				//is a left brace. If we deal with it, we push function into
				//operator stack, otherwise we check if we deal with base functions
				else if (search != operPriority.end() && infixEq[i] == '(') {
					oper.push(tmp); --i; ++funcBrace; continue ;
				} else {
					error = MarkError(infixEq, i) +
							 "\nerror: there should be open brace after function name";
					postfixQueue.clear(); return ;
				}
			}
			//Check is operator unary minus.
			if (!number && infixEq[i] == '-' && std::isalnum(infixEq[i + 1])) {
				number = true;
				infixEq[i] = '~';
			}
			tmp.push_back(infixEq[i]);
			//Check is we don't deal with digit
			if (number) number = false;
			else {
				//std::cerr << "wrong order: there should be sign" << std::endl;
				error = MarkError(infixEq, i) +
						 "\nerror: wrong order there should be a number";
				postfixQueue.clear(); return ;
			}
			//Push into queue all operators with higher priority
			while (!oper.empty() && operPriority[oper.top()] >= operPriority[tmp]) {
				value operToken; operToken.state = 0; operToken.eq = oper.top();
				//res += oper.top(); oper.pop(); res.push_back(' ');
				postfixQueue.push_back(operToken); oper.pop();
			}
			//Push operator into stack
			oper.push(tmp);
		}
		//And finaly return error if we deal with some obscure character
		else {
			error = MarkError(infixEq, i) +
					"\nerror: obscure character";
			postfixQueue.clear(); return ;
		}
		//Passing whitespaces
		while (std::isspace(infixEq[i + 1])) ++i;
	}
	//Push the rest into string
	while (!oper.empty()) {
		//res += oper.top();
		value operToken; operToken.state = 0; operToken.eq = oper.top();
		oper.pop();
		postfixQueue.push_back(operToken);
		//if (!oper.empty()) res.push_back(' ');
	}
	return ;
}

//Seeking implicit multiplication in expression
void MatrixCalc::StrMultiplySearch(std::string &src, const std::string &token) {
	for (int i = 0; i < src.size(); ) {
		if (src.find(token, i) != std::string::npos) {
			i = src.find(token, i);
			if (i > 0 && std::isdigit(src[i - 1]))
				src.insert(i, " * ");
			i += token.size();
		}
		else break ;
	}
}

//Parametrized constructor
MatrixCalc::MatrixCalc(std::map<std::string, Func> &funcsSrc,
		   std::map<std::string, Matrix> &matriciesSrc,
		   const std::string &src, const std::string &tokenSrc) : funcs(funcsSrc), matricies(matriciesSrc),
		   infixEq(src), baseOpers("+-*/%^"), token(tokenSrc) {
	//Set priorities for functions
	operPriority["("] = 0;
	operPriority["+"] = 1;
	operPriority["-"] = 1;
	operPriority["*"] = 2;
	operPriority["/"] = 2;
	operPriority["%"] = 2;
	operPriority["^"] = 3;
	operPriority["~"] = 5;

	//Set of base functions
	baseFuncsReg = { "sin", "cos", "tan", "exp", "sqrt", "abs" } ;
	baseFuncsMatrix = { "inv", "transp", "l1norm", "l2norm", "det", "adj" };
	//Set priority for base functions
	for (std::string var : baseFuncsReg)
		operPriority[var] = 4;
	for (std::string var : baseFuncsMatrix)
		operPriority[var] = 4;
	//Seeking for multiplying parts
	if (!token.empty())
		StrMultiplySearch(infixEq, token);
	StrMultiplySearch(infixEq, "pi");
	StrMultiplySearch(infixEq, "i");
}

std::string MatrixCalc::CalcIt() {
	//Stack of values
	std::stack<value> nums;
	//Result string
	std::string res;
	//String parser
	std::string parser;
	//reset calcError for recalculcations;
	calcError = false;

	//If there is already a calculated string or error
	//return them
	if (!calcResult.empty()) return calcResult;
	else if (!error.empty()) return error;

	//Processing the postfix queue
	for (std::list<value>::iterator it = postfixQueue.begin(); it != postfixQueue.end(); ++it) {
		//Check whether if this is a operand, no matter if it's number, token or matrix
		if (it->state) nums.push(*it);
		/*if (std::isdigit(postfixExpr[i]) || postfixExpr[i] == '.') {
			parser.clear();
			std::string number = GetStringNumber(postfixExpr, i);
			//Push processed number into the stack of operands
			nums.push(number);
		}
			//check if this a complex number or not
		else if (!parser.compare("i") && !std::isalpha(postfixExpr[i + 1])) {
			nums.push(parser);
			parser.clear();
		}
			//check whether this is a token or not
		else if (!token.empty() && parser.size() == token.size() && !parser.compare(token)
				 && !std::isalpha(postfixExpr[i + 1])) {
			nums.push(parser);
			parser.clear();
		}*/
		//check if function is in the list of user defined strings
		else if (auto search = funcs.find(it->eq); !it->state && search != funcs.end()) {
			//Check if nums is empty, if it is, return error
			if (nums.empty()) {
				error = "error: operands stack is empty";
				calcResult.clear(); return calcResult;
			}
			value tmp = nums.top(); nums.pop();
			nums.push(RemoveTrailZeros(funcExecute(parser, tmp)));
			if (calcError) return nums.top();
			parser.clear();
		}
			//check function if string is in the list of functions
		else if (auto search = funcs.find(parser); search != funcs.end() && !std::isalpha(postfixExpr[i + 1])) {
			//Check whether stack is empty.  If there is none, we get zero value
			//If there is some value we pop value from stack.
			std::string tmp = (nums.empty()) ? 0 : nums.top();
			if (!nums.empty()) nums.pop();
			nums.push(RemoveTrailZeros(funcExecute(parser, tmp)));
			if (calcError) return nums.top();
			parser.clear();
		}

		else if (baseOpers.find(parser) != std::string::npos) {
			//Check if operator is unary
			if (!parser.compare("~")) {
				//Check whether stack is empty.  If there is none, we get zero value
				//If there is some value we pop value from stack.
				std::string tmp = (nums.empty()) ? "" : nums.top();
				if (!nums.empty()) nums.pop();
				//Push a new value into top of the stack
				nums.push(RemoveTrailZeros(Execute("-", "0", tmp)));
				//Onto next cycle iteration
				parser.clear();
				while (std::isspace(postfixExpr[i + 1])) ++i;
				continue ;
			}

			//Declare first and second operand for some operation
			std::string first, second;
			//Get values for these operands in reverse order
			if (!nums.empty()) { second = nums.top(); nums.pop(); }
			else second = "";

			if (!nums.empty()) { first = nums.top(); nums.pop(); }
			else first = "";
			//Get operation result and push it into stack
			nums.push(RemoveTrailZeros(Execute(parser, first, second)));
			if (calcError) return nums.top();
			parser.clear();
		}
		//Passings whitespaces
		while (std::isspace(postfixExpr[i + 1])) ++i;
	}
	res = nums.top();
	nums.pop();
	return res;
}

MatrixCalc::value MatrixCalc::funcExecute(const std::string &oper, const value &var) {
	//Does string have token?
	bool hasAlpha;
	auto ifAlpha = [](const char &c) { return std::isalpha(c); };
	//Search in user defined functions
	auto search = funcs.find(oper);
	//Search in base function set
	auto funcSearch = baseFuncsReg.find(oper);
	//Search in matrix function set
	auto matrixSearch = baseFuncsMatrix.find(oper);
	//result value
	value res;

	//if (var.state == 2) hasAlpha = std::find_if(var.eq.begin(), var.eq.end(), ifAlpha) != var.eq.end();
	//If we got token
	if (var.state == 3) {
		//Search in matrix function set
		if (matrixSearch != baseFuncsMatrix.end() || (search != funcs.end() && search->second.tokenIsMatrix == 1)) {
			//If there is such matrix function, check if token is matrix
			//and if it's not return error
			if (!tokenIsMatrix) tokenIsMatrix = 1;
			else if (tokenIsMatrix != 1) {
				error = "error: " + var.eq + " should be a matrix";
				res.state = 4; return res;
			}
			res.state = 3;
			res.matrix.setToken(var.eq);
			res.eq = oper + "(" + var.eq + ")";
		}
		//Search in regular function set
		if (funcSearch != baseFuncsReg.end() || (search != funcs.end() && search->second.tokenIsMatrix == 2)) {
			//If there is such regular function, check if token is regular expression
			//and if it's not return error
			if (!tokenIsMatrix) tokenIsMatrix = 2;
			else if (tokenIsMatrix != 2) {
				error = "error: " + var.eq + " should be a regular expresssion";
				res.state = 4; return res;
			}
			res.state = 3;
			res.matrix.setToken(var.eq);
		}
	}
	//In case of basic matrix function
	if (matrixSearch != baseFuncsMatrix.end()) {
		//Check is variable a matrix, if it's not return error
		if (var.state != 1) {
			error = "error: " + oper + ": " + var.eq + "isn't a matrix";
			res.state = 4; return res;
		}
		//{ "inv", "transp", "l1norm", "l2norm", "det", "adj" };
		if (*matrixSearch == "l1norm") res = L1norm(var.matrix);
		else if (*matrixSearch == "l2norm") res = L2norm(var.matrix);
		else if (*matrixSearch == "transp") res = Transpose(var.matrix);
		else if (*matrixSearch == "det") res = Det(var.matrix);
		else if (*matrixSearch == "adj") res = Adj(var.matrix);
		else if (*matrixSearch == "inv") res = Inv(var.matrix);
	}
	//In case of user defined function, that processes matricies
	else if (search != funcs.end() && search->second.tokenIsMatrix == 1) {
		//Check is variable a matrix, if it's not return error
		if (var.state != 1) {
			error = "error: " + oper + ": " + var.eq + "isn't a matrix";
			res.state = 4; return res;
		}
		//Expression to calculate Matrix
		std::string expr;
		//Replace matrix function
		expr = ((!var.eq.empty())) ? funcExpose(search->second, var.eq)
				: funcExpose(search->second, var.matrix.getMatrix());
		//Matrix expression calculator
		MatrixCalc subCalc(funcs, matricies, expr, token);
		if (!subCalc.getError().empty()) {
			error = subCalc.getError();
			res.state = 4; return res;
		}
		res = subCalc.getFinValue();
	}
	//In other cases calculate it as a regular expression
	else if (search != funcs.end() && search->second.tokenIsMatrix == 2) {
		//Check is variable a regular expression, if it's not return error
		if (var.state != 2) {
			error = "error: " + oper + ": " + var.matrix.getMatrix() + "isn't a matrix";
			res.state = 4; return res;
		}
		//Regular expression calculator
		RevPolNotation pol(funcs);
		pol.setInfixExpr(oper + "(" + var.eq + ")");
		//if there was error, return it
		if (!pol.getErrMsg().empty()) {
			error = pol.getErrMsg();
			res.state = 4; return res;
		}
		res.eq = pol.CalcIt();
		res.state = 1;
	}
	//if (hasAlpha && search == userDefFuncs.end()) return oper + "(" + var + ")";
	//else if (hasAlpha && search != userDefFuncs.end()) return UDfuncExpose(search->second, var);

	/*if (search != userDefFuncs.end()) {
		std::string calc = UDfuncExpose(search->second, var);
		return RevPolNotation(std::move(calc), userDefFuncs).CalcIt();
	}
	if (!oper.compare("sin")) res = std::sinl(std::stold(var));
	else if (!oper.compare("cos")) res = std::cosl(std::stold(var));
	else if (!oper.compare("tan")) {
		if (!var.compare("1.570796")) {
			calcError = true;
			return "error: tan(pi/2) if undefined value";
		}
		if (!var.compare("4.712389")) {
			calcError = true;
			return "error: tan(3*pi/2) if undefined value";
		}
		res = std::tan(std::stod(var));
	}
	else if (!oper.compare("exp")) res = std::expl(std::stold(var));
	else if (!oper.compare("sqrt")) res = std::sqrtl(std::stold(var));
	else if (!oper.compare("abs")) res = std::abs(std::stold(var));*/
	return res;
}

//Return l1norm value result
MatrixCalc::value MatrixCalc::L1norm(const Matrix &matrix) {
	//Result value
	value res;
	//Expression to calculate
	std::string expression;
	//Expression parser
	RevPolNotation parser(funcs);

	//If matrix is not a vector return error
	if (matrix.getRow() != 1 && matrix.getColumn() != 1) {
		error = "error: l1norm: " + matrix.getMatrix() + " is not a vector";
		res.state = 4; return res;
	}
	res.state = 2;
	//values of target vector
	const std::vector<std::string> &targetValues = matrix.getValues();
	//Generating expression
	for (int i = 0; i < targetValues.size(); ++i)
		expression.append("abs(" + targetValues[i] + ")"
				+ ((i == targetValues.size() - 1) ? "" : " + "));
	//Calculate expression
	parser.setToken(token);
	parser.setInfixExpr(std::move(expression));
	//In case of error, return it
	if (!parser.getErrMsg().empty()) {
		error = parser.getErrMsg();
		res.state = 4; return res;
	}
	//Else generate result;
	res.eq = parser.CalcIt();
	return res;
}

//Return l2norm value result
MatrixCalc::value MatrixCalc::L2norm(const Matrix &matrix) {
	//Result value
	value res;
	//Expression to calculate
	std::string expression;
	//Expression parser
	RevPolNotation parser(funcs);

	//If matrix is not a vector return error
	if (matrix.getRow() != 1 && matrix.getColumn() != 1) {
		error = "error: l1norm: " + matrix.getMatrix() + " is not a vector";
		res.state = 4; return res;
	}
	res.state = 2;
	//values of target vector
	const std::vector<std::string> &targetValues = matrix.getValues();
	//Generating expression
	expression.append("sqrt(");
	for (int i = 0; i < targetValues.size(); ++i)
		expression.append("(" + targetValues[i] + ")^2"
			+ ((i == targetValues.size() - 1) ? "" : " + "));
	//Calculate expression
	parser.setToken(token);
	parser.setInfixExpr(std::move(expression));
	//In case of error, return it
	if (!parser.getErrMsg().empty()) {
		error = parser.getErrMsg();
		res.state = 4; return res;
	}
	//Else generate result;
	res.eq = parser.CalcIt();
	return res;
}

//Return result of matrix transpose
MatrixCalc::value MatrixCalc::Transpose(const Matrix &matrix) {
	//Row and column of the new matrix
	int row = matrix.getColumn(), column = matrix.getRow();
	//Source vector of values
	const std::vector<std::string> &src = matrix.getValues();
	//New set of values for transposed matrix
	std::vector<std::string> resVector;
	//Result value
	value res;

	//copy info into transpose matrix
	for (int i = 0; i < column; ++i) {
		for (int j = 0; j < row; ++j)
			resVector.push_back(src[j * row + i]);
	}
	res.state = 1;
	res.matrix = Matrix(resVector, row, column, token);
	return res;
}

//Determinant of matrix
MatrixCalc::value MatrixCalc::Det(const Matrix &matrix) {
	//result
	value res;
	//Expression string
	std::string expression;
	//Get values of source matrix
	std::vector<std::string> values = matrix.getValues();

	//if matrix is not square, return error
	if (matrix.getRow() != matrix.getColumn()) {
		error = "error: matrix " + matrix.getMatrix() + " is not square\n";
		res.state = 4; return res;
	}
	//matrix square size
	int n = matrix.getRow();

	//value is regular expression
	res.state = 2;
	//if size of matrix is 2, return strict value
	if (n == 1) {
		res.eq = matrix.getValues().back();
		return res;
	}
	if (n == 2) {
		//Expression calculator
		RevPolNotation pol(funcs);
		pol.setToken(token);
		//return src[0] * src[3] - src[2] * src[1];
		expression = "(" + values[0] + ") * (" + values[3] + ") - ("
				+ values[2] + ") * (" + values[1] + ")";
		pol.setInfixExpr(std::move(expression));
		//If there is some error, return it
		if (!pol.getErrMsg().empty()) {
			error = pol.getErrMsg().empty();
			res.state = 4; return res;
		}
		res.eq = pol.CalcIt(); return res;
	}
	//else recourse it
	else {
		for (int c = 0; c < n; ++c) {
			//genereate new vector with lesser size
			std::vector<std::string> newVal(std::pow(n - 1, 2));
			//newVal iterator
			int valIt = 0;
			for (int i = 1; i < n; ++i) {
				for (int j = 0; j < n; ++j) {
					if (j == c)
						continue ;
				newVal[valIt++] = (values[i * n + j]);
				}
			}
			// d = d + (pow(-1, c) * mat[0][c] * det(n - 1, submat));
			//Temporary matrix for next iteration
			Matrix tmp(newVal, n - 1, n - 1, token);
			//next iteration of Det
			value nextIt = Det(tmp);
			//in case of error, return it
			if (nextIt.state == 4) return nextIt;
			res.eq += std::to_string(std::pow(-1, c)) + " * " + values[c] + " * (" + nextIt.eq + ")";
			if (c != n - 1) res.eq.append(" + ");
		}
	}
	//If Determinant has token and token is matrix, return error
	for (int i = 0; i < res.eq.size(); ++i) {
		if (std::isalpha(res.eq[i])) {
			//Suspected token
			std::string susToken;
			while (std::isalpha(res.eq[i]))
				susToken.push_back(res.eq[i++]);
			//if it's token, check if it's regular expression
			//and if it's not, return error
			if (susToken == token && tokenIsMatrix == 1) {
				error = "error: token '" + token + "' is matrix and should be a regular expression";
				res.eq.clear(); res.state = 4; return res;
			}
			//else if token is matrix, but it's state if not defined yet
			//set it as a regular expression and break loop
			else if (susToken == token && !tokenIsMatrix) {
				tokenIsMatrix = 1; break ;
			}
		}
	}
	//Expression calculator
	RevPolNotation pol(funcs);
	pol.setToken(token);
	pol.setInfixExpr(std::move(res.eq));
	//If calculator found some error, return it
	if (!pol.getErrMsg().empty()) {
		error = pol.getErrMsg();
		res.state = 4; return res;
	}
	res.eq = pol.CalcIt();
	return res;
}

//Adjoint matrix
MatrixCalc::value MatrixCalc::Adj(const Matrix &src) {
	//Result
	value res;
	//check is matrix square, and if it's not
	//return error
	if (src.getRow() != src.getColumn()) {
		error = "error: matrix " + src.getMatrix() + " is not square\n";
		res.state = 4; return res;
	}
	//size of matrix
	int n = src.getRow();
	//values vector for matrix;
	std::vector<std::string> values(std::pow(n, 2));
	//values of source Matrix;
	std::vector<std::string> valSrc = src.getValues();

	//If size of matrix is 1 result matrix is [ 1 ]
	res.state = 1;
	if (n == 1) {
		values[0] = "1";
		res.matrix = Matrix(values, 1, 1);
		return res;
	}
	//Generating minor matrix
	for (int c = 0; c < std::pow(n, 2); ++c) {
		//exclude row and column
		int exRow = c / n, exCol = c % n;
		//values of submatrix
		std::vector<std::string> sub(std::pow(n - 1, 2));
		//submatrix iterator
		int subIt = 0;
		for (int i = 0; i < n; ++i) {
			//Pass excluded row
			if (i == exRow) continue ;
			for (int j = 0; j < n; ++j) {
				//Pass excluded column
				if (j == exCol) continue;
				sub[subIt++] = valSrc[i * n + j];
			}
		}
		//Submatrix acquisition
		Matrix submatrix(sub, n - 1, n - 1, token);
		//Calculate determinant, if determinant has error
		//return it
		value det = Det(submatrix);
		if (det.state == 4) return det;
		values[c] = std::to_string(std::pow(-1, exRow + exCol))
				+ " * (" + det.eq + ")";
		//Expression calculator
		RevPolNotation pol(funcs);
		pol.setToken(token);
		//Calculate cofactor in minor matrix. If there is error
		//return it
		pol.setInfixExpr(std::move(values[c]));
		if (!pol.getErrMsg().empty()) {
			error = pol.getErrMsg();
			res.state = 4; return res;
		}
	}
	//Generate result matrix and transpose it
	res.matrix = Matrix(values, n, n, token);
	res = Transpose(res.matrix);
	return res;
}

//Inverse matrix
MatrixCalc::value MatrixCalc::Inv(const Matrix &src) {
	//Result value
	value res;
	//check is matrix square, and if it's not
	//return error
	if (src.getRow() != src.getColumn()) {
		error = "error: matrix " + src.getMatrix() + " is not square\n";
		res.state = 4; return res;
	}
	//square size
	int n = src.getRow();
	//Expression calculator
	RevPolNotation pol(funcs);
	pol.setToken(token);
	//factor
	std::string factor = "1 / (" +  Det(src).eq + ")";
	//Lambda function. Error preview
	auto errorPreview = [](const std::string &msg, std::string &error) {
		value res;

		if (!msg.empty()) {
			error = msg; res.state = 4;
		}
		return res;
	};
	//Calculate factor. If there is error return it
	pol.setInfixExpr(std::move(factor));
	if (errorPreview(pol.getErrMsg(), error).state == 4) { res.state = 4; return res; }
	factor = pol.CalcIt();
	if (errorPreview(pol.getErrMsg(), error).state == 4) { res.state = 4; return res; }
	//Adjugate matrix. If there is some error
	//return it
	res = Adj(src);
	if (res.state == 4) return res;
	//values of Adjugate matrix
	std::vector<std::string> adjValues = res.matrix.getValues();
	//Inverse matrix values
	std::vector<std::string> invValues(std::pow(n, 2));
	//Multiply every element and factor
	for (int i = 0; i < adjValues.size(); ++i) {
		//Matrix soket value
		std::string soketVal = "(" + adjValues[i] + ") * (" + factor + ")";
		//Calculate soket, and if there is error return it
		pol.setInfixExpr(std::move(soketVal));
		if (errorPreview(pol.getErrMsg(), error).state == 4) { res.state = 4; return res; }
		invValues[i] = pol.CalcIt();
		if (errorPreview(pol.getErrMsg(), error).state == 4) { res.state = 4; return res; }
	}
	//Generate matrix and return it
	res.state = 2;
	res.matrix = Matrix(invValues, n, n, token);
	return res;
}

//Expose function and replace token
std::string MatrixCalc::funcExpose(const Func &src, const std::string &forReplace) {
	std::string res = src.equation;

	while(true) {
		if (auto s = res.find(src.token); s != std::string::npos) {
			res.replace(s, src.token.size(), forReplace);
			continue ;
		}
		break ;
	}
	return res;
}

//Get error string
const std::string &MatrixCalc::getError() const { return error; }

//Get final value
const MatrixCalc::value MatrixCalc::getFinValue() const { return finCalc; }