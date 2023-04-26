//
// Created by WildBoarGonnaGo on 01.04.2023.
//

#include "MatrixCalc.h"
#include <cmath>
#include <cstring>
#include <limits>
#include <sstream>
#include <iomanip>

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
		if (infixEq[i] == '[') { prev = i; ++brace; }
		else if (infixEq[i] == ']') { prev = i; --brace; }
		res.push_back(infixEq[i]);
		if (!brace || brace > 2 || brace < 0) break;
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

	//Seek 'lpnorm' function in infix notation expression
	//and if there is error,
	LPnorm();
	if (!error.empty()) return ;
	//Parsing string
	for (int i = 0; i < infixEq.size(); ++i) {
		//If character is digit
		if (std::isdigit(infixEq[i]) || infixEq[i] == '.') {
			//Initiate new number value
			value numberUnit;
			//Check order of operands and operators, in case of error return it
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
		//if character is a left squarebrace or right squarebrace
		else if (infixEq[i] == '[' || infixEq[i] == ']') {
			//Check order of operands and operators, in case of error return it
			if (!number) number = true;
			else {
				error = MarkError(infixEq, i) + "\nerror: wrong order: there should be a matrix";
				postfixQueue.clear(); return ;
			}
			if (!ParseMatrix(i)) { postfixQueue.clear(); return ; }
		}
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
		   infixEq(src), baseOpers("+-*/%^~"), token(tokenSrc) {
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
	baseFuncsReg = { "sin", "cos", "tan", "exp", "sqrt", "abs", "rad",
					 "acos", "asin", "atan", "ceil", "floor", "cosh",
					 "log", "logt", "tanh", "deg", "sinh" } ;
	baseFuncsMatrix = { "inv", "transp", "lonenorm", "ltwonorm", "linfnorm", "det", "adj" };
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
	ProcessPostfix();
}

std::string MatrixCalc::CalcIt() {
	//Stack of values
	std::stack<value> nums;
	//String parser
	std::string parser;
	//reset calcError for recalculcations;
	calcError = false;
	//Lambda function. It calculate function result no matter
	//if it's for matricies, regular expression or some user defined

	//If there is already a calculated string or error
	//return them
	if (!calcResult.empty()) return calcResult;
	else if (!error.empty()) return error;

	//Processing the postfix queue
	for (std::list<value>::iterator it = postfixQueue.begin(); it != postfixQueue.end(); ++it) {
		//Check whether if this is a operand, no matter if it's number, token or matrix
		if (it->state) nums.push(*it);
		//check if function is in the list of user defined strings
		else if (auto search = funcs.find(it->eq); !it->state && search != funcs.end()) {
			if (funcExprExec(it->eq, nums).state == 4) return "";
		}
		//Check if it's some base function operato
		else if (auto search = baseFuncsMatrix.find(it->eq); !it->state && search != baseFuncsMatrix.end()) {
			if (funcExprExec(it->eq, nums).state == 4) return "";
		}
		else if (auto search = baseFuncsReg.find(it->eq); !it->state && search != baseFuncsReg.end()) {
			if (funcExprExec(it->eq, nums).state == 4) return "";
			//parser.clear();
		}
		//Check if it's bas
		else if (it->eq.size() == 1 && !it->state && baseOpers.find(it->eq) != std::string::npos) {
			//Check if operator is unary
			if (it->eq == "~") {
				//Check whether stack is empty.  If it's return error
				//else pop it
				if (nums.empty()) {
					error = "error: '" + it->eq + "': operands stack is empty";
					calcResult.clear(); finCalc.state = 4; return "";
				}
				value tmp = nums.top(); nums.pop();
				//Set new value with '-1' multiplier
				value multip; multip.eq = "-1"; multip.state = 2;
				//Push a new value into top of the stack
				tmp = Execute("*", multip, tmp);
				nums.push(tmp);
				//Onto next cycle iteration
				continue ;
			}

			//Declare first and second operand for some operation
			value first, second;
			//Get values for these operands in reverse order
			//Check whether stack is empty.  If it's return error
			//else pop it
			if (nums.empty()) {
				error = "error: '" + it->eq + "': operands stack is empty";
				calcResult.clear(); finCalc.state = 4; return "";
			}
			second = nums.top(); nums.pop();

			//Check whether stack is empty.  If it's return error
			//else pop it
			if (nums.empty()) {
				error = "error: '" + it->eq + "': operands stack is empty";
				calcResult.clear(); finCalc.state = 4; return "";
			}
			first = nums.top(); nums.pop();
			//Calculate result value
			value tmp = Execute(it->eq, first, second);
			//If there was error due calculation, return error
			if (tmp.state == 4) { calcResult.clear(); finCalc.state = 4; return ""; }
			//Get operation result and push it into stack
			nums.push(tmp);
			//if (calcError) return nums.top();
			//parser.clear();
		}
		//Passings whitespaces
		//while (std::isspace(postfixExpr[i + 1])) ++i;
	}
	//Generate result value
	finCalc = nums.top();
	//If result is a regular expression
	if (finCalc.state == 2) calcResult = finCalc.eq;
	//If result is a matrix
	if (finCalc.state == 1) calcResult = finCalc.matrix.toString();
	nums.pop();
	return calcResult;
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

	//In case of basic matrix function
	if (matrixSearch != baseFuncsMatrix.end()) {
		//Check is variable a matrix, if it's not return error
		if (var.state != 1) {
			error = "error: " + oper + ": " + var.eq + "isn't a matrix";
			res.state = 4; return res;
		}
		//{ "inv", "transp", "lonenorm", "ltwonorm", "linfnorm", "det", "adj" };
		if (*matrixSearch == "lonenorm") res = L1norm(var.matrix);
		else if (*matrixSearch == "ltwonorm") res = L2norm(var.matrix);
		else if (*matrixSearch == "transp") res = Transpose(var.matrix);
		else if (*matrixSearch == "det") res = Det(var.matrix);
		else if (*matrixSearch == "adj") res = Adj(var.matrix);
		else if (*matrixSearch == "inv") res = Inv(var.matrix);
		else if (*matrixSearch == "linfnorm") res = LInfNorm(var.matrix);
	}
	//In other cases calculate it as a regular expression
	else if (search != funcs.end() || funcSearch != baseFuncsReg.end()) {
		//Check is variable a regular expression, if it's not return error
		if (var.state != 2) {
			error = "error: " + oper + ": " + var.matrix.getMatrix() + "isn't a regular expression";
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
	return res;
}

//Return norm of p degree
void MatrixCalc::LPnorm() {
	//Expression calculator
	RevPolNotation pol(funcs);

	while (infixEq.find("lpnorm") != std::string::npos) {
		//position iterator
		int pos = infixEq.find("lpnorm") + std::strlen("lpnorm");

		//If in 'pos' position there is no brace
		//return error
		if (infixEq[pos] != '(') {
			error = MarkError(infixEq, pos) + "\nerror: there should be brace";
			return ;
		}
		//start position of inner contains of 'lpnorm'
		//function
		int begin = ++pos;
		//number of braces
		int brace = 1;
		for ( ; pos < infixEq.size(); ++pos) {
			if (infixEq[pos] == '(') ++brace;
			else if (infixEq[pos] == ')') --brace;
			if (!brace) break;
		}
		//If number of braces doesn't fit return error
		if (brace) {
			--begin;
			error = MarkError(infixEq, begin) + "\nerror: wrong number of braces";
			return ;
		}
		//position of end of 'lpnorm' function
		int end = pos;
		//Matrix for calculation
		Matrix m;
		//Matrix index parser
		int mi = begin;
		//Passing whitespaces
		while (std::isspace(infixEq[mi])) ++mi;
		//Matrix variable string
		std::string matVar;
		//If we deal with variable, assign to matrix 'm'
		//If there is no such variable, return error
		while(std::isalpha(infixEq[mi])) matVar.push_back(infixEq[mi++]);
		if (!matVar.empty()) {
			if (auto search = matricies.find(matVar); search != matricies.end())
				m = matricies[matVar];
			else {
				error = MarkError(infixEq, mi) + "error: matricies: there is no such variable";
				return ;
			}
		}
		//If it's not a variable, let's parse first attribute of 'lpnorm' function
		else {
			//If there is no squarebrace in 'mi' position return error
			if (infixEq[mi] != '[') {
				error = MarkError(infixEq, mi) + "error: matricies: there is no such variable";
				return ;
			}
			//number of squarebraces.
			brace = 0;
			//Parse piece of matrix string in 'lpnorm' function
			//If number of squarebraces doesn't fit, return error
			for ( ; mi < infixEq.size(); ++mi) {
				if (infixEq[mi] == '[') ++brace;
				else if (infixEq[mi] == ']') --brace;
				if (!brace) break;
			}
			if (brace) {
				error = MarkError(infixEq, mi) + "\nerror: wrong number of braces";
				return ;
			}
			m.setMatrix(infixEq.substr(begin, mi + 1 - begin), funcs, matricies);
			++mi;
		}
		//Passing whitespaces and check if there is comma character
		//and if there is no, return error
		while (std::isspace(infixEq[mi])) ++mi;
		if (infixEq[mi] != ',') {
			error = MarkError(infixEq, mi) + "error: matricies: there should be comma character";
			return ;
		}
		pos = ++mi;
		//Check matrix for error. If there is, return it
		if (!m.getError().empty()) { error = m.getError(); return ; }
		//If matrix isn't a vector (it's row or column should only one)
		//return error
		if (m.getRow() != 1 && m.getColumn() != 1) {
			error = "error: lpnorm: " + m.getMatrix() + ": this matrix isn't vector";
			return ;
		}
		//Let's calculate power value, in case of error
		//return it
		matVar = infixEq.substr(pos, end - pos);
		pol.setInfixExpr(std::move(matVar));
		if (!pol.getErrMsg().empty()) { error = pol.getErrMsg(); return ; }
		matVar = pol.CalcIt();
		if (!pol.getErrMsg().empty()) { error = pol.getErrMsg(); return ; }
		//set of matrix values
		std::vector<std::string> values = m.getValues();
		//final expression string for calculating
		std::string expr = "(";
		for (int i = 0; i < values.size(); ++i) {
			expr += "abs(" + values[i] + ")^"
					+ matVar + ((i != values.size() - 1) ? " + " : ")^(1 / " + matVar + ")");
		}
		//Calculate expression. In case of error return it
		pol.setInfixExpr(std::move(expr));
		if (!pol.getErrMsg().empty()) { error = pol.getErrMsg(); return ; }
		expr = pol.CalcIt();
		if (!pol.getErrMsg().empty()) { error = pol.getErrMsg(); return ; }
		//Replace 'lpnorm' function with new calculated value in expression
		infixEq.replace(infixEq.find("lpnorm"), end - infixEq.find("lpnorm") + 1, expr);
	}
}

//Return infinite norm
MatrixCalc::value MatrixCalc::LInfNorm(const Matrix &matrix) {
	//Result value
	value res;
	//Expression calculator
	RevPolNotation pol(funcs);
	//Set of source matrix values
	std::vector<std::string> values = matrix.getValues();
	//Max value. When initiated, it's value is the most
	//minimum possible value
	long double max = std::numeric_limits<long double>::min();
	//string stream, to format value. Set precision to 10
	std::stringstream oss;

	//If matrix isn't a vector (it's row or column should only one)
	//return error
	if (matrix.getRow() != 1 && matrix.getColumn() != 1) {
		error = "error: linfnorm: " + matrix.getMatrix() + ": this matrix isn't vector";
		res.state = 4; return res;
	}
	for (std::string var : values) {
		//expression entity
		std::string expr = "abs(" + var + ")";
		//temporary double value
		long double tmp;

		//Calculate expression. In case of error return it
		pol.setInfixExpr(std::move(expr));
		if (!pol.getErrMsg().empty()) {
			error = pol.getErrMsg(); res.state = 4 ; return res;
		}
		expr = pol.CalcIt();
		if (!pol.getErrMsg().empty()) {
			error = pol.getErrMsg(); res.state = 4; return res;
		}
		tmp = std::stold(expr);
		if (max < tmp) max = tmp;
	}
	//Generate result value
	oss << std::setprecision(10) << max;
	res.state = 2;
	res.eq = oss.str();
	return res;
}

//Return lonenorm value result
MatrixCalc::value MatrixCalc::L1norm(const Matrix &matrix) {
	//Result value
	value res;
	//Expression to calculate
	std::string expression;
	//Expression parser
	RevPolNotation parser(funcs);

	//If matrix is not a vector return error
	if (matrix.getRow() != 1 && matrix.getColumn() != 1) {
		error = "error: lonenorm: " + matrix.getMatrix() + " is not a vector";
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

//Return ltwonorm value result
MatrixCalc::value MatrixCalc::L2norm(const Matrix &matrix) {
	//Result value
	value res;
	//Expression to calculate
	std::string expression;
	//Expression parser
	RevPolNotation parser(funcs);

	//If matrix is not a vector return error
	if (matrix.getRow() != 1 && matrix.getColumn() != 1) {
		error = "error: lonenorm: " + matrix.getMatrix() + " is not a vector";
		res.state = 4; return res;
	}
	res.state = 2;
	//values of target vector
	const std::vector<std::string> &targetValues = matrix.getValues();
	//Generating expression
	expression.append("sqrt(");
	for (int i = 0; i < targetValues.size(); ++i)
		expression.append("(" + targetValues[i] + ")^2"
			+ ((i == targetValues.size() - 1) ? ")" : " + "));
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
	for (int i = 0; i < row; ++i) {
		for (int j = 0; j < column; ++j)
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
		values[c] = pol.CalcIt();
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
	res.state = 1;
	res.matrix = Matrix(invValues, n, n, token);
	return res;
}

//Execute function block, no matter it's for matricies, regular expression
//or it's some user defined
MatrixCalc::value MatrixCalc::funcExprExec(const std::string &oper, std::stack<value> &nums) {
	//Result value
	value res;

	if (nums.empty()) {
		error = "error: '" + oper + "': operands stack is empty";
		calcResult.clear(); res.state = 4; return res;
	}
	value tmp = nums.top(); nums.pop();
	nums.push(funcExecute(oper, tmp));
	//If there was some error return it
	if (nums.top().state == 4) {
		calcResult.clear();
		finCalc = nums.top(); res.state = 4; return res;
	}
	res = nums.top();
	return res;
}

//Execute some base operands between two values
MatrixCalc::value MatrixCalc::Execute(const std::string &oper,
									  const value &first, const value &second) {
	//Result value
	value res;

	//In case of first and second variables are matricies
	if (first.state == 1 && second.state == 1) {
		//in case of summing and subtract
		if (oper == "+" || oper == "-") res = MatriciesSumSub(oper, first, second);
		//in case of multiplication
		else if (oper == "*") res = MatrixMulti(first, second);
		//In case of division operation return error
		else if (oper == "/") {
			error = "error: '/': " + FinResGenerate(first, true) + ": "
			+ FinResGenerate(second, true) + ": matricies division is not permitted";
			res.state = 4;
		}
		//In case of power raising return error
		else if (oper == "^") {
			error = "error: '/': " + FinResGenerate(first, true) + ": "
				+ FinResGenerate(second, true) + ": matricies power raise is not permitted";
			res.state = 4;
		}
	}
	//In case of first variable is matrix and second is regular expression
	else if (first.state == 1 && second.state == 2) {
		//In case of multiplication
		if (oper == "*") res = MatrixNumMulti(second, first);
		//In case of summation return error
		else if (oper == "+") {
			error = "error: '+': " + FinResGenerate(first, true) + ": "
				+ FinResGenerate(second, true) + ": summation of matrix and regular expression is not permitted";
			res.state = 4;
		}
		//In case of subtraction return error
		else if (oper == "-") {
			error = "error: '-': " + FinResGenerate(first, true) + ": "
				+ FinResGenerate(second, true) + ": subtraction of matrix and regular expression is not permitted";
			res.state = 4;
		}
		//In case of division
		else if (oper == "/") {
			//Temporary value for further calculations
			value tmp; tmp.eq = "1 / (" + second.eq + ")";
			res = MatrixNumMulti(tmp, first);
		}
		//In case of power raising
		else if (oper == "^") res = MatrixPowerRaise(first, second);
	}
	//In case of first variable is a regular expression and second is a matrix
	else if (first.state == 2 && second.state == 1) {
		//In case of multiplication of regular expression and matrix
		if (oper == "*") res = MatrixNumMulti(first, second);
		//In case of summation return error
		else if (oper == "+") {
			error = "error: '+': " + FinResGenerate(first, true) + ": "
				+ FinResGenerate(second, true) + ": summation of regular expression and matrix is not permitted";
			res.state = 4;
		}
		//In case of subtraction return error
		else if (oper == "-") {
			error = "error: '-': " + FinResGenerate(first, true) + ": "
				+ FinResGenerate(second, true) + ": subtraction of regular expression and matrix is not permitted";
			res.state = 4;
		}
		//In case of division return error
		else if (oper == "/") {
			error = "error: '/': " + FinResGenerate(first, true) + ": "
				+ FinResGenerate(second, true) + ": division of regular expression and matrix is not permitted";
			res.state = 4;
		}
		//In case of power raise return error
		else if (oper == "^") {
			error = "error: '^': " + FinResGenerate(first, true) + ": "
				+ FinResGenerate(second, true) + ": regular expression raising to a matrix is not permitted";
			res.state = 4;
		}
	}
	//In case of both values are regular expressions, or
	//one these values is token, and second is regular expression
	else if ((first.state == 2 && second.state == 2)
		|| (first.state == 3 && second.state == 2)
		|| (first.state == 2 && second.state == 3)) {
		//Expression calculator, in case of error return it
		RevPolNotation pol(funcs);
		pol.setInfixExpr("(" + first.eq + ") " + oper + " (" + second.eq + ")");
		if (!pol.getErrMsg().empty()) { error = pol.getErrMsg(); res.state = 4; }
		res.eq = pol.CalcIt();
		if (!pol.getErrMsg().empty()) { error = pol.getErrMsg(); res.state = 4; }
		res.state = 2;
	}
	//If first variable is matrix and second is token
	else if (first.state == 1 && second.state == 3) {
		//In case of matrix and token multiplication
		if (oper == "*")
			res = MultiTokenMatrix(first, second);
		//In case of matrix and token division
		else if (oper == "/") {
			//Temporary value for further calculations
			value tmp; tmp.eq = "1 / (" + second.eq + ")";
			res = MatrixNumMulti(tmp, first);
		}
		//In other cases just add operator
		else
			res = AddOperTokenMatrix(oper, first, second);
	}
	//If first variable is token and second is matrix
	else if (first.state == 3 && second.state == 1) {
		//In case of token and matrix multiplication
		if (oper == "*")
			res = MultiTokenMatrix(first, second);
		//In case of division return error
		else if (oper == "/") {
			error = "error: '/': " + FinResGenerate(first, true) + ": "
				+ FinResGenerate(second, true) + ": division of regular expression and matrix is not permitted";
			res.state = 4;
		}
		//In case of power raise return error
		else if (oper == "^") {
			error = "error: '^': " + FinResGenerate(first, true) + ": "
				+ FinResGenerate(second, true) + ": regular expression raising to a matrix is not permitted";
			res.state = 4;
		}
		//In other cases just add the operation
		else res = AddOperTokenMatrix(oper, first, second);
	}
	return res;
}

//Matricies summing and subtract
MatrixCalc::value MatrixCalc::MatriciesSumSub(const std::string &oper,
											  const value &first, const value &second) {
	//Result value
	value res;

	//In case of first value is complex matrix equation and the second is
	//simple matrix
	if (!first.lst.empty() && second.lst.empty())
		return ComplexSimpleMatrixSumSub(first, second, oper);
	//In case of first value is simple matrix and the second is
	//complex matrix equation
	if (first.lst.empty() && !second.lst.empty())
		return SimpleComplexMatrixSumSub(first, second, oper);
	//In case of both values are complex matrix equations
	if (!first.lst.empty() && !second.lst.empty())
		return ComplexComplexMatrixSumSub(first, second, oper);
	//Compare size of matricies. If their's sizes are not equal, return error
	if (first.matrix.getRow() != second.matrix.getRow() &&
			first.matrix.getRow() != second.matrix.getColumn()) {
		error = "error: " + first.matrix.getMatrix() + " " +
				oper + " " + second.matrix.getMatrix() + ": sizes are not equal";
		calcResult.clear(); res.state = 4; return res;
	}
	//values of the first matrix
	std::vector<std::string> fVal = first.matrix.getValues();
	//values of the second matrix
	std::vector<std::string> sVal = second.matrix.getValues();
	//Row and column size
	int row = first.matrix.getRow(); int column = second.matrix.getColumn();
	//Result values vector and it's iterator
	std::vector<std::string> resVal(row * column); int resIt = 0;
	//Sum or operand matrix values
	for (int i = 0; i < row * column; ++i) {
		//Expression calculator
		RevPolNotation calc(funcs);
		calc.setToken(token);
		//Set infix expression and calculate it
		//in case of error return it
		calc.setInfixExpr("(" + fVal[i] + ") " + oper + " (" + sVal[i] + ")");
		if (!calc.getErrMsg().empty()) {
			error = calc.getErrMsg();
			res.state = 4; return res;
		}
		resVal[i] = calc.CalcIt();
		if (!calc.getErrMsg().empty()) {
			error = calc.getErrMsg();
			res.state = 4; return res;
		}
	}
	res.state = 1;
	res.matrix = Matrix(resVal, row, column);
	return res;
}

MatrixCalc::value MatrixCalc::MatrixSocketMultiply(const Matrix &f, const Matrix &s, int row, int column) {
	//Result value
	value res;

	//Computing result
	for (int i = 0; i < f.getColumn(); ++i)
		res.eq += (res.eq.empty() ? "((" : " + ((") + f.getValues()[row * f.getColumn() + i]
				+ ") * (" + s.getValues()[i * s.getColumn() + column] + "))";
	//Expression calculator. If there will be some due calculation
	//error return it
	RevPolNotation pol(funcs);
	pol.setInfixExpr(std::move(res.eq));
	pol.setToken(token);
	if (!pol.getErrMsg().empty()) {
		error = pol.getErrMsg();
		res.state = 4; return res;
	}
	res.eq = pol.CalcIt();
	if (!pol.getErrMsg().empty()) {
		error = pol.getErrMsg();
		res.state = 4; return res;
	}
	res.state = 2;
	return res;
}

//Matricies multiplication
MatrixCalc::value MatrixCalc::MatrixMulti(const value &f, const value &s) {
	//Result value
	 value res;

	//In case of first value is simple matrix and
	//second is complex matrix equation

	//if number of columns of 'f' matrix doesn't equal
	//to number of rows of 's' matrix return error
	//and return empty matrix
	if (f.matrix.getColumn() != s.matrix.getRow()) {
		error = "error: first matrix (" + f.matrix.getMatrix() + ") number of columns isn't equal\n"
			+ "to number of rows of second matrix (" + s.matrix.getMatrix() + ")";
		res.state = 4;
	}
	//Number of rows and columns of result matrix
	int row = f.matrix.getRow(), column = s.matrix.getColumn();
	//Values of result matrix
	std::vector<std::string> values(row * column);
	//Assigning result matricies number of rows and number of columns
	//Calculating result matrix values
	for (int i = 0; i < row; ++i) {
		for (int j = 0; j < column; ++j) {
			//Temporary value. If there was error due socket calculation
			//return error
			value tmp = MatrixSocketMultiply(f.matrix, s.matrix, i, j);
			if (tmp.state == 4) return tmp;
			values[i * column + j] = tmp.eq;
		}
	}
	//Generate result matrix
	res.matrix = Matrix(values, row, column, token);
	res.state = 1;
	return res;
}

//Number and Matrix multiplication 'f' is a regular expression, 's' is a matrix
MatrixCalc::value MatrixCalc::MatrixNumMulti(const value &f, const value &s) {
	//In case of f has one value and matrix - multiple values
	//Result value
	value res;
	//Expression calculator
	RevPolNotation pol(funcs);
	pol.setToken(token);
	//Is there is some token in regular expression
	bool isThereToken = false;

	//Search token in regular expression, and if there is set
	//'isThereToken' as true
	if (f.eq.find(token) != std::string::npos) isThereToken = true;
	//In case, when matrix is complex
	if (!s.lst.empty()) return MatrixVectorNumMulti(s, f);
	//In case when regular expression is complex: or it has
	//multiple elements either it has some basic function
	else if (isThereToken) {
		//Result value is matrix
		res.state = 1;
		res.lst.push_back(f);
		res.lst.push_back(value(0, "*"));
		res.lst.push_back(s);
		return res;
	}
	//Lambda function. in case of error, return it
	auto ErrorCheck = [](const std::string &errMsg, std::string &error) {
		//Result value
		value res;

		if (!errMsg.empty()) {
			error = errMsg; res.state = 4; return res;
		}
		res.state = 1;
		return res;
	};

	//Let's calculate factor. In case of error, return it
	pol.setInfixExpr(std::string(f.eq));
	if (ErrorCheck(pol.getErrMsg(), error).state == 4) { res.state = 4; return res; }
	//Calculate factor
	std::string factor = pol.CalcIt();
	if (ErrorCheck(pol.getErrMsg(), error).state == 4) { res.state = 4; return res; }
	//Result values vector
	std::vector<std::string> values(s.matrix.getValues().size());
	//Multiply every element of second matrix and factor
	for (int i = 0; i < s.matrix.getValues().size(); ++i) {
		//Multiplication calculator. If error occured, return it
		pol.setInfixExpr("(" + factor + ") * (" + s.matrix.getValues()[i] + ")");
		if (ErrorCheck(pol.getErrMsg(), error).state == 4) { res.state = 4; return res; }
		values[i] = pol.CalcIt();
		if (ErrorCheck(pol.getErrMsg(), error).state == 4) { res.state = 4; return res; }
	}
	//Generate result matrix and return it
	res.state = 1;
	res.matrix = Matrix(values, s.matrix.getRow(), s.matrix.getColumn());
	return res;
}

//Matrix power raising. 'f' is a matrix, 's' should be integer number
MatrixCalc::value MatrixCalc::MatrixPowerRaise(const value &f, const value &s) {
	//Result value
	value res;

	//If matrix is not square return error
	if (f.matrix.getColumn() != f.matrix.getRow()) {
		error = "error: '^': " + f.matrix.getMatrix()
				+ ": for proper power raising matrix should be squared.";
		res.state = 4; return res;
	}
	//Expression calculator. In case of calculation error return it
	RevPolNotation pol(funcs);
	pol.setToken(token);
	pol.setInfixExpr("(" + std::string(s.eq) + ")");
	if (!pol.getErrMsg().empty()) {
		error = pol.getErrMsg();
		res.state = 4; return res;
	}
	//Power value
	std::string power = pol.CalcIt();
	if (!pol.getErrMsg().empty()) {
		error = pol.getErrMsg();
		res.state = 4; return res;
	}
	//Check is power a interger value, and if it's
	//not return error
	if (power.find('.') != std::string::npos) {
		error = "error: '^': " + f.matrix.getMatrix()
				+ ": " + power + ": power value is not an integer value";
		res.state = 4; return res;
	}
	//Iterator bound
	int bound = std::stoi(power);
	res = f;
	//Matrix raising. In case of error, return it
	for (int i = 1; i < bound; ++i) {
		res = MatrixMulti(res, f);
		if (res.state == 4) return res;
	}
	return res;
}

//Denomination elements for regular expression
std::list<std::string> MatrixCalc::DenomElems(const std::string &src) {
	std::list<std::string> res;
	std::string elem;
	int ghost = 0;

	for (int i = 0; i < src.length(); ++i) {
		elem.push_back(src[i]);
		if (src[i] == '(') ++ghost;
		else if (src[i] == ')') --ghost;
		else if ((src[i] == '+' || (src[i] == '-' && std::isspace(src[i + 1]))) && !ghost) {
			int tmp = elem.size() - 1;

			elem = elem.substr(0, tmp);
			elem = elem.substr(0, elem.find_last_not_of(" ") + 1);
			elem = elem.substr(elem.find_first_not_of(" "));
			res.push_back(elem);
			elem.clear();
			elem.push_back(src[i]);
			res.push_back(elem);
			elem.clear();
		}
	}
	if (!elem.empty()) {
		elem = elem.substr(0, elem.find_last_not_of(" ") + 1);
		elem = elem.substr(elem.find_first_not_of(" "));
		res.push_back(elem);
	}
	return res;
}

//Generating string in case of error, or in case of result
std::string MatrixCalc::FinResGenerate(const value &val, const bool &isError) {
	//If source value's list is empty return it corresponding string
	if (val.lst.empty()) {
		//In case of regular expression or token
		if (val.state == 2 || val.state == 3) return val.eq;
		//In case of matrix and calculating error
		if (val.state == 1 && isError) return val.matrix.getMatrix();
		//And finally, if it's ok, just return matrix
		else return val.matrix.toString();
	}
	//Result string
	std::string res;
	//In other case generate result with
	//vector parsing, result string
	for (auto it = val.lst.begin(); it != val.lst.end(); ++it) {
		//In case of first element just append it to string
		if (it == val.lst.begin()) {
			//In case of matrix
			if (it->state == 1) res.append(it->matrix.getMatrix());
			//In case of regular expression
			else if (it->state == 2) res.append(it->eq);
			//In case of operator (function)
			else if (!it->state) res.append(it->eq);
			//In case of brace
			else if (it->state == 5) res.append(it->eq);
		}
		//In other cases parse looking forward operations
		//In case of some function or base operator
		if (!it->state) {
			//Search among base regular functions
			auto s = baseFuncsReg.find(it->eq);
			//Search among base matrix functions
			auto sm = baseFuncsMatrix.find(it->eq);

			//If function was found append function name and brace
			//after it
			if (s != baseFuncsReg.end() || sm != baseFuncsMatrix.end()) {
				res.append(it->eq); //res.append(val.vec[i].eq);
			}
			//In other cases just append operation with spaces
			else res.append(" " + it->eq + " ");
		}
		//In case of matrix
		else if (it->state == 1) res.append(it->matrix.getMatrix());
		//in other cases just append vector element
		else res.append(it->eq);
	}
	return res;
}

MatrixCalc::value MatrixCalc::MultiTokenMatrix(const value &f, const value &s) {
	//Let determine token and matrix value
	value token = (f.state == 3) ? f : s;
	value matrix = (f.state == 3) ? s : f;
	//Result value
	value res;
	//number of braces
	int brace = 0;

	//In case of complex matrix value
	if (!matrix.lst.empty()) {
		for (auto it = matrix.lst.begin(); it != matrix.lst.end(); ++it) {
			//If we have '+' or '-' operation and no
			//braces encounter we multiplicate token
			//and intermidiate value
			if (it->state == 0 && !brace && (it->eq == "+" || it->eq == "-")) {
				res.lst.push_back(value(0, "*"));
				res.lst.push_back(token);
			}
			//In case of open brace increment brace counter
			else if (it->state == 5 && it->eq == "(") ++brace;
			//In case of close brace decrement brace counter
			else if (it->state == 5 && it->eq == ")") --brace;
			res.lst.push_back(*it);
		}
		res.lst.push_back(value(0, "*"));
		res.lst.push_back(token);
		res.state = 1;
		return res;
	}
	res.state = 1;
	res.lst.push_back(f);
	res.lst.push_back(value(0, "*"));
	res.lst.push_back(s);
	return res;
}

//Add operation for token and matrix
MatrixCalc::value MatrixCalc::AddOperTokenMatrix(const std::string &oper, const value &f,
							 const value &s) {
	//Result value
	value res;
	//Token value
	value token = (f.state == 3) ? f : s;
	//Matrix value
	value matrix = (f.state == 1) ? f : s;

	//In case of complex matrix value
	if (!matrix.lst.empty()) {
		res.lst = matrix.lst;
		//If we deal with power raising add braces
		//besides of power raising
		if (oper == "^") {
			res.lst.push_front(value(5, "("));
			res.lst.push_back(value(5, ")"));
		}
		//Add operation to the end of the list
		res.lst.push_back(value(0, oper));
		res.lst.push_back(token);
		res.state = 1;
		return res;
	}
	res.lst.push_back(f);
	res.lst.push_back(value(0, oper));
	res.lst.push_back(s);
	return res;
}

/*Multiplying complex matrix equation and regular expression
 'm' represents matrix, 'r' represents regular expression*/
MatrixCalc::value MatrixCalc::MatrixVectorNumMulti(const value &m, const value &r) {
	//Result value
	value res;
	//matrix values
	std::list<value> lst = m.lst;
	//number of braces
	int brace = 0;
	//Should we add brace in multiplying equation
	bool shAddBr = false;

	//Check complexity of equation
	for (auto it = m.lst.begin(); it != m.lst.end(); ++it) {
		//If number of braces is zero (we're not inside function
		//or complex equation) and there is subtracting or summing
		//we initiate brace adding to equation
		if (!brace && it->state == 0 && (it->eq == "-" || it->eq == "+")) {
			shAddBr = true; break ;
		}
		else if (it->state == 5 && it->eq == "(") ++brace;
		else if (it->state == 5 && it->eq == ")") --brace;
	}
	//Generate final equation for result value
	res.lst.push_back(r);
	//Add multiplying operator to list
	res.lst.push_back(value(0, "*"));
	//Add open brace, if there should be
	if (shAddBr == true) res.lst.push_back(value(5, "("));
	//Add other elements from equation
	for (auto it = m.lst.begin(); it != m.lst.end(); ++it)
		res.lst.push_back(*it);
	//Add close brace, if there should be
	if (shAddBr == true) res.lst.push_back(value(5, ")"));
	//Finally we return matrix
	res.state = 1;
	return res;
}

//Summing or subtracting of complex matrix and simple matrix
MatrixCalc::value MatrixCalc::ComplexSimpleMatrixSumSub(const value &cm, const value &sm,
														const std::string &oper) {
	//Copy of complex matrix value list
	std::list<value> c = cm.lst;
	//Result value
	value res;
	//'is matrix pure' value
	bool isMatrixPure = true;
	//'Was matrix (sm) computated' value
	bool comp = false;
	//sign multiplyer (it can be '-1' or 1)
	std::string multi;
	//number of braces
	int brace = 0;
	//Sign iterator
	auto sIt = res.lst.rbegin();

	//Search for pure matrix (there is just matrix, and that's all,
	//without any multiplying part and functions). And if there is
	//make 'oper' operation with
	for (auto it = c.begin(); it != c.end(); ++it) {
		//In case of operation value and operation is not subtraction
		//or summing set 'isMatrixPure' as false
		if (!it->state && it->eq != "+" && it->eq != "-") isMatrixPure = false;
		//In case of non-matrix value (if it's operation go to next section)
		//set 'isMatrixPure' as false
		else if (it->state != 1 && it->state) isMatrixPure = false;
		//In case of summing or subracting inside braces (in case of
		//power raising or some matrix function), set 'isMatrixPure' state
		//as false
		else if (!it->state && (it->eq == "+" || it->eq == "-") && brace)
			isMatrixPure = false;
		//In case of summing or subtracting operation and not pure matrix
		//to operate with, reset 'isMatrixPure' value as true and set multiplier
		//for next iteration
		else if (!it->state && (it->eq == "+" || it->eq == "-") && !brace
				 && !isMatrixPure) {
			//Set multiplier according to encountered sign
			multi = (it->eq == "-") ? "-1" : "1";
			//Reset 'isMatrixPure' state as true
			isMatrixPure = true;
			//Fix position of the last sign
			res.lst.push_back(*it);
			sIt = res.lst.rbegin();
			//Continue iterations
			continue;
		}
		//In case of summing or subtracting operation
		//with pure Matrix, let's make computation, if it wasn't
		//done yet
		else if (!it->state && (it->eq == "+" || it->eq == "-") && isMatrixPure
				 && !brace) {
			//In case of negative multiplier, let's perform
			//an additional multiply operation
			if (multi == "-1") {
				res.lst.back() = Execute("*", value(2, "-1"), res.lst.back());
				//In case of error return it
				if (res.lst.back().state == 4) return res.lst.back();
				sIt->eq = "+";
			}
			//Computation is performed
			comp = true;
			//Push calculating result
			res.lst.back() = Execute(oper, res.lst.back(), sm);
			//If there is error return it
			if (res.lst.back().state == 4) return res.lst.back();
			//Reset 'isMatrixPure' state as true
			isMatrixPure = true;
		}
		//In case of open brace increment brace counter
		if (it->state == 5 && it->eq == "(") ++brace;
		//In case of close brace decrement brace counter
		if (it->state == 5 && it->eq == ")") --brace;
		res.lst.push_back(*it);
	}
	//If in the iteration end we have pure matrix and computation is
	//not made yet, we perform this computation
	if (isMatrixPure && !comp) {
		res.lst.push_back(Execute(oper, res.lst.back(), sm));
		//If there is error return it
		if (res.lst.back().state == 4) return res.lst.back();
		//If there wasn't error, set result as matrix and return it
		res.state = 1;
		return res;
	}
	//In other cases we push operation and 'sm' matrix. We also set result
	//as matrix
	res.state = 1;
	res.lst.push_back(value(0, oper));
	res.lst.push_back(sm);
	return res;
}

//Summing or Subtracting of simple matrix and complex matrix
MatrixCalc::value MatrixCalc::SimpleComplexMatrixSumSub(const value &sm, const value &cm,
								const std::string &oper) {
	//Result value
	value res;
	//'Is matrix pure' variable
	bool isMatrixPure = true;
	//number of braces
	int brace = 0;
	//Temporary list of values
	std::list<value> tmp;
	//Floating operator
	std::string foper = oper;
	//'Computation is made for sm' variable
	bool comp = false;

	//Push simple matrix into result list
	res.lst.push_back(sm);
	//Iterate complex matrix from start to end, searching pure matricies
	for (auto it = cm.lst.begin(); it != cm.lst.end(); ++it) {
		//In case of operation (or function) token and not summing ('+') or
		//subracting operation ('-') we don't deal with pure matrix
		//thus assign 'isMatrixPure' to false
		if (!it->state && it->eq != "+" && it->eq != "-") isMatrixPure = false;
		//In case of not matrix and non-operation value
		//we don't deal with pure matrix also.
		else if (it->state != 1 && it->state) isMatrixPure = false;
		//In case we have summming or subtracting, BUT inside braces
		//(which means power raising or some matrix base function)
		//we don't deal with pure matrix, so assign 'isMatrixPure' to false
		else if (!it->state && (it->eq == "+" || it->eq == "-") && brace)
			isMatrixPure = false;
		//In case of summing or subtracting of pure matrix and not performed
		//calculations yet
		else if (!it->state && (it->eq == "+" && it->eq == "-")
				 && isMatrixPure && !brace && !comp) {
			//Let's execute operation
			res.lst.front() = Execute(foper, res.lst.front(), tmp.back());
			//If there is some execution error, return it
			if (res.lst.front().state == 4) return res.lst.front();
			//clear 'tmp' list
			tmp.clear();
			//If foper is '-' and we deal with subracting operation
			//in complex matrix, than we change foper to '+'
			if (it->eq == "-" && foper == "-") foper = "+";
			//In case of 'foper' is '+' and we deal with subtracting operation
			//in complex matrix, change foper to '-'
			if (it->eq == "-" && foper == "+") foper = "-";
			//In other cases foper operator doesn't change, and keep it for next
			//iteration. Set compilation status as true and go to next iteration
			comp = true;
			continue ;
		}
		//In case of summing or subtracting non pure matrix which is not inside
		//power-raise function or some of base matrix functions
		else if (!it->state && (it->eq == "+" && it->eq == "-")
				 && !isMatrixPure && !brace) {
			res.lst.push_back(value(0, foper));
			//Add every element of tmp value and then clear the list
			for (auto itTmp = tmp.begin(); itTmp != tmp.end(); ++itTmp)
				res.lst.push_back(*itTmp);
			tmp.clear();
			//Reset pure matrix state variable
			isMatrixPure = true;
			//If foper is '-' and we deal with subracting operation
			//in complex matrix, than we change foper to '+'
			if (it->eq == "-" && foper == "-") foper = "+";
			//In case of 'foper' is '+' and we deal with subtracting operation
			//in complex matrix, change foper to '-'
			if (it->eq == "-" && foper == "+") foper = "-";
			//Go to next iteration
			continue ;
		}
		//In case of open brace increment brace counter
		else if (it->state == 5 && (it->eq == "(")) ++brace;
		//In case of close brace decrement brace counter
		else if (it->state == 5 && (it->eq == ")")) --brace;
		tmp.push_back(*it);
	}
	//In final iteration, if calculations is not performed yet, and
	//we deal with pure matrix, let's caluclate value
	if (isMatrixPure && !comp && !tmp.empty()) {
		res.lst.front() = Execute(foper, res.lst.front(), tmp.back());
		//If there is some execution error, return it
		if (res.lst.front().state == 4) return res.lst.front();
		//If everything is alright, set result value as matrix
		//and return it
		res.state = 1;
		return res;
	}
	//In other cases, and tmp containes in the end of result list
	//and set result value as matrix
	res.lst.push_back(value(0, foper));
	for (auto it = tmp.begin(); it != tmp.end(); ++it)
		res.lst.push_back(*it);
	res.state = 1;
	return res;
}

//Summing and subtracting in case when both values are complex matrix equations
MatrixCalc::value MatrixCalc::ComplexComplexMatrixSumSub(const value &fcm, const value &scm,
									 const std::string &oper) {
	//Result value
	value res;
	//Multiplier sign
	std::string multi;
	//'Is matrix pure' state value
	bool isMatrixPure = true;
	//brace counter
	int brace = 0;
	//'was computation performed' state value
	bool comp = false;
	//Sign iterator
	auto sIt = res.lst.rbegin();

	//Let's iterate over the list of first value
	for (auto fIt = fcm.lst.begin(); fIt != fcm.lst.end(); ++fIt) {
		//In case of operation value and operation is not subtraction
		//or summing set 'isMatrixPure' as false
		if (!fIt->state && fIt->eq != "+" && fIt->eq != "-") isMatrixPure = false;
		//In case of non-matrix value (if it's operation go to next section)
		//set 'isMatrixPure' as false
		else if (fIt->state != 1 && fIt->state) isMatrixPure = false;
		//In case of summing or subracting inside braces (in case of
		//power raising or some matrix function), set 'isMatrixPure' state
		//as false
		else if (!fIt->state && (fIt->eq == "+" || fIt->eq == "-") && brace)
			isMatrixPure = false;
		//In case of summing or subtracting operation and not pure matrix
		//to operate with, reset 'isMatrixPure' value as true and set multiplier
		//for next iteration
		else if (!fIt->state && (fIt->eq == "+" || fIt->eq == "-") && !brace
				 && !isMatrixPure) {
			//Set multiplier according to encountered sign
			multi = (fIt->eq == "-") ? "-1" : "1";
			//Reset 'isMatrixPure' state as true
			isMatrixPure = true;
			//Assign 'sIt' iterator to current iterator
			res.lst.push_back(*fIt);
			sIt = res.lst.rbegin();
		}
		//In case of summing or subtracting operation
		//with pure Matrix, let's make computation, if it wasn't
		//done yet
		else if (!fIt->state && (fIt->eq == "+" || fIt->eq == "-") && isMatrixPure
				 && !brace) {
			//In case of negative multiplier, let's perform
			//an additional multiply operation
			if (multi == "-1") {
				res.lst.back() = Execute("*", value(2, "-1"), res.lst.back());
				//In case of error return it
				if (res.lst.back().state == 4) return res.lst.back();
				//change value of 'sIt' iterator depending on sign
				sIt->eq = "+";
			}
			//auxiliary value
			value aux;
			//Let's calculate value 'SimpleComplexMatrixSumSub'
			aux = SimpleComplexMatrixSumSub(res.lst.back(), scm, oper);
			//Push these values from aux to result value
			for (auto itAux = aux.lst.begin(); itAux != aux.lst.end(); ++itAux) {
				if (itAux == aux.lst.begin()) res.lst.back() = *itAux;
				else res.lst.push_back(*itAux);
			}
			//If there was computation error, return it
			if (res.lst.back().state == 4) return res.lst.back();
			//Computation is peformed so set 'comp' as true;
			comp = true;
		}
		//In case of open brace increment brace counter
		if (fIt->state == 5 && fIt->eq == "(") ++brace;
		//In case of close brace decrement brace counter
		if (fIt->state == 5 && fIt->eq == ")") --brace;
		res.lst.push_back(*fIt);
	}
	//Set state of result as matrix
	res.state = 1;
	//If no computation was performed just push
	//elements of second value list
	if (!comp) {
		//If operation is binary '-' set multi as '-1'
		multi = "-1";
		//Reset brace variable
		brace = 0;
		for (auto it = scm.lst.begin(); it != scm.lst.end(); ++it) {
			//If we only starting iteration, add binary operation
			//represented in oper variable
			if (it == scm.lst.begin()) res.lst.push_back(value(0, oper));
			//If we deal with base operator and not inside braces
			//(in case of power raising functin or some base matrix
			//function check it
			else if (!it->state && (it->eq == "+" || it->eq == "-") && !brace) {
				//If multi is a negative number change sign
				//And continue iteration
				if (multi == "-1") {
					res.lst.push_back(value(0, (it->eq == "-") ? "+" : "-"));
					continue ;
				}
			}
			//In case of open brace increment brace variable
			else if (it->state == 5 && it->eq == "(") ++brace ;
			//In case of close brace decrement brace variable
			else if (it->state == 5 && it->eq == ")") --brace;
			res.lst.push_back(*it);
		}
	}
	return res;
}

//Multiplying of simple matrix and complex matrix equation
MatrixCalc::value MatrixCalc::SimpleComplexMatrixMulti(const value &f, const value &s) {
	//Result value
	value res;
	//Auxiliary value
	value aux;
	//brace counter
	int brace = 0;
	//'computation' state
	bool comp = false;
	//'is function or brace' variable
	bool funcOrBrace = false;

	//Iterate over the list of the second value
	for (auto it = s.lst.begin(); it != s.lst.end(); ++it) {
		//In case of open brace increment brace counter
		if (it->state == 5 && it->eq == "(") ++brace;
		//In case of close brace decrement brace counter
		if (it->state == 5 && it->eq == ")") --brace;
		//In case of matrix not in inner brace (in case
		//of power-raising function or some base matrix
		//function) make multiply operation

	}
}

//Get error string
const std::string &MatrixCalc::getError() const { return error; }

//Get final value
const MatrixCalc::value MatrixCalc::getFinValue() const { return finCalc; }

//Get result of calculations as a string
const std::string MatrixCalc::getCalcResult() const { return calcResult; }
