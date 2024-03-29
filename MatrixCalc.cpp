//
// Created by WildBoarGonnaGo on 01.04.2023.
//

#include "MatrixCalc.h"
#include <cmath>
#include <cstring>
#include <limits>
#include <sstream>
#include <iomanip>
#include <algorithm>

//Comparing values
bool MatrixCalc::value::operator==(const value &s) {
	if (eq != s.eq) return false;
	else return true;
}

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
			numberUnit.eq = GetStringNumber(infixEq, i);
			numberUnit.state = 2;
			if (numberUnit.eq.empty()) {
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
			++brace;
			//Push it into stack and announcing about a left brace
			std::string brace_str;
			//If there is nothing between left and right braces
			//there should be error
			if (infixEq[i + 1] == ')') {
				error = MarkError(infixEq, i) + "\nerror: there is no content between left and right braces";
				postfixQueue.clear(); return ;
			}
			brace_str.push_back(infixEq[i]);
			if (funcBrace <= brace && funcBrace) {
				std::string tmp = oper.top();
				oper.pop();
				oper.push(brace_str);
				oper.push(tmp);
			}
			else
				oper.push(brace_str);
		}
		//Check a wrong right brace case
		else if (infixEq[i] == ')' && !brace) {
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
				postfixQueue.push_back(operUnit);
				oper.pop();
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
				else if (token == tmp) {
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
				else if (tmp == "i") {
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
					oper.push(tmp); --i; /*++funcBrace;*/ funcBrace = brace + 1; continue;
				}
				//Check if we deal with some base function and next character
				//is a left brace. If we deal with it, we push function into
				//operator stack, otherwise we check if we deal with base functions
				else if (search != operPriority.end() && infixEq[i] == '(') {
                    //In case of LPnorm function
                    if (search->first == "lpnorm") {
                        ++i;
                        oper.push(tmp);
                        //Brace counter
                        int b = 1, s = i;
                        for ( ; i < infixEq.size(); ++i) {
                            if (infixEq[i] == '(') ++b;
                            else if (infixEq[i] == ')') --b;
                            else if (!b) break;
                        }
                        postfixQueue.push_back(value(1, infixEq.substr(s, i - s)));
                    }
					oper.push(tmp); --i; /*++funcBrace;*/funcBrace = brace + 1; continue ;
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
		value operToken; operToken.state = 0; operToken.eq = oper.top();
		oper.pop();
		postfixQueue.push_back(operToken);
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
	baseFuncsMatrix = { "inv", "transp", "lpnorm", "lonenorm", "ltwonorm", "linfnorm", "det", "adj" };
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
	}
	//Generate result value
	finCalc = nums.top();
    //If result is complex value
    if (!finCalc.lst.empty())
        calcResult = FinResGenerate(finCalc, false);
	//If result is a regular expression
	else if (finCalc.state == 2) calcResult = finCalc.eq;
	//If result is a matrix
	else if (finCalc.state == 1) calcResult = finCalc.matrix.toString();
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

    //In case of token
    if (var.state == 3)
        return funcExecuteTokenOrComplex(oper, var);
    
	//In case of basic matrix function
	if (matrixSearch != baseFuncsMatrix.end()) {
		//Check is variable a matrix, if it's not return error
		if (var.state != 1) {
			error = "error: " + oper + ": " + var.eq + "isn't a matrix";
			res.state = 4; return res;
		}
		//In case of complex equation
        if (!var.lst.empty())
            return funcExecuteTokenOrComplex(oper, var);
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
		//In case of complex equation
		if (!var.lst.empty() || (search != funcs.end() && search->second.isMatrix))
			return funcExecuteTokenOrComplex(oper, var);
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

//Executing functions with token parameter
MatrixCalc::value MatrixCalc::funcExecuteTokenOrComplex(const std::string &oper, const value &var) {
    //Result value
    value res;
    //Set of functions that returns regular equation
    std::set<std::string> setRegFunc = { "sin", "cos", "tan", "exp", "sqrt", "abs", "rad",
        "acos", "asin", "atan", "ceil", "floor", "cosh", "log", "logt", "tanh", "deg", "sinh"};
    //Set of norm functions
    std::set<std::string> setNormFunc = { "lpnorm", "lonenorm", "ltwonorm", "linfnorm" };
    //Set of functions that returns matricies
    std::set<std::string> setMatrixFunc = { "inv", "transp", "det", "adj" };
    //Is function is user defined
    auto uds = funcs.find(oper);
    
    //In case of token is a complex value and we
    //deal with user defind function
    if (uds != funcs.end())
        return ExposeUserDefFunc(uds->second, var);
    //In case of token is complex value
    //Generate result value
    res.lst.insert(res.lst.end(), { value(0, oper), value(5, "(") });
    if (!var.lst.empty())
        res.lst.insert(res.lst.end(), var.lst.begin(), var.lst.end());
    else
        res.lst.push_back(var);
    res.lst.push_back(value(5, ")"));
    //If expression is amongst regular functions,
    //set result state as regular expression state
    if (auto s = setRegFunc.find(oper); s != setRegFunc.end())
        res.state = 2;
    //If expression is amongst norm functions,
    //set result state as regular expression state
    else if (auto s = setNormFunc.find(oper); s != setNormFunc.end())
        res.state = 2;
    //If expression is amongst matrix functions,
    //set result state as matrix expression state
    else if (auto s = setMatrixFunc.find(oper); s != setNormFunc.end())
        res.state = 1;
    return res;
}

//Expose user defined functions
MatrixCalc::value MatrixCalc::ExposeUserDefFunc(const Func &f, const value &var) {
	//Result value
	value res;
    //number of elements in equation
    int num;
    //equation string
    std::string expr;
    //Func equation
    std::string fEq = f.equation;
    
    //if token is complex value, let's turn
    //it into string and assign it to 'expr'
    //variable, or else just assign it's
    //equation value to our expression string
    expr = (!var.lst.empty()) ? FinResGenerate(var, false) :
		(var.state == 2) ? var.eq : var.matrix.getMatrix();
	expr = "(" + expr + ")";
    //search iterator
    size_t s = 0;
    while(true) {
        if (s = fEq.find(f.token, s); s != std::string::npos) {
            fEq.replace(s, f.token.size(), expr);
            s += expr.size();
            continue ;
        }
        break ;
    }

    //Matrix expression calculator
    MatrixCalc calc(funcs, matricies, fEq, token);
	//In case of error return it
	if (!calc.getError().empty()) { res.state = 4; return res; }
	//Calculate expression and return it
	calc.CalcIt();
    return calc.getFinValue();
}

//Return norm of p degree
void MatrixCalc::LPnorm() {
	//Expression calculator
	RevPolNotation pol(funcs);
    //Lpnorm function search iterator
    int begin = 0;

	while (infixEq.find("lpnorm", begin) != std::string::npos) {
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
		begin = ++pos;
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
		//Matrix variable and regular variable strings
		std::string matVar, regVar;
        LpnormDev(infixEq, matVar, regVar, begin, end);
        //Matrix calculator
        MatrixCalc mc(funcs, matricies, matVar, token);
        //In case of error return it
        if (!mc.getError().empty()) { error = mc.getError(); return; }
		mc.CalcIt();
		if (!mc.getError().empty()) { error = mc.getError(); return; }
		//matVar = infixEq.substr(pos, end - pos);
		pol.setInfixExpr(std::move(regVar));
		if (!pol.getErrMsg().empty()) { error = pol.getErrMsg(); return ; }
		regVar = pol.CalcIt();
		if (!pol.getErrMsg().empty()) { error = pol.getErrMsg(); return ; }
        //In case of regular expression has token or
        //matrix part is complex proceed to next lpnorm
        //function iteration
        if ((!token.empty() && regVar.find(token) != std::string::npos) || !mc.getFinValue().lst.empty()) {
            begin = ++end; continue;
        }
		m = mc.getFinValue().matrix;
		//set of matrix values
		std::vector<std::string> values = m.getValues();
		//final expression string for calculating
		std::string expr = "(";
		for (int i = 0; i < values.size(); ++i) {
			expr += "abs(" + values[i] + ")^"
					+ regVar + ((i != values.size() - 1) ? " + " : ")^(1 / " + regVar + ")");
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

void MatrixCalc::LpnormDev(const std::string &src, std::string &matEq, std::string &regEq,
              const int &begin, const int &end) {
    //round brace and squarebrace counters
    int b = 0, sb = 0;
    //midst position
    int pos;

    for (int i = begin; i < src.size(); ++i) {
        if (src[i] == '(') ++b;
        else if (src[i] == ')') --b;
        else if (src[i] == '[') ++sb;
        else if (src[i] == ']') --sb;
        else if (src[i] == ',' && !b && !sb) {
            pos = i; break ;
        }
    }
    matEq = src.substr(begin, pos - begin);
    regEq = src.substr(pos + 1, end - pos - 1);
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
	if (first.state == 1 && second.state == 1)
        res = ProcessMatricies(oper, first, second);
	//In case of first variable is matrix and second is regular expression
	else if (first.state == 1 && second.state == 2)
        res = ProcessMatrixAndRegEq(oper, first, second);
	//In case of first variable is a regular expression and second is a matrix
	else if (first.state == 2 && second.state == 1)
        res = ProcessRegEqAndMatrix(oper, first, second);
    //In case of one value is token and other is regular expression
    else if ((first.state == 3 && second.state == 2) || (first.state == 2 && second.state == 3))
        res = ProcessTokenAndRegEq(oper, first, second);
	//In case of both values are regular expressions
	else if (first.state == 2 && second.state == 2)
        res = ProcessBothRegEq(oper, first, second);
	//If first variable is matrix and second is token
	else if (first.state == 1 && second.state == 3)
        res = ProcessMatrixAndToken(oper, first, second);
	//If first variable is token and second is matrix
	else if (first.state == 3 && second.state == 1)
        res = ProcessTokenAndMatrix(oper, first, second);
    //In case of both values are tokens
    else if (first.state == 3 && second.state == 3)
        return ProcessBothTokens(oper, first, second);
	return res;
}

//Process both matricies
MatrixCalc::value MatrixCalc::ProcessMatricies(const std::string &oper, const value &f, const value &s) {
    //Result value
    value res;
    
    //in case of summing and subtract
    if (oper == "+" || oper == "-") res = MatriciesSumSub(oper, f, s);
    //in case of multiplication
    else if (oper == "*") res = MatrixMulti(f, s);
    //In case of division operation return error
    else if (oper == "/") {
        error = "error: '/': " + FinResGenerate(f, true) + ": "
        + FinResGenerate(s, true) + ": matricies division is not permitted";
        res.state = 4;
    }
    //In case of power raising return error
    else if (oper == "^") {
        error = "error: '/': " + FinResGenerate(f, true) + ": "
            + FinResGenerate(s, true) + ": matricies power raise is not permitted";
        res.state = 4;
    }
    return res;
}

//Process matrix and regular equation
MatrixCalc::value MatrixCalc::ProcessMatrixAndRegEq(const std::string &oper, const value &f, const value &s) {
    //Result value
    value res;
    
    //In case of multiplication
    if (oper == "*") res = MatrixNumMulti(s, f);
    //In case of summation return error
    else if (oper == "+") {
        error = "error: '+': " + FinResGenerate(f, true) + ": "
            + FinResGenerate(s, true) + ": summation of matrix and regular expression is not permitted";
        res.state = 4;
    }
    //In case of subtraction return error
    else if (oper == "-") {
        error = "error: '-': " + FinResGenerate(f, true) + ": "
            + FinResGenerate(s, true) + ": subtraction of matrix and regular expression is not permitted";
        res.state = 4;
    }
    //In case of division
    else if (oper == "/") {
        value tmp;
        //If second (token) value is empty
        if (s.lst.empty())
            tmp.eq = "1 / (" + s.eq + ")";
        //In other cases generate equation list
        else {
            //Insert dividing operation
            tmp.lst.insert(tmp.lst.end(), { value(2, "1"), value(0, "/") });
            //Check does secondary value has multiple elements, and if it has
            //surround secondary value with braces and push it to the
            //end of temporary list
            bool m = DoesComplexValHaveMultiple(s);
            if (m) tmp.lst.push_back(value(5, "("));
            tmp.lst.insert(tmp.lst.end(), s.lst.begin(), s.lst.end());
            if (m) tmp.lst.push_back(value(5, ")"));
        }
        res = MatrixNumMulti(tmp, f);
    }
    //In case of power raising
    else if (oper == "^") res = MatrixPowerRaise(f, s);

	return res;
}

//Process regular equation and matrix
MatrixCalc::value MatrixCalc::ProcessRegEqAndMatrix(const std::string &oper, const value &f, const value &s) {
    //Result value
    value res;
    
    //In case of multiplication of regular expression and matrix
    if (oper == "*") res = MatrixNumMulti(f, s);
    //In case of summation return error
    else if (oper == "+") {
        error = "error: '+': " + FinResGenerate(f, true) + ": "
            + FinResGenerate(s, true) + ": summation of regular expression and matrix is not permitted";
        res.state = 4;
    }
    //In case of subtraction return error
    else if (oper == "-") {
        error = "error: '-': " + FinResGenerate(f, true) + ": "
            + FinResGenerate(s, true) + ": subtraction of regular expression and matrix is not permitted";
        res.state = 4;
    }
    //In case of division return error
    else if (oper == "/") {
        error = "error: '/': " + FinResGenerate(f, true) + ": "
            + FinResGenerate(s, true) + ": division of regular expression and matrix is not permitted";
        res.state = 4;
    }
    //In case of power raise return error
    else if (oper == "^") {
        error = "error: '^': " + FinResGenerate(f, true) + ": "
            + FinResGenerate(s, true) + ": regular expression raising to a matrix is not permitted";
        res.state = 4;
    }
    
    return res;
}

//Process token and regular equation
MatrixCalc::value MatrixCalc::ProcessTokenAndRegEq(const std::string &oper, const value &f, const value &s) {
    //Result value
    value res;
    //'Is token first' statement
    bool isTokenFirst = f.state == 3;
    //Token and matrix values
    value tok = (isTokenFirst) ? f : s;
    value reg = (isTokenFirst) ? s : f;
    //Case of token is complex value
    if (!tok.lst.empty()) {
        //Set token and regular equation and perform
        //computations depending on token position.
        //If there were errors due calculation return it
        tok.state = 2;
        res = (isTokenFirst) ? Execute(oper, tok, reg) : Execute(oper, reg, tok);
        if (res.state == 4) return res;
        //If first operand is token set result state as token
        //else as regular equation and return result
        res.state = (isTokenFirst) ? 3 : 2;
        return res;
    }
    
    //In case of summing or subtraction
    if (oper == "+" || oper == "-")
        res = SumSubRegEqToken(oper, reg, tok, isTokenFirst);
    //In case of multiplication or division
    else if (oper == "*" || oper == "/")
        res =  MultiDivRegExToken(oper, reg, tok, isTokenFirst);
    //In case of power raising
    else if (oper == "^")
        res = PowRaiseRegEqToken(reg, tok, isTokenFirst);
    
    return res;
}



//Summing and subraction of regular equation and token
MatrixCalc::value MatrixCalc::SumSubRegEqToken(const std::string &oper, const value &reg, const value &tokenVal,
                                               const bool &isTokenFirst) {
    //Result value
    value res;
    
    //If regular expression is complex
    if (!reg.lst.empty())
            return SumSubComplexRegEqToken(oper, reg, tokenVal, isTokenFirst);
    //Expression calculator
    RevPolNotation pol(funcs);
    //Expression string
    std::string expr;
    
    //In case of first value is token, generate next expression
    if (isTokenFirst)
        expr = "(" + tokenVal.eq + ") " + oper + " (" + reg.eq + ")";
    //In case of token is second value, generate next expression
    else
        expr = "(" + reg.eq + ") " + oper + " (" + tokenVal.eq + ")";
    //Let's preset the result state as an error
    res.state = 4;
    //Let's calculate expression and if some error occured, return it
    pol.setInfixExpr(std::move(expr));
    if (!pol.getErrMsg().empty()) return res;
    expr = pol.CalcIt();
    if (!pol.getErrMsg().empty()) return res;
    //Set state as regular equation, and assign expression
    //result to result string ('res.eq')
    res.state = 1;
    res.eq = expr;
    return res;
}

//Complex summing or subtraction complex regular equation and token
MatrixCalc::value MatrixCalc::SumSubComplexRegEqToken(const std::string &oper, const value &reg, const value &tokenVal,
                                                      const bool &isTokenFirst) {
    //Result value
    value res;
    //Brace counter;
    int brace = 0;
    //'Is element is token' statement
    //By default it's true
    bool isToken = true;
    //'Were token computations performed' statement
    //By default it's false
    bool comp = false;
    //'Previous operation' statement. true - it's sum ('+'), false - subtraction ('-')
    //By default it's true
    bool sign = true;
    //auxiliary value
    value aux;
    
    //If token is first variable let's insert it to the end of result list
    if (isTokenFirst) res.lst.push_back(tokenVal);
    //Iterate over regular expression value
    for (auto it = res.lst.begin(); it != res.lst.end(); ++it) {
        //In case of open brace increment brace counter
        //and set 'isToken' as false
        if (it->state == 5 && it->eq == "(") { ++brace; isToken = false; }
        //In case of close brace decrement brace counter
        //and set 'isToken' as false
        else if (it->state == 5 && it->eq == ")") { --brace; isToken = false; }
        //In case of outer brace non-base-operation element and non token value
        //set isToken as false
        else if (auto s = baseOpers.find(it->eq); ((!it->state && s == std::string::npos) || it->state != 3) && !brace)
            isToken = false;
        //In case of outer brace summing or subtraction and first value is token
        else if (!it->state && (it->eq == "+" || it->eq == "-") && !brace && isTokenFirst) {
            SumSubComplexRegEqFirstToken(oper, res, aux, sign, comp, isToken);
            //If some error occured, due calculations, return it
            if (res.state == 4) return res;
            //Fix current operation in 'sign' statement
            sign = (it->eq == "-") ? false : true;
            //Clear auxiliary list and proceed to next iteration
            aux.lst.clear();
            continue ;
        }
        //In case of outer brace summing or subtraction and first value is not token
        else if (!it->state && (it->eq == "+" || it->eq == "-") && !brace && !isTokenFirst) {
            SumSubComplexRegEqIsNotFirstToken(oper, res, aux, sign, comp, isToken, tokenVal);
            //If some error occured due calculations, return it
            if (res.lst.back().state == 4) return res.lst.back();
            //Fix current operation in 'sign' statement
            sign = (it->eq == "-") ? false : true;
            //Clear auxiliary list and proceed to next iteration
            aux.lst.clear();
            continue ;
        }
        aux.lst.clear();
    }
    //If token is first element in this equaiton
    if (isTokenFirst)
        SumSubComplexRegEqFirstToken(oper, res, aux, sign, comp, isToken);
    //And in case of first equation element is not token
    else
        SumSubComplexRegEqIsNotFirstToken(oper, res, aux, sign, comp, isToken, tokenVal);
    //If no operation was performed and first value is not token
    //Append operation value ('oper') and token to the end of result list
    if (!comp && !isTokenFirst)
        res.lst.insert(res.lst.end(), { value(0, oper), tokenVal });
    //Check result for zeros
    CheckZero(res);
    //Set state as regular equation and return it
    res.state = 2;
    return res;
}

//Submethod for summing or subtraction regular equation and token, in case of token
//is first value
void MatrixCalc::SumSubComplexRegEqFirstToken(const std::string &oper, value &res, value &aux, bool &sign,
                                                           bool &comp, bool &isToken) {
    //lambda function. It returns interim operation
    auto OperChec = [](const std::string &oper, const bool &sign) {
        //Result operation as string
        std::string res;
        
        //If 'prev' sign of matrix is '-' and initially we perform summation
        //('+') interim operation is '-'
        if (oper == "+" && !sign) res = "-";
        //If 'prev' sign of matrix is '-' and initially we perform subtraction
        //('-') interim operation is '+'
        else if (oper == "-" && !sign) res = "+";
        //In other cases interim operation is equal to
        //initial operation (declared in 'oper' value)
        else res = oper;
        return res;
    };
    //Interim operation
    std::string intSign;
    
    //Set operation to perform
    intSign = OperChec(oper, sign);
    
    //In case of non-token value or already performed computations
    //assign to the end of result value list auxiliary list
    if (!isToken || comp) {
        //Push interim operation value
        res.lst.push_back(value(0, intSign));
        res.lst.insert(res.lst.end(), aux.lst.begin(), aux.lst.end());
    }
    //In other cases execute operations between token
    //and computations as performed
    else {
        res.lst.back() = Execute(intSign, res.lst.back(), aux.lst.back());
        comp = true;
    }
}

void MatrixCalc::SumSubComplexRegEqIsNotFirstToken(const std::string &oper, value &res, value &aux, bool &sign,
                                                   bool &comp, bool &isToken, const value &tokenVal) {
    //lambda function. It returns interim operation
    auto OperChec = [](const std::string &oper, const bool &sign) {
        //Result operation as string
        std::string res;
        
        //If 'prev' sign of matrix is '-' and initially we perform summation
        //('+') interim operation is '-'
        if (oper == "+" && !sign) res = "-";
        //If 'prev' sign of matrix is '-' and initially we perform subtraction
        //('-') interim operation is '+'
        else if (oper == "-" && !sign) res = "+";
        //In other cases interim operation is equal to
        //initial operation (declared in 'oper' value)
        else res = oper;
        return res;
    };
    //Interim operation
    std::string intSign;
    
    //Set operation to perform
    intSign = OperChec(oper, sign);
    
    //In case of non-token value or already performed computations
    //assign to the end of result value list auxiliary list
    if (!isToken || comp) {
        //Push operation value if it's not the first component
        if (!res.lst.empty()) res.lst.push_back(value(0, (!sign) ? "-" : "+"));
        res.lst.insert(res.lst.end(), aux.lst.begin(), aux.lst.end());
    }
    //In other cases execute operations between token
    //and computations as performed
    else {
        //temporary value
        value tmp = Execute(intSign, aux.lst.back(), tokenVal);
        //Else push it to the end of result list with operation
        res.lst.insert(res.lst.end(), { value(0, (!sign) ? "-" : "+"), tmp } );
        comp = true;
    }
}

//Multiplication or divison of regex token
MatrixCalc::value MatrixCalc::MultiDivRegExToken(const std::string &oper, const value &reg, const value &tok,
                                                 const bool &isTokenFirst) {
    //Result value
    value res;
    
    //In case of multiplication of complex regular expression and
    //token
    if (oper == "*" && !reg.lst.empty())
        return MultiComplexRegExToken(reg, tok, isTokenFirst);
    //In case of division complex regular expression and token
    else if (oper == "/" && !reg.lst.empty() && !isTokenFirst) {
        //'Does regular expression has multiple elements' statement
        bool multi = DoesComplexValHaveMultiple(reg);
        
        //If reg expression has multiple values, surround
        //it with braces and insert it to the end of result list
        if (multi) res.lst.push_back(value(5, "("));
        res.lst.insert(res.lst.end(), reg.lst.begin(), reg.lst.end());
        if (multi) res.lst.push_back(value(5, ")"));
        res.lst.insert(res.lst.end(), { value(0, "/"), tok });
        return res;
    }
    //In case of token and division complex regular
    else if (oper == "/" && !reg.lst.empty() && isTokenFirst) {
        //'Does regular expression has multiple elements' statement
        bool multi = DoesComplexValHaveMultiple(reg);
        
        //If reg expression has multiple values, surround
        //it with braces and insert it to the end of result list
        if (multi) res.lst.push_back(value(5, "("));
        res.lst.insert(res.lst.end(), reg.lst.begin(), reg.lst.end());
        if (multi) res.lst.push_back(value(5, ")"));
        res.lst.insert(res.lst.begin(), { tok, value(0, "/") });
        return res;
    }
    //In other cases set expression, depending on token position
    std::string expr = (isTokenFirst) ? "(" + tok.eq + ") " + oper + " (" + reg.eq + ")" :
        "(" + reg.eq + ") " + oper + " (" + tok.eq + ")";
    //Preset res value state as error
    res.state = 4;
    //Expression calculator
    RevPolNotation pol(funcs);
    //Calculate expression. In case of calulation error, return it
    pol.setInfixExpr(std::move(expr));
    if (!pol.getErrMsg().empty()) return res;
    expr = pol.CalcIt();
    if (!pol.getErrMsg().empty()) return res;
    //If expression result is zero, add multiplication by token
    //because token can be a matrix
    if (expr == "0") expr += " * " + token;
    //Set result as token and return it
    res.state = 3;
    res.eq = expr;
    return res;
}

//Multiplication of complex regular expression and token
MatrixCalc::value MatrixCalc::MultiComplexRegExToken(const value &reg, const value &tok, const bool &isTokenFirst) {
    //Result value
    value res;
    //Last operation outside braces. By default
    //it's multiplication
    std::string interOper = "*";
    //Brace counter
    int brace = 0;
    //auxiliary value
    value aux;
    
    //Let's iterate over complex regular expression
    for (auto it = reg.lst.begin(); it != res.lst.end(); ++it) {
        //In case of open brace increment brace counter
        if (it->state == 5 && it->eq == "(") ++brace;
        //In case of close brace decrement brace counter
        else if (it->state == 5 && it->eq == ")") --brace;
        //In case of non summing or subraction outside braces
        //assign this operation to 'interOper' variable
        else if (!it->state && it->eq != "+" && it->eq != "-" && !brace)
            interOper = it->eq;
        //In case of summing or subtracting outside braces
        else if (!it->state && (it->eq == "+" || it->eq == "-") && !brace) {
            //Multiply auxiliary element and token
            CheckAndFixAuxValueToken(aux, tok, isTokenFirst);
            //Append auxiliary list to the end of result list
            res.lst.insert(res.lst.end(), aux.lst.begin(), aux.lst.end());
            //Insert current operation value to the end of result list
            res.lst.push_back(*it);
            //Clear auxiliary list and proceed to next iteration
            aux.lst.clear();
            continue ;
        }
        aux.lst.push_back(*it);
    }
    //Multiply auxiliary element and token
    CheckAndFixAuxValueToken(aux, tok, isTokenFirst);
    //Append auxiliary list to the end of result list
    res.lst.insert(res.lst.end(), aux.lst.begin(), aux.lst.end());
    //Set state as regular equation and return it
    res.state = 2;
    return res;
}

//Check and fix auxiliary value, due regular expression and token multiplication
void MatrixCalc::CheckAndFixAuxValueToken(value &aux, const value &tok, const bool &isTokenFirst) {
    //In case of first multiplicator is token and first element
    //if auxiliary value is token
    if (isTokenFirst && aux.lst.front().state == 3) {
        //Let's execute operation
        aux.lst.front() = Execute("*", aux.lst.front(), tok);
        //If there were errors due calculations, return it
        if (aux.lst.front().state == 4) return ;
        //If result is zero, set it as '0*x', because x can be
        //a matrix
        if (aux.lst.front().eq == "0")
            aux.lst.front().eq += " * x";
    }
    //In case of first multiplicator is token and first
    //element of auxiliary value is not token insert
    //to the beginning of auxiliary list
    else if (isTokenFirst && aux.lst.front().state != 3)
        aux.lst.insert(aux.lst.begin(), { tok, value(0, "*") });
    //In case of second multiplicator is token and last element
    //if auxiliary value is token
    if (!isTokenFirst && aux.lst.back().state == 3) {
        //Let's execute operation
        aux.lst.back() = Execute("*", aux.lst.back(), tok);
        //If there were errors due calculations, return it
        if (aux.lst.front().state == 4) return ;
        //If result is zero, set it as '0*x', because x can be
        //a matrix
        if (aux.lst.back().eq == "0")
            aux.lst.back().eq += " * " + token;
    }
    //In case of second multiplicator is token and last
    //element of auxiliary value is not token insert
    //to the end of auxiliary list
    else if (!isTokenFirst && aux.lst.back().state != 3)
        aux.lst.insert(aux.lst.begin(), { value(0, "*"), tok });
}

//Power raising of token and regular equations
MatrixCalc::value MatrixCalc::PowRaiseRegEqToken(const value &reg, const value &tok, const bool &isTokenFirst) {
    //Result value
    value res;
    
    //In case of complex regular expression
    if (!reg.lst.empty()) {
        //Does regular expression has multiple values
        bool multi = DoesComplexValHaveMultiple(reg);
        
        //If regular expression has multiple values
        //surround regular expression with braces
        //and insert it to the end of result list
        if (multi) res.lst.push_back(value(5, "("));
        res.lst.insert(res.lst.end(), reg.lst.begin(), reg.lst.end());
        if (multi) res.lst.push_back(value(5, ")"));
        //In case of first value is token, set result as token
        //and add token with power raising operation to result
        //list beginning
        if (isTokenFirst) {
            res.lst.insert(res.lst.begin(), { tok, value(0, "^") } );
            res.state = 3;
        }
        //Else add it to the end of the list and set is as regular expression
        else {
            res.lst.insert(res.lst.end(), { value(0, "^"), tok });
            res.state = 1;
        }
        return res;
    }
    //Expression calculator
    RevPolNotation pol(funcs);
    //Generate expression.
    std::string expr = (isTokenFirst) ? "(" + tok.eq + ") ^ (" + reg.eq + ")" :
        "(" + reg.eq + ") ^ (" + tok.eq + ")";
    //Preset result state as an error
    res.state = 4;
    //Calcualte expression. If there were some errors due calculations, return it
    pol.setInfixExpr(std::move(expr));
    if (!pol.getErrMsg().empty()) return res;
    expr = pol.CalcIt();
    if (!pol.getErrMsg().empty()) return res;
    //If first value is token, set result as token
    //else, as regular expression
    res.state = (isTokenFirst) ? 3 : 2;
    res.eq = expr;
    return res;
}

//Computating both regular expression values
MatrixCalc::value MatrixCalc::ProcessBothRegEq(const std::string &oper, const value &f, const value &s) {
    //Result value
    value res;
    
    //In case of summing or subtraction
    if (oper == "+")
        res = SumSubRegEq(oper, f, s);
	else if (oper == "-")
		res = SumSubRegEq(oper, f, s);
    //In case of multiplication or division
    else if (oper == "*")
        res = MultiDivRegEq(oper, f, s);
	else if (oper == "/")
		res = MultiDivRegEq(oper, f, s);
    //In case of power raising
    else
        res = PowRaiseRegEq(f, s);
    return res;
}

//Computating first matrix variable and second token variable
MatrixCalc::value MatrixCalc::ProcessMatrixAndToken(const std::string &oper, const value &f, const value &s) {
    //Result value
    value res;
    
    //In case of matrix and token multiplication
    if (oper == "*")
        res = MultiTokenMatrix(f, s);
    //In case of matrix and token division
    else if (oper == "/") {
        //In case of token value is complex
        if (!s.lst.empty()) {
            //Set auxiliary value with 's' value and set it
            //state as regular equation
            value aux = s; aux.state = 2;
            //Computate result, and in case of error
            //return it
            res = Execute(oper, f, aux);
            //If there were errors due calculations,
            //return it
            if (res.state == 4) return res;
            //Set state as matrix ('1') and return result
            res.state = 1;
            return res;
        }
        //Temporary value for further calculations
        value tmp;
        //If second (token) value is empty
        tmp.eq = "1 / (" + s.eq + ")";
        //If matrix is complex insert values
        //of first matrix, then multiplication
        //and temporary value
        if (!f.lst.empty()) {
            res.lst.insert(res.lst.end(), f.lst.begin(), f.lst.end());
            //'Does result has multiple elements statement'. And if it has
            //multiple elements surround it with braces
            if (DoesComplexValHaveMultiple(res)) {
                res.lst.push_front(value(5, "("));
                res.lst.push_back(value(5, ")"));
            }
            res.lst.insert(res.lst.end(), { value(0, "*"), tmp } );
        }
        //In other cases set initialize list of values
        else res.lst.insert(res.lst.end(), { f, value(0, "*"), tmp } );
        //Simplify result equation and in case of error due calculation
        //return it
        res = AnalizeModifyValue(std::move(res));
        if (res.state == 4) return res;
        //Set result as matrix
        res.state = 1;
    }
    //In other cases just add operator
    else
        res = AddOperTokenMatrix(oper, f, s);
    return res;
}

//Computating first token variable and second matrix variable
MatrixCalc::value MatrixCalc::ProcessTokenAndMatrix(const std::string &oper, const value &f, const value &s) {
    //Result value
    value res;
    
    //In case of token and matrix multiplication
    if (oper == "*")
        res = MultiTokenMatrix(f, s);
    //In case of division return error
    else if (oper == "/") {
        error = "error: '/': " + FinResGenerate(f, true) + ": "
            + FinResGenerate(s, true) + ": division of regular expression and matrix is not permitted";
        res.state = 4;
    }
    //In case of power raise return error
    else if (oper == "^") {
        error = "error: '^': " + FinResGenerate(f, true) + ": "
            + FinResGenerate(s, true) + ": regular expression raising to a matrix is not permitted";
        res.state = 4;
    }
    //In other cases just add the operation
    else res = AddOperTokenMatrix(oper, f, s);
    return res;
}

//Computating both token values
MatrixCalc::value MatrixCalc::ProcessBothTokens(const std::string &oper, const value &f, const value &s) {
    //Result value
    value res;
    //'Complexity' statements for both values
    bool fComp = !f.lst.empty(), sComp = !s.lst.empty();
    //aux values for both elements.
    value auxF, auxS;
    
    //In case of both values are complex. Assign
    //source values to their auxiliary values
    //accordingly and set them as regular equations
    if (fComp && sComp) {
        auxF = f; auxF.state = 2;
        auxS = s; auxS.state = 2;
        //Computate expression
        res = Execute(oper, auxF, auxS);
    }
    //In case of only first value is complex
    //assign first value to first auxiliary
    //value accordingly and set it as regular equation
    else if (fComp) {
        auxF = f; auxF.state = 2;
        res = Execute(oper, auxF, s);
    }
    //In case of only second value is complex
    //assign second value to second auxiliary
    //value accordingly and set it as regular equation
    else if (fComp) {
        auxS = s; auxS.state = 2;
        res = Execute(oper, f, auxS);
    }
    //In other cases computate them as
    //as ordinary expressions
    else {
        //Expression calculator
        RevPolNotation pol(funcs);
        
        res.state = 4;
        //Set expression and calculate it
        //In case of error, return it
        std::string expr = "(" + f.eq + ") " + oper + " (" + s.eq + ")";
        pol.setInfixExpr(std::move(expr));
        if (!pol.getErrMsg().empty()) return res;
        expr = pol.CalcIt();
        if (!pol.getErrMsg().empty()) return res;
        //Set result state as token
        res.state = 3;
    }
    //If there were errors due calculations
    //return it
    if (res.state == 4) return res;
    //Set result as token, and return it
    res.state = 3;
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
        return SumSubComplexComplexRegEq(oper, first, second, 1);//return ComplexComplexMatrixSumSub(first, second, oper);
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
    if (f.lst.empty() && !s.lst.empty()) return SimpleComplexMatrixMulti(f, s);
    //In case of first value is complex matrix equation
    //and second is simple matrix value
    else if (!f.lst.empty() && s.lst.empty()) return ComplexSimpleMatrixMulti(f, s);
    //In case of both values are complex matrix equations
    else if (!f.lst.empty() && !s.lst.empty()) return ComplexComplexMatrixMulti(f, s);
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

	if (!token.empty() && f.eq.find(token) != std::string::npos) isThereToken = true;
    //In case of reg expression ('f') value is simple and matrix ('s')
    //is complex
	if (!s.lst.empty() && f.lst.empty()) return MatrixVectorNumMulti(s, f);
    //In case of reg expression ('f') value is complex and matrix ('s')
    //is simple
    else if (s.lst.empty() && !f.lst.empty()) return MultiComplexRegEqSimpleMatrix(f, s);
    //In case of both regular expression and matrix are complex values
    else if (!s.lst.empty() && !s.lst.empty()) return MultiComplexRegEqComplexMatrix(f, s);
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

    //In case of one of the values is complex
    if (!f.lst.empty() || !s.lst.empty())
        return ComplexMatrixPowerRaise(f, s);
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
	long bound = std::stol(power);
	if (!bound) return UnitMatrix(f.matrix.getColumn(), f.matrix.getRow());
	res = f;
	if (bound < 0) {
		res = Inv(res.matrix);
		bound *= -1;
	}
	//Matrix raising. In case of error, return it
	for (int i = 1; i < bound; ++i) {
		res = MatrixMulti(res, f);
		if (res.state == 4) return res;
	}
	return res;
}

//Return Unit Matrix
MatrixCalc::value MatrixCalc::UnitMatrix(const int &row, const int &column) {
	//Result value
	value res;
	//vector of values
	std::vector<std::string> vals(row * column);

	for (int i = 0; i < row; ++i) {
		for (int j = 0; j < column; ++j)
			vals[i * column + j] = (i == j) ? "1" : "0";
	}
	res.eq = "";
	res.matrix = Matrix(vals, row, column, token);
	res.state = 1;
	return res;
}

//Complex matrix power raise. 'f' is a matrix, 's' should be integer number
MatrixCalc::value MatrixCalc::ComplexMatrixPowerRaise(const value &f, const value &s) {
    //Result value
    value res;
    //'Does value have multiple elements' statements for
    //both variables ('f' and 's')
    bool mF, mS;

    //In case of first value is complex and other is simple
    if (!f.lst.empty() && s.lst.empty()) {
        //Does first value has multiple elements
        mF = DoesComplexValHaveMultiple(f);
        //If first value has multiple elements add to result
        //list open brace, 'f' value list, close brace,
        //power raise operation and second value
        if (mF) res.lst.push_back(value(5, "("));
        res.lst.insert(res.lst.end(), f.lst.begin(), f.lst.end());
        if (mF) res.lst.push_back(value(5, ")"));
        res.lst.insert(res.lst.end(), { value(0, "^"), s } );
    }
    //In case of first value is simple and other is complex
    else if (f.lst.empty() && !s.lst.empty()) {
        mS = DoesComplexValHaveMultiple(s);
        //If second value has multiple elements add to result
        //list open brace, 's' value list, close brace.
        //power raise operation and second value add to
        //the beginning of the result list
        if (mS) res.lst.push_back(value(5, "("));
        res.lst.insert(res.lst.end(), s.lst.begin(), s.lst.end());
        if (mS) res.lst.push_back(value(5, ")"));
        res.lst.insert(res.lst.begin(), { value(0, "^"), f } );
    }
    //In case of both values are complex
    else if (!f.lst.empty() && !s.lst.empty()) {
        mF = DoesComplexValHaveMultiple(f);
        mS = DoesComplexValHaveMultiple(s);

        //If one of these values have multiple elements
        //add braces as in previous cases
        if (mF) res.lst.push_back(value(5, "("));
        res.lst.insert(res.lst.end(), f.lst.begin(), f.lst.end());
        if (mF) res.lst.push_back(value(5, ")"));
        res.lst.push_back(value(0, "^"));
        if (mS) res.lst.push_back(value(5, "("));
        res.lst.insert(res.lst.end(), s.lst.begin(), s.lst.end());
        if (mS) res.lst.push_back(value(5, ")"));
    }
    //set result as matrix and return it
    res.state = 1;
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
			if (it->state == 1 && it->eq.empty()) res.append(it->matrix.getMatrix());
			//In case of regular expression
			//In case of operator (function)
			else if (!it->state) res.append(it->eq);
			//In case of brace
			else if (it->state == 5) res.append(it->eq);
			else res.append(it->eq);
		}
		//In other cases parse looking forward operations
		//In case of some function or base operator
		else if (!it->state) {
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
	//auxiliary value
	value aux;

    //In case of token is complex
    if (!token.lst.empty()) {
        //Assign token to auxiliary value and it's state
        //as regular equation ('2');
        aux = token; aux.state = 2;
        //Computate result value depending of token position
        res = (f.state == 3) ? Execute("*", aux, s) : Execute("*", f, aux);
        //If there were errors due computations, return it
        if (res.state == 4) return res;
        //Set result state as matrix and return it
        res.state = 1;
        return res;
    }
	//In case of complex matrix value
	else if (!matrix.lst.empty()) {
		for (auto it = matrix.lst.begin(); it != matrix.lst.end(); ++it) {
			//If we have '+' or '-' operation and no
			//braces encounter we multiplicate token
			//and intermidiate value
			if (it->state == 0 && !brace && (it->eq == "+" || it->eq == "-")) {
				//Remember, token can be a matrix, so appending scenario
				//Depends on token position (is it first or second value)
				//because of next matrix features:
				//	A * (B + C) = AB + AC
				//	(B + C) * A = BA + CA
				if (f.state == 3)
					aux.lst.insert(aux.lst.begin(), { token, value(0, "*") });
				else
					aux.lst.insert(aux.lst.end(), { value(0, "*"), token });
				//Insert auxiliary list to the end of result list
				res.lst.insert(res.lst.end(), aux.lst.begin(), aux.lst.end());
				//Clear auxiliary list and proceed to next
				//iteration
				aux.lst.clear();
				continue ;
 			}
			//In case of open brace increment brace counter
			else if (it->state == 5 && it->eq == "(") ++brace;
			//In case of close brace decrement brace counter
			else if (it->state == 5 && it->eq == ")") --brace;
			aux.lst.push_back(*it);
		}
		if (f.state == 3)
			aux.lst.insert(aux.lst.begin(), { token, value(0, "*") });
		else
			aux.lst.insert(aux.lst.end(), { value(0, "*"), token });
		//Insert auxiliary list to the end of result list
		res.lst.insert(res.lst.end(), aux.lst.begin(), aux.lst.end());
		//Set state as matrix and return result
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
	value tok = (f.state == 3) ? f : s;
	//Matrix value
	value matrix = (f.state == 1) ? f : s;

    //Case of token is complex value
    if (!tok.lst.empty()) {
        //Set matrix state of token in case of summing or subtraction
        //in other case as regular expression
        tok.state = (oper == "+" || oper == "-") ? 1 : 2;
        //Set token and regular equation and perform
        //computations depending on token position.
        //If there were errors due calculation return it
        res = (f.state == 3) ? Execute(oper, tok, matrix) : Execute(oper, matrix, tok);
        if (res.state == 4) return res;
        //Set result state as matrix and return it
        res.state = 1;
        return res;
    }
	//In case of complex matrix value
	else if (!matrix.lst.empty()) {
		res.lst = matrix.lst;
		//If we deal with power raising add braces
		//besides of power raising
		if (oper == "^") {
			res.lst.push_front(value(5, "("));
			res.lst.push_back(value(5, ")"));
		}
		//Add operation to the end of the list
		res.lst.push_back(value(0, oper));
		res.lst.push_back(tok);
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
	//Should we add brace in multiplying equation
	bool shAddBr = DoesComplexValHaveMultiple(m);

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
        if (it->state == 5 && it->eq == "(") { ++brace; funcOrBrace = true; }
		//In case of close brace decrement brace counter
        else if (it->state == 5 && it->eq == ")") { --brace; funcOrBrace = true; }
		//In case of matrix not in inner brace (in case
		//of power-raising function or some base matrix
		//function) and we don't encountered function or brace yet
        //(funcOrBrace is false) make multiply operation
        else if (it->state == 1 && !funcOrBrace && !brace) {
            aux.lst.push_back(Execute("*", f, *it));
            //If some error occured due calculations, return it
            if (aux.lst.back().state == 4) return aux.lst.back();
            //Set computation state as performed
            comp = true;
        }
        //In case of function encountering. set funcOrBrace as true
        else if (auto s = baseFuncsMatrix.find(it->eq); !it->state && s != baseFuncsMatrix.end())
            funcOrBrace = true;
        //In case of summing or subraction that is not inside braces
        else if (!it->state && (it->eq == "+" || it->eq == "-") && !brace) {
            //If computation wasn't performed yet, insert to the end
            //of result list 'f' multiplier and multiplying operation.
            //Or else reset 'comp' value
            if (!comp)
                res.lst.insert(res.lst.end(), { f, value(0, "*") } );
            else comp = false;
            //Insert aux list to the result list
            res.lst.insert(res.lst.end(), aux.lst.begin(), aux.lst.end());
            //Push current operation value to result list
            res.lst.push_back(*it);
            //Clear auxiliary list
            aux.lst.clear();
            //Continue iterations
            continue ;
        }
        //Push iterated value to the auxiliary list
        aux.lst.push_back(*it);
	}
    //If computation wasn't performed yet, insert to the end
    //of result list 'f' multiplier and multiplying operation.
    if (!comp)
        res.lst.insert(res.lst.end(), { f, value(0, "*") } );
    //Insert aux list to the result list
    res.lst.insert(res.lst.end(), aux.lst.begin(), aux.lst.end());
    //Set result as matrix
    res.state = 1;
    return res;
}

//Multiplying of complex matrix equation and simple matrix
MatrixCalc::value MatrixCalc::ComplexSimpleMatrixMulti(const value &f, const value &s) {
    //Result value
    value res;
    //'Is there complex summing or subracting' value
    //for first and second complex matrix equations
    bool fComp = false, sComp = false;
    //Brace counter
    int brace = 0;

    //Iterate over the first value searching
    //complex summing or subracting
    for (auto it = f.lst.begin(); it != f.lst.end(); ++it) {
        //In case of open brace increment brace counter
        if (it->state == 5 && it->eq == "(") ++brace;
        //In case of close brace decrement brace counter
        else if (it->state == 5 && it->eq == ")") --brace;
        //In case of summing or subtracting outside the
        //braces, set 'fComp' variable as true and
        //break the loop
        else if (!it->state && (it->eq == "-" || it->eq == "+") && !brace) {
            fComp = true; break ;
        }
    }
    //Iterate over the second value searching
    //complex summing or subracting
    for (auto it = s.lst.begin(); it != s.lst.end(); ++it) {
        //In case of open brace increment brace counter
        if (it->state == 5 && it->eq == "(") ++brace;
        //In case of close brace decrement brace counter
        else if (it->state == 5 && it->eq == ")") --brace;
        //In case of summing or subtracting outside the
        //braces, set 'sComp' variable as true and
        //break the loop
        else if (!it->state && (it->eq == "-" || it->eq == "+") && !brace) {
            sComp = true; break ;
        }
    }
    //In case fComp is true add open brace
    if (fComp) res.lst.push_back(value(5, "("));
    //Insert values of first equation to the end of result list
    res.lst.insert(res.lst.end(), f.lst.begin(), f.lst.end());
    //In case fComp is true add close brace
    if (fComp) res.lst.push_back(value(5, ")"));
    //Add operation value
    res.lst.push_back(value(0, "*"));
    //In case sComp is true add open brace
    if (sComp) res.lst.push_back(value(5, "("));
    //Insert values of second equation to the end of result list
    res.lst.insert(res.lst.end(), s.lst.begin(), s.lst.end());
    //In case sComp is true add close brace
    if (sComp) res.lst.push_back(value(5, ")"));
    //Set result state as matrix and return it
    res.state = 1;
    return res;
}

//Analizing and modifying complex source value
MatrixCalc::value MatrixCalc::AnalizeModifyValue(value &&src) {
    //Number of braces
    int brace = 0;
    //'Were regular expressions computated' state
    bool regComp = false;
    //Last matrix value state
    bool mState;
    //result value
    value res;
    
    //Multiply regular expressions in value
    for (auto it = src.lst.begin(); it != src.lst.end(); ++it) {
        //In case of open brace increment 'brace' value
        if (it->state == 5 && (it->eq == "(")) { ++brace; mState = false; }
        //In case of close brace decrement 'brace' value
        else if (it->state == 5 && (it->eq == ")")) { --brace; mState = false; }
        //In case of regular expression outside braces
        //seek for other regular expressions
        else if (it->state == 2 && !brace && !regComp) {
            mState = false;
            //Create auxiliary iterator
            auto auxIt = it;
            ++auxIt;
            //Temporary value
            value tmp = *it;
            //Searching other regular expression values
            //that are not inside braces
            for ( ; auxIt != src.lst.end(); ++auxIt) {
                mState = false;
                //In case of open brace increment 'brace' value
                if (it->state == 5 && (it->eq == "(")) ++brace;
                //In case of close brace decrement 'brace' value
                else if (it->state == 5 && (it->eq == ")")) --brace;
                //In case of regular expression outside braces
                else if (auxIt->state == 2 && !brace) {
                    //Go back to sign and pop it
                    --auxIt;
					//Operation value
					value oper = *auxIt;
                    auxIt = src.lst.erase(auxIt);
                    //Calculate value
                    tmp = Execute(oper.eq, tmp, *auxIt);
                    //In case of error return it
                    if (tmp.state == 4) return tmp;
                    //Erase element regular expression element
                    //from the list
                    auxIt = src.lst.erase(auxIt);
                }
            }
            //Push temporary value to result list
            res.lst.push_back(tmp);
            //Set regular expression computation sequence
            //as finished
            regComp = true;
            //Continue iterations
            continue ;
        }
        //Fix matrix value state outside braces
        else if (it->state == 1 && !mState && !brace) mState = true;
        //In case multiplication of nearest matricies
        //Perform multiplication
        else if (it->state == 1 && mState & !brace) {
            res.lst.back() = Execute("*", res.lst.back(), *it);
            //In case of error, return it
            if (res.lst.back().state == 4) return res.lst.back();
            //Continue iterations
            continue ;
        }
        //In case of multiplying operator and last matrix
        //value outside braces continue iterations
        else if (!it->state && it->eq == "*" && mState) continue ;
        //In other cases we're not encountering pure matrix
        //so set 'mState' variable as false
        else mState = false;
        res.lst.push_back(*it);
    }
    //If last element of source list is pure matrix
    //'mState' is true, perform computations
    if (mState && src.lst.back().state == 2) {
        res.lst.back() = Execute("*", res.lst.back(), src.lst.back());
        //In case of error return it
        if (res.lst.back().state == 4) return res.lst.back();
    }
    //Set final result as matrix and return it
    //Push regular part into the beginning of complex element
    res.state = 1;
    return res;
}

//Multiplying of complex matrix equations
MatrixCalc::value MatrixCalc::ComplexComplexMatrixMulti(const value &f, const value &s) {
    //Result value
    value res;
    //'Does value have multiple elements' statement for both values
    bool fComp = DoesComplexValHaveMultiple(f), sComp = DoesComplexValHaveMultiple(s);
    
    //If value have multiple elements
    //surround it with braces in result value
    //and add multiplication sign between them
    if (fComp) res.lst.push_back(value(5, "("));
    res.lst.insert(res.lst.end(), f.lst.begin(), f.lst.end());
    if (fComp) res.lst.push_back(value(5, ")"));
    res.lst.push_back(value(0, "*"));
    if (sComp) res.lst.push_back(value(5, "("));
    res.lst.insert(res.lst.end(), s.lst.begin(), s.lst.end());
    if (sComp) res.lst.push_back(value(5, ")"));
    //Set state as matrix and return it
    res.state = 1;
    
    //If non of these values has no multiple elements
    //let's analyze it
    if (!fComp && !sComp)
        res = EqAnalyzeSimplify(std::move(res));
    return res;
}

MatrixCalc::value MatrixCalc::SumSubRegEq(const std::string &oper, const value &f, const value &s) {
	//Result value
	value res;

	//In case the first regular equation is simple and
	//the second is complex
	if (f.lst.empty() && !s.lst.empty()) return SumSubSimpleComplexRegEq(oper, f, s);
	//In case the first regular equation is complex and
	//the second is simple
	else if (!f.lst.empty() && s.lst.empty()) return SumSubComplexSimpleRegEq(oper, f, s);
	//In case of both regular equations are complex
	else if (!f.lst.empty() && !s.lst.empty()) return SumSubComplexComplexRegEq(oper, f, s, 2);
	//Expression calculator, in case of error return it
	RevPolNotation pol(funcs);

	//Calculate expression. If there was error due calculations, return it
	pol.setInfixExpr("(" + f.eq + ") " + oper + " (" + s.eq + ")");
	if (!pol.getErrMsg().empty()) { error = pol.getErrMsg(); res.state = 4; }
	res.eq = pol.CalcIt();
	if (!pol.getErrMsg().empty()) { error = pol.getErrMsg(); res.state = 4; }
	//Set state as regular equation and return it
	res.state = 2;
	return res;
}

//Summing or subracting simple regular equation (f) and
//complex regular equation (s)
MatrixCalc::value MatrixCalc::SumSubSimpleComplexRegEq(const std::string &oper, const value &f, const value &s) {
	//Result value
	value res;
	//Sign status
	bool sign = true;
	//Is regular value pure
	bool pure = true;
	//Brace counter
	int brace = 0;
	//Were computations performed
	bool comp = false;
	//Is element first?
	bool first = true;
	//interim operation
	std::string intOper;
	//auxiliary value
	value aux;
	//lambda function. Return operation as string
	auto signStr = [](const std::string &oper, bool sign) {
		//'-' and '-' gives us summation
		if (oper == "-" && !sign) return "+";
		//'-' and '+' gives us subtraction
		else if (oper == "-" && sign) return "-";
		//'+' and '-' gives us subtraction
		else if (oper == "+" && !sign) return "-";
		//In other cases we have summation
		else return "+";
	};

	//Put 'f' value to result list as first
	res.lst.push_back(f);
	//Iterate over the complex values
	for (auto it = s.lst.begin(); it != s.lst.end(); ++it) {
		//In case of open brace increment brace counter and
		//set pure as false
		if (it->state == 5 && it->eq == "(") { ++brace; pure = false; }
		//In case of close brace decrement brace counter
		//and set pure as false
		else if (it->state == 5 && it->eq == ")") { --brace; pure = false; }
		//In case of non subract or sum operation set pure as false
		else if (!it->state && it->eq != "+" && it->eq != "-") pure = false;
		//In case of subraction or summing pure regular equation outside braces
		//and non comp state perform computations
		else if (!it->state && (it->eq == "+" || it->eq == "-")
				 && pure && !comp && !brace) {
			//Check sign of interim operation
			intOper = signStr(oper, sign);

			//Perform calculation between simple equation and
			//'eq'. If there was error due calculations, return it
			res.lst.back() = Execute(intOper, res.lst.back(), aux.lst.back());
			if (res.lst.back().state == 4) return res.lst.back();
			//If current operation is '-' set 'sign' as false
			//in other case set it as 'true'
			sign = (it->eq == "+") ? true : false;
			//Set computation as performed ("true")
			comp = true;
			//clear auxiliary list and continue iterations
			aux.lst.clear();
			continue ;
		}
		//Case of summing or subtraction non-pure value outside braces,
		//or pure value in case when computation were already
		//performed
		else if (!it->state && (it->eq == "+" || it->eq == "-") &&
				 (!pure || comp) && !brace) {
			intOper = signStr(oper, sign);

			//Push interim operation and complex regular equation
			res.lst.push_back(value(0, intOper));
			res.lst.insert(res.lst.end(), aux.lst.begin(), aux.lst.end());
			//Clear auxiliary list
			aux.lst.clear();
			//Reset pure value
			pure = true;
			//If current operation is '-' set 'sign' as false
			//in other case set it as 'true'
			sign = (it->eq == "+") ? true : false;
			//Proceed to next iteration
			continue ;
		}
		aux.lst.push_back(*it);
	}
	//Check sign of interim operation
	intOper = signStr(oper, sign);
	//In case of pure value and not performed computation
	if (pure && !comp) {
		//Perform calculation between simple equation and
		//'eq'. If there was error due calculations, return it
		res.lst.back() = Execute(intOper, res.lst.back(), aux.lst.back());
		if (res.lst.back().state == 4) return res.lst.back();
	}
	//In other case just operation and aux list
	//to the end of result list
	else {
		//Push interim operation and complex regular equation
		res.lst.push_back(value(0, intOper));
		res.lst.insert(res.lst.end(), aux.lst.begin(), aux.lst.end());
	}
	//Set result of value as regular equation and return it
	res.state = 2;
	return res;
}

//Summing or subtracting complex regular equation and
//simple regular equation
MatrixCalc::value MatrixCalc::SumSubComplexSimpleRegEq(const std::string &oper, const value &f, const value &s) {
	//Result value
	value res;
	//number of braces
	int brace = 0;
	//is value pure state
	bool pure = true;
	//sign value. In case of '+' it's true, else it's false
	//By default it's true
	bool sign = true;
	//'were calculations performed' state. By default it's false;
	bool comp = false;
	//Auxiliary value
	value aux;

	//Iterate over the list of the first value
	for (auto it = f.lst.begin(); it != f.lst.end(); ++it) {
		//In case of open brace increment brace counter and
		//set pure as false
		if (it->state == 5 && it->eq == "(") { ++brace; pure = false; }
		//In case of close brace decrement brace counter
		//and set pure as false
		else if (it->state == 5 && it->eq == ")") { --brace; pure = false; }
		//In case of non subract or sum operation set pure as false
		else if (!it->state && it->eq != "+" && it->eq != "-") pure = false;
		//In case of subraction or summing outside braces
		//pure regular equation and non comp state perform computations
		else if (!it->state && (it->eq == "+" || it->eq == "-")
				 && pure && !comp && !brace) {
			//Push auxiliary value to list
			res.lst.push_back(aux.lst.back());

			//Perform calculation between simple equation and
			//'eq'. If there was error due calculations, return it
			//Operations depends on sign and base 'oper' value
			res.lst.back() = Execute((!sign && oper == "-") ? "+" : "-", res.lst.back(), aux.lst.back());
			if (res.lst.back().state == 4) return res.lst.back();
			//If current operation is '-' set 'sign' as false
			//in other case set it as 'true'
			sign = (it->eq == "+") ? true : false;
			//Set computation as performed ("true")
			comp = true;
			//clear auxiliary list and continue iterations
			aux.lst.clear();
			continue ;
		}
		//Case of summing or subtraction outside braces non-pure value,
		//or pure value in case when computation were already
		//performed
		else if (!it->state && (it->eq == "+" || it->eq == "-") &&
				 (!pure || comp) && !brace) {
			//Insert operation value and aux list to the
			//end of result list
			res.lst.push_back(value(0, (!sign) ? "-" : "+"));
			res.lst.insert(res.lst.end(), aux.lst.begin(), aux.lst.end());
			//Clear auxiliary list
			aux.lst.clear();
			//Reset pure value
			pure = true;
			//If current operation is '-' set 'sign' as false
			//in other case set it as 'true'
			sign = (it->eq == "+") ? true : false;
			//Proceed to next iteration
			continue ;
		}
		aux.lst.push_back(*it);
	}
	//In case of pure value and not performed computation
	if (pure && !comp) {
		//Push auxiliary value to list
			res.lst.push_back(aux.lst.back());

		//Perform calculation between simple equation and
		//'eq'. If there was error due calculations, return it
		//Operations depends on sign and base 'oper' value
		res.lst.back() = Execute((!sign && oper == "-") ? "+" : "-", res.lst.back(), aux.lst.back());
		if (res.lst.back().state == 4) return res.lst.back();
	}
	//In other case just operation and aux list
	//to the end of result list
	else {
		//Insert operation value and aux list to the
		//end of result list
		res.lst.push_back(value(0, (!sign) ? "-" : "+"));
		res.lst.insert(res.lst.end(), aux.lst.begin(), aux.lst.end());
	}
	//Set result of value as regular equation and return it
	res.state = 2;
	return res;
}

//Summing or subtracting both complex regular expression
MatrixCalc::value MatrixCalc::SumSubComplexComplexRegEq(const std::string &oper, const value &f, const value &s, const int &state) {
	//Result value and complex auxiliary values for both
	//complex values
	value res, auxF;
	//number of braces
	int brace = 0;
	//'Is element is simple'
	//By default it's true
	bool simF = true;
	//sign value. In case of '+' it's true, else it's false
	//By default it's true
	bool signF = true;
	//Were computations of simple values performed statement
	//By default it's not, thus it's false
	bool comp = false;
	//auxiliary and general lists of computated values
	std::list<std::list<value>::const_iterator> auxLst, genLst;
	//Interim operation value
	std::string interSign = oper;
	//Temporary value
	value tmp;

	//Iteration over the list of first value
	for (auto itF = f.lst.begin(); itF != f.lst.end(); ++itF) {
		//In case of open brace increment brace variable
		//and set 'simF' as false
		if (itF->state == 5 && itF->eq == "(") { ++brace, simF = false; }
		//In case of close brace decrement brace variable
		//and set 'simF' as false
		else if (itF->state == 5 && itF->eq == ")") { --brace, simF = false; }
		//In case of operation statement and not sum or subtract
		//set 'simF' as false, thus it's not regular equation
		else if (!itF->state && itF->eq != "+" && itF->eq != "-")
			simF = false;
		//In case of summing or subtracting outside braces
		//Let's iterate over the second complex value
		else if (!itF->state && (itF->eq == "+" || itF->eq == "-") && !brace) {
			tmp = IterateOverSecValueSumSub(oper, auxF, s, signF, simF, auxLst, genLst, res, state);
			//In case of error return it
			if (tmp.state == 4) return tmp;
			//Insert to the end result list temporary value list
			res.lst.insert(res.lst.end(), tmp.lst.begin(), tmp.lst.end());
			//Clear first auxiliary value
			auxF.lst.clear();
			//Proceed to next iteration
			continue ;
		}
		auxF.lst.push_back(*itF);
	}
	//Calculate computation value for final iteratrion
	tmp = IterateOverSecValueSumSub(oper, auxF, s, signF, simF, auxLst, genLst, res, state);
	//If there was error due calculations, return it
	if (tmp.state == 4) return tmp;
	//Itearate over elements of second value to generate final expression
	IterateOverSecValueFinGen(res, genLst, oper, s);
	//Check zero elements in result value list
	CheckZero(res);
	//Set result as regular value and return it
	res.state = state;
	return res;
}

//Complex reg values values analizer for summing and subtraction
MatrixCalc::value MatrixCalc::ComplexRegEqAnalyzerSumSub(const std::string &oper, const value &f, const value &s) {
	//Number values for computations for both values
	value numF, numS;
	//Comparison beginning iterators for both values
	auto itF = f.lst.begin(), itS = s.lst.begin();
	//result value
	value res;

	//If first element is simple regular equation
	//assign it to 'numF' value and increment itF
	//for further comparisons
	if (itF->state == 2) numF = *(itF++);
	//Else we just 'numF' value as '1'
	else numF = value(2, "1");
	//The same algorithm for second complex value
	if (itS->state == 2) numS = *(itS++);
	else numS = value(2, "1");


	//Compare contents of 'f' value of the range [itF, f.lst.end)
	//to contents of 's' value of the range [itS, s.lst.end)
	//If they are not equal return empty value, with regular
	//expression state
	bool eq;
	if (std::list<value>(itF, f.lst.end()).size() != std::list<value>(itS, s.lst.end()).size())
		{ res.state = 2; return res; }
	for (auto itFC = itF, itSC = itS; itFC != f.lst.end() && itSC != s.lst.end(); ++itSC, ++itFC) {
		if (itFC->eq != itSC->eq || itFC->matrix.getMatrix() != itSC->matrix.getMatrix()
				|| itFC->state != itSC->state) { res.state = 2; return res; }
	}

	//In other cases let's calculate 'numF' and 'numS' values
	//with common postfix (no matter [itF, f.lst.end) or [itS, s.lst.end))
	res.lst.push_back(Execute(oper, numF, numS));
	//If there was some error due calculations, return it
	if (res.lst.back().state == 4) return res.lst.back();
	//If result of computation is zero return it
	if (res.lst.back().eq == "0") return res.lst.back();
	//In other cases insert to the end of the list
	//common part of both equations
	res.lst.insert(res.lst.end(), itF, f.lst.end());
	//set result state as regular equation ('2') and return it
	res.state = 2;
	return res;
}

MatrixCalc::value MatrixCalc::IterateOverSecValueSumSub(const std::string &oper,
														const value &auxF, const value &s,
														const bool &signF, const bool &simF,
														std::list<std::list<value>::const_iterator> &auxLst,
														std::list<std::list<value>::const_iterator> &genLst,
														const value &res, const int &state) {
	//Temporary value
	value tmp;
	//brace counter
	int brace = 0;
	//Interim operator value
	std::string interSign;
	//Is reviewed value a simple equation
	//By default it's true
	bool simS = true;
	//Auxiliary value of interim equation element
	value auxS;
	//Preceding an interim element ('auxS') operator value
	//Is sign is negative ('-') set it as false, else if sign
	//is positive ('+') set it as true. By default it's true
	bool signS = true;
	//lambda function. It returns final sign for computation
	auto finSign = [](const std::string &oper, const bool &signF, const bool &signS) {
		std::string interSign = oper;

		//Analize first sign and interim operator
		if (interSign == "-" && !signF) interSign = "+";
		else if (interSign == "+" && !signF) interSign = "-";

		//Analize second sign and interim operator
		if (interSign == "-" && !signS) interSign = "+";
		else if (interSign == "+" && !signS) interSign = "-";

		return interSign;
	};

	//insert to result list auxiliary value list
	tmp.lst.insert(tmp.lst.end(), auxF.lst.begin(), auxF.lst.end());
	//clear first auxiliary list
	for (auto itS = s.lst.begin(); itS != s.lst.end(); ++itS) {
	//Assign interim operation to base ('oper') intially
		interSign = oper;
		//If element was found in general iterator list
		//proceed to next iteration, thus it was computed
		//already
		if (auto s = std::find(genLst.begin(), genLst.end(), itS); s != genLst.end())
			continue ;
		//In case of open brace increment brace variable
		//and set 'simS' as false
		if (itS->state == 5 && itS->eq == "(") { ++brace, simS = false; }
		//In case of close brace decrement brace variable
		//and set 'simS' as false
		else if (itS->state == 5 && itS->eq == ")") { --brace, simS = false; }
		//In case of operation statement and not sum or subtract
		//set simF as false, thus it's not simple equation
		else if (!itS->state && itS->eq != "+" && itS->eq != "-")
			simS = false;
		//Case of summing or subtracting simple values outside braces
		else if (!itS->state && (itS->eq == "+" || itS->eq == "-") && !brace
			 && simF && simS) {
			//If Auxiliary list is empty proceed iterations
			if (auxS.lst.empty()) continue ;

			//If result list is not empty check
			//operation sign
			if (!res.lst.empty()) interSign = finSign(interSign, signF, signS);
			//Execute equation, and if there is error, return it
			tmp.lst.back() = Execute(interSign, tmp.lst.back(), auxS.lst.back());
			//If there was some error due calculations return it
			if (tmp.lst.back().state == 4) return tmp.lst.back();
			//Insert to the end general list of iterators
			//auxiliary list of iterators
			genLst.insert(genLst.end(), auxLst.begin(), auxLst.end());
			//Clear auxiliary lists
			auxLst.clear(); auxS.lst.clear();
			//Proceed to next iteration
			continue ;
		}
		//Case of summing or subracting complex values outside braces
		else if (!itS->state && (itS->eq == "+" || itS->eq == "-")
				 && !simF && !simS && !brace) {
			//If Auxiliary list is empty proceed iterations
			if (auxS.lst.empty()) continue ;
			//complex value computation result
			value valCompRes;

			//If result list is not empty check
			//operation sign
			if (!res.lst.empty()) interSign = finSign(interSign, signF, signS);
			//Calculate temporary value
			valCompRes = ComplexRegEqAnalyzerSumSub(interSign, tmp, auxS);
			//If there was some error due calculation, return it
			if (valCompRes.state == 4) return tmp;
			//If result is empty, proceed to next iteration
			else if (valCompRes.eq.empty()) continue ;
			//Insert to the end general list of iterators
			//auxiliary list of iterators
			genLst.insert(genLst.end(), auxLst.begin(), auxLst.end());
			//Clear auxiliary lists
			auxLst.clear(); auxS.lst.clear();
			//Assign 'tmp' element to calculated
			tmp = valCompRes;
			//Proceed to next iteration
			continue ;
		}
		//Push iterator to iterator auxiliary list of second value list
		auxLst.push_back(itS);
		//Push values to auxiliary value list of second value list
		auxS.lst.push_back(*itS);
	}
	//Case of summing or subtracting simple values outside braces
	//and if auxiliary element is not computated yet, that is,
	//'auxS.lst' is not empty
	if (!brace && simF && simS && !auxS.lst.empty()) {

		//If result list is not empty check
		//operation sign
		if (!res.lst.empty()) interSign = finSign(interSign, signF, signS);
		//Execute equation, and if there is error, return it
		tmp.lst.back() = Execute(interSign, tmp.lst.back(), auxS.lst.back());
		//If there was some error due calculations return it
		if (tmp.lst.back().state == 4) return tmp.lst.back();
		//Insert to the end general list of iterators
		//auxiliary list of iterators
		genLst.insert(genLst.end(), auxLst.begin(), auxLst.end());
		//Clear auxiliary iterator list
		auxLst.clear();
	}
	//Case of summing or subracting complex values outside braces
	//if auxiliary element is not computated yet, that is,
	//'auxS.lst' is not empty
	else if (!brace && !simF && !simS && !auxS.lst.empty()) {
		//If result list is not empty check
		//operation sign
		if (!res.lst.empty()) interSign = finSign(interSign, signF, signS);
		//Calculate temporary value
		tmp = ComplexRegEqAnalyzerSumSub(interSign, tmp, auxS);
		//If there was some error due calculation, return it
		if (tmp.state == 4) return tmp;
		//Insert to the end general list of iterators
		//auxiliary list of iterators
		genLst.insert(genLst.end(), auxLst.begin(), auxLst.end());
		//Clear auxiliary iterator list
		auxLst.clear();
	}
	//Set tmp state as regular equation ('2')
	tmp.state = state;
	return tmp;
}

void MatrixCalc::CheckZero(value &src) {
	//If value has only one zero value
	//set it regular equation state, exit function
	if (src.lst.size() == 1 && src.lst.back().eq == "0") {
		src.lst.clear(); src = value(2, "0");
		return ;
	}

	//In cases iterate over values of the source list
	for (auto it = src.lst.begin(); it != src.lst.end(); ++it) {
		//If we encounter zero value, remove it and
		//preceding sign
		if (it->state == 2 && it->eq == "0") {
			//If 'zero' is first value remove it
			//and next sign
			if (it == src.lst.begin()) {
				it = src.lst.erase(it);
				//Is next iterator's sign is '-' add
				//'-1' multiplicator to next value
				if (!it->state && it->eq == "-1") {
					it = src.lst.erase(it);
					src.lst.insert(src.lst.begin(), { value(2, "-1"), value(0, "*") });
				}
				//proceed to next iteration
				continue ;
			}
			//In other cases, go back to sign iterator,
			//remove it and next 'zero' value
			--it;
			it = src.lst.erase(it);
			it = src.lst.erase(it);
		}
	}
	//If only one value left, let's assign it
	//to the 'src' value, because it's most likely
	//a simple, not a complex value
	if (src.lst.size() == 1)
		src = src.lst.back();
}

void MatrixCalc::IterateOverSecValueFinGen(value &res,
										   const std::list<std::list<value>::const_iterator> &genLst,
										   const std::string &oper, const value &s) {
	//sign modificator. In case of '+' operation
	//it's true, in other case ('-') it's false . By default
	//it's true
	bool sign = true;
	//Auxiliary value that represent element of summing and subtracting
	//equation
	value aux;
	//brace counter
	int brace = 0;

	//Let's iterate over the second value 's'
	for (auto it = s.lst.begin(); it != s.lst.end(); ++it) {
		//If iterator 'it' is amongst already reviewed iterators
		//(which means it's a part of already computated value)
		//proceed to to next iterator
		if (auto s = std::find(genLst.begin(), genLst.end(), it); s != genLst.end())
			continue;
		//In case of open brace increment brace counter
		else if (it->state == 5 && it->eq == "(") ++brace;
		//In case of close brace decrement brace counter
		else if (it->state == 5 && it->eq == ")") --brace;
		//In case of summing or subtraction outside brace
		//add auxiliary sign and value to the end of
		//result value list
		else if (!it->state && (it->eq == "+" || it->eq == "-") && !brace) {
			//if auxiliary value is empty, proceed to next
			//iterator
			if (aux.lst.empty()) continue;

			res.lst.push_back(value(0, (sign) ? "+" : "-"));
			res.lst.insert(res.lst.end(), aux.lst.begin(), aux.lst.end());
			//Set next preceding operation sign
			//If oper is '-' and current operation is '-'
			//set sign as true ('+')
			if (oper == "-" && it->eq == "-") sign = true;
			//If oper is '-' and current operation is '+'
			//set sign as true ('-')
			else if (oper == "-" && it->eq == "+") sign = false;
			//In other cases it sign depends on current operation value
			else sign = (it->eq == "+") ? true : false;
			//Clear aux list and proceed to next iteration
			aux.lst.clear();
			continue ;
		}
		aux.lst.push_back(*it);
	}
}

//Multiplication of dividing regular equations
MatrixCalc::value MatrixCalc::MultiDivRegEq(const std::string &oper, const value &f, const value &s) {
	//Result value
	value res;

	//In case of one of values is empty and other is not
	//is simple and operation is multiplication
	if (((f.lst.empty() && !s.lst.empty())
		 || (!f.lst.empty() && s.lst.empty())) && oper == "*")
		res = MultiDivSimpleComplexRegEq(oper, f, s);
	//In case of first value is complex and the second is simple
	//And operatiom is division
	else if (!f.lst.empty() && s.lst.empty() && oper == "/")
		res =  MultiDivSimpleComplexRegEq(oper, f, s);
	//In case of first value is simple, second is complex,
	//and operation is division
	else if (f.lst.empty() && !s.lst.empty() && oper == "/")
		res = DivisionSimpleByComplexRegEq(f, s);
	//In case of both values are complex
	else if (!f.lst.empty() && !s.lst.empty())
		res = MultiDivBothComplexRegEq(oper, f, s);
	//In other cases calculate them as simple
	//regular values
	else {
		//Regular expression calculator
		RevPolNotation pol(funcs);
		//Expressoin string
		std::string expr = "(" + f.eq + ") " + oper + "(" + s.eq + ")";
		//Let's calculate expression, and in case of error return it
		pol.setInfixExpr(std::move(expr));
		if (!pol.getErrMsg().empty()) { res.state = 4; return res; }
		expr = pol.CalcIt();
		if (!pol.getErrMsg().empty()) { res.state = 4; return res; }
		//set state as regular equaiton
		res.state = 2;
		res.eq = expr;
	}
	return res;
}

//Multiplicatoin of simple regular equation and complex regular equation
MatrixCalc::value MatrixCalc::MultiDivSimpleComplexRegEq(const std::string &oper, const value &f, const value &s) {
	//Result value
	value res;
	//Let's determine complex value
	value complex = (f.lst.empty()) ? s : f;
	//Let's determine simple value
	value simple = (f.lst.empty()) ? f : s;
	//brace counter
	int brace = 0;
	//Preceding multiplication of complex equation element
	value prec(2, "1");
	//'Is element' simple value
	bool sim = true;
	//auxiliary value
	value aux;

	//In case of division by zero, return error
	if (simple.eq == "0" && oper == "/") {
		error = "error: " + FinResGenerate(f, true) + ": "
			+ FinResGenerate(s, true) + ": divison by zero is not permitted";
		res.state = 4; return res;
	}
	//In case of multiplication and simple value
	//is zero, return zero element
	else if (simple.eq == "0") return simple;
	//Let's iterate over the complex value
	for (auto it = complex.lst.begin(); it != complex.lst.end(); ++it) {
		//In case of open brace increment brace counter and set
		//'sim' as false
		if (it->state == 5 && it->eq == "(") { ++brace;  sim = false; }
		//In case of close brace decrement brace counter and set
		//'sim' as false
		else if (it->state == 5 && it->eq == ")") { --brace;  sim = false; }
		//In case of non summation or non subtraction operation
		//set 'sim' as false, becase we deal with non simple value
		else if (!it->state && it->eq != "+" && it->eq != "-")
			sim = false;
		//In case of first part of element is simple, assign it
		//to 'prec' value and proceed to next iteration
		else if (it->state == 2 && sim) { prec = *it; continue ; }
		//In case of summing of subtraction outside braces let's
		//append multiplication 'prec' value by 'simple' value
		//result to the beginning of auxiliary value, then
		//append aux value list to the end of result value
		else if (!it->state && (it->eq == "+" || it->eq == "-")
				 && !brace) {
			//Multiply 'prec' by 'simple'. If there was
			//error due calculations, return it
			aux.lst.push_front(Execute(oper, prec, simple));
			if (aux.lst.front().state == 4) return aux.lst.front();
			res.lst.insert(res.lst.end(), aux.lst.begin(), aux.lst.end());
			//Clear aux list
			aux.lst.clear();
			//Reset preceding multiplier, 'sim' statement
			//and proceed to next iteration
			sim = true; prec = value(2, "1"); continue ;
		}
		aux.lst.push_back(*it);
	}
	//Multiply 'prec' by 'simple'. If there was
	//error due calculations, return it
	aux.lst.push_front(Execute(oper, prec, simple));
	if (aux.lst.front().state == 4) return aux.lst.front();
	res.lst.insert(res.lst.end(), aux.lst.begin(), aux.lst.end());
	//Set result state as regular equation ('2') and return it
	res.state = 2;
	return res;
}

//Does complex value have multiple elements (with summing or subtraction operations)
bool MatrixCalc::DoesComplexValHaveMultiple(const value &f) {
	//'Does value has multiple elements' statement
	//By default, it's false
	bool state = false;
	//number of braces;
	int brace = 0;

	//Let's iterate the complex value to find
	//out, does it have multiple elements
	for (auto it = f.lst.begin(); it != f.lst.end(); ++it) {
		//In case of open brace increment brace counter
		if (it->state == 5 && it->eq == "(") ++brace;
		//In case of close brace decrement brace counter
		else if (it->state == 5 && it->eq == ")") --brace;
		//In case of summation or subtraction outside braces
		//we have multiple values, so set 'state' as true
		//and break the loop
		else if (!it->state && (it->eq == "+" || it->eq == "-") && brace)
			{ state = true; break ;  }
	}
	return state;
}

//Case of division simple value by complex value (both values are regular equations)
MatrixCalc::value MatrixCalc::DivisionSimpleByComplexRegEq(const value &f, const value &s) {
	//Result value. Initially we assign it to second element
	value res = s;
	//'Does value has multiple elements' statement
	bool state = DoesComplexValHaveMultiple(s);
	//brace counter
	int brace = 0;

	//In case of 'zero' value of first element
	//return zero value
	if (f.eq == "0") return f;
	//If state is true, add to the end and start
	//of result value braces
	if (state) {
		res.lst.push_front(value(5, "("));
		res.lst.push_back(value(5, ")"));
	}
	//Insert first value and division operation to
	//the beggining of result list
	res.lst.insert(res.lst.begin(), { f, value(0, "/") } );
	//Set result state as regular equation ('2') and return it
	res.state = 2;
	return res;
}

//Case of multiplication or division of complex regular equations
MatrixCalc::value MatrixCalc::MultiDivBothComplexRegEq(const std::string &oper, const value &f, const value &s) {
	//Result value
	value res;
	//'Does both values have multiple elements' statements
	//for both values
	bool mF = DoesComplexValHaveMultiple(f), mS = DoesComplexValHaveMultiple(s);

	//In case of one element has multiple elements and other is not
	if ((mF && !mS) || (!mF && mS))
		return MultiDivComplexRegEqs(oper, f, s, mF, mS);
	//In case of both elements have multiple values
	else if (mF && mS) {
		//Insert first element list inside braces
		res.lst.push_back(value(5, "("));
		res.lst.insert(res.lst.end(), f.lst.begin(), f.lst.end());
		res.lst.push_back(value(5, ")"));
		//Push operation value
		res.lst.push_back(value(0, oper));
		//Insert second element list inside braces
		res.lst.push_back(value(5, "("));
		res.lst.insert(res.lst.end(), s.lst.begin(), s.lst.end());
		res.lst.push_back(value(5, ")"));
		//Set it's state as regular equation and return it
		res.state = 2;
		return res;
	}
	//In other cases insert first and second value lists (f.lst adn s.lst)
	//with operation value in the midst
	res.lst.insert(res.lst.end(), f.lst.begin(), f.lst.end());
	res.lst.push_back(value(0, oper));
	res.lst.insert(res.lst.end(), s.lst.begin(), s.lst.end());
	//Set it state as regular equation
	res.state = 2;
	//Let's calculate equation and in case of error,
	//return it
	res = EqAnalyzeSimplify(std::move(res));
	if (res.state == 4) return res;
	//Check zero values is result value and return it
	CheckZero(res);
	return res;
}

//Multiplication or Division of complex regular equation with multiple elements and complex
//regular equation with one element
MatrixCalc::value MatrixCalc::MultiDivComplexRegEqs(const std::string &oper,
													const value &f, const value &s,
													const bool &mf, const bool &ms) {
	//Result value
	value res;
	//Let's determine complex value with multiple elements
	value multi = (mf) ? f : s;
	//Let's determine complex value with only one element
	value single = (mf) ? s : f;
	//brace counter
	int brace = 0;
	//auxiliary value
	value aux;

	//Let's iterate over complex value with multiple elements
	for (auto it = multi.lst.begin(); it != multi.lst.end(); ++it) {
		//In case of open brace increment brace counter
		if (it->state == 5 && it->eq == "(") ++brace;
		//In case of close brace decrement brace counter
		else if (it->state == 5 && it->eq == ")") --brace;
		//In case of summing or subtraction outside braces
		//multiply (devide) 'single' variable and element
		//of multiple complex value ('multi')
		else if (!it->state && (it->eq == "+" || it->eq == "-")
				 && !brace) {
			aux.lst.push_back(value(0, oper));
			aux.lst.insert(aux.lst.end(), single.lst.begin(), single.lst.end());
			//Let's computate and simplify equation element
			//In case of error, return it
			aux = EqAnalyzeSimplify(std::move(aux));
			if (aux.state == 4) return aux;
			//If auxiliary value is simple (list is simple)
			if (aux.lst.empty() || !aux.eq.empty()) res.lst.push_back(aux);
			//In other cases insert auxiliary list to the end of
			//result list
			else if (!aux.lst.empty())
				res.lst.insert(res.lst.end(), aux.lst.begin(), aux.lst.end());
			//Let's insert current operation to the end of result list
			//and proceed to next iteration
			res.lst.push_back(value(0, it->eq));
			continue ;
		}
	}
	CheckZero(res);
	//Set state as regular equation and return it
	res.state = 2;
	return res;
}

//Analyze element for multiplications or division
MatrixCalc::value MatrixCalc::AnalyzeForMultiDiv(const value &src, value &m, value &p,
													  const int &state) {
	//'m' is multiplyer part of 'src' value
	//Initially, by default, it's equal to "1";
	m = value(2, "1");
	//'p' is power raise part of 'src' value
	//Initially, by default, it's equal to "1";
	p = value(2, "1");
	//Last operation of value 'src', and beggining
	//if main part of value
	auto lastOp = src.lst.end(), firstOp = src.lst.begin();
	//brace counter
	int brace = 0;
	//'Is first element regular element' statement
	//By default it's true
	bool isRegFirst;
	//Result value
	value res;

	//Let's iterate over the source value
	for (auto it = src.lst.begin(); it != src.lst.end(); ++it) {
		//In case of open brace increment brace counter
		//and set 'isRegFirst' as false
		if (it->state == 5 && it->eq == "(") { ++brace; isRegFirst = false; }
		//In case of close brace decrement brace counter
		//and set 'isRegFirst' as false
		else if (it->state == 5 && it->eq == ")") { --brace; isRegFirst = false; }
		//Assign last operation outside braces to 'lastOp'
		//value
		else if (!it->state && !brace && it != src.lst.begin()) lastOp = it;
		//If 'isRegFirst' is true, and current iterator
		//points to regular value, assign it to 'm' value
		//and set isRegFirst as false and iterate firstOp
		else if (it->state == 2 && isRegFirst) {
			m = *it;
			isRegFirst = false;
			++firstOp; ++firstOp;
		}
	}
	//Generate result value list
	res.lst.insert(res.lst.end(), firstOp, lastOp);
	//If last operation for complex element was power raising
	//assign next elements after 'lastOp' iterator as power raise value
	//to 'p' variable
	if (!lastOp->state && lastOp->eq == "^") {
		p.eq.clear();
		p.lst.insert(p.lst.end(), ++lastOp, src.lst.end());
	}
	//Set result state as 'state' value and return it
	res.state = state;
	return res;
}

//Equation simplifier and analyzer for source value
//Something like bubble sort
MatrixCalc::value MatrixCalc::EqAnalyzeSimplify(value &&src) {
	//result value
	value res;
	//Set of norm functions
	std::set<std::string> setNormFunc = { "lonenorm", "lpnorm", "ltwonorm", "linfnorm", "det" };
    //Set of matrix functions
    std::set<std::string> setMatrixFunc = { "inv", "transp", "det", "adj" };
	//brace counter
	int brace = 0;
	//state of reviewd function due iteration
	//1 - matrix, 2 - regular equation
	//By default it's 2
	int state = 2;
	//auxiliary value
	value aux;

	//Iterate over source value lst with buble sort iteration
	for (auto it = src.lst.begin(); it != src.lst.end(); ++it) {
		//In case of open brace increment brace counter
		if (it->state == 5 && it->eq == "(") ++brace;
		//In case of close brace decrement brace counter
		else if (it->state == 5 && it->eq == ")") --brace;
		//If value is pure matrix or regular equation outside braces,
		//push it to result list and proceed to next iteration
		else if ((it->state == 1 || it->state == 2) && !brace)
			{ res.lst.push_back(*it); continue; }
		//If operation is multiplication or division
		//outside braces, let's iterate over next values
		//after 'it' position
		else if (!it->state && (it->eq == "*" || it->eq == "/") && !brace) {
			//In case of norm function let's iterate over next
			//values
			if (state == 2 )
                aux = RegEqAnalyzeSimplifySecondIt(src, it, state, aux);
            //In case of matrix function iterate until next value
            //is equal to previous
            if (state == 1 || state == 3)
                aux = MatrixAnalyzeSimplifySecondIt(src, it, state, aux);
            //If there was error due calculations, return it
            if (aux.state == 4) return aux;
            //If computation result is simple regular equation
            //push it to the end of result list
            else if (aux.lst.empty() && !aux.eq.empty()) res.lst.push_back(aux);
            //In other cases append result of the auxiliary list to the end of result list
            else if (!aux.lst.empty()) res.lst.insert(res.lst.end(), aux.lst.begin(), aux.lst.end());
            //Clear auxiliary list and proceed to next iteration
            aux.lst.clear();
            aux.eq.clear();
			//Let's add current operation to the end of result list
			res.lst.push_back(value(0, it->eq));
            continue ;
        }
        //If function is norm and outside of braces set state
        //as regular equation
        else if (auto s = setNormFunc.find(it->eq); !it->state && s != setNormFunc.end() && !brace)
            state = 2;
        //If function is matrix funciton (like 'inversion') and outside braces
        //set state as matrix equation
        else if (auto s = setMatrixFunc.find(it->eq); !it->state && s != setNormFunc.end() && !brace)
            state = 1;
        //In case of token outside braces set state as token
        else if (it->state == 3 && !brace)
            state = 3;
        aux.lst.push_back(*it);
	}
    //Set state for result and return it
    res.state = src.state;
    //Check for zeros and units complex expression
    CheckZeroAndUnitsInComplexValue(res);
    return res;
}

//regular equation analyzer and simplifier for
//source value second iteration
MatrixCalc::value MatrixCalc::RegEqAnalyzeSimplifySecondIt(value &src, std::list<value>::iterator &it,
                                                           const int &state, const value &aux) {
    //auxiliary iterator
    auto auxIt = it;
    //'Does value fit' statement, by default it's true
    bool fit = true;
    //Last operation iterator
    auto opIt = it;
    //brace counter
    int brace = 0;
    //Auxiliary value for second iteration
    value auxS;
    //power raise, multiplier and primary part of primary
    //auxiliary value. Let's assign them
    value pF, mF, primF;
    primF = AnalyzeForMultiDiv(aux, mF, pF, state);
    //result value. Assign it to auxiliary parameter
    value res = aux;
    
    //Let's iterate over next values
    for (++auxIt; auxIt != src.lst.end(); ++auxIt) {
        //In case of open brace increment brace counter
        if (auxIt->state == 5 && auxIt->eq == "(") ++brace;
        //In case of close brace decrement brace counter
        else if (auxIt->state == 5 && auxIt->eq == ")") --brace;
        //In case non multiplication or division operation
        //Let's compare function names of auxiliary value
        //and currently iterated function outside brace.
        //If they're not equal, set 'fit' as false
        if (!auxIt->state && !brace && auxIt->eq != "*" && auxIt->eq != "/"
            && res.lst.front().eq == auxIt->eq) fit = true;
        //In other cases set it as false
        else if (!auxIt->state && !brace && auxIt->eq != "*" && auxIt->eq != "/"
                 && auxIt->eq != "^" && res.lst.front().eq != auxIt->eq) fit = false;
        //In case of multiplication or division outside braces
        //Let's compare second auxiliary value with primary
        else if (!it->state && !brace
                 && (auxIt->eq == "*" || auxIt->eq == "/")) {
            //In case of aux simple regular value
            if (res.lst.empty() && res.state == 2 && !res.eq.empty()) {
                res = MultiDivRegEq(opIt->eq, res, auxS);
                //clear assign current operation iterator to 'opIt'
                opIt = auxIt;
                //clear secondary auxiliary list and proceed
                //to next iteration
                auxS.lst.clear();
                continue ;
            }
            //If previous setup of auxiliary values doesn't fit
            //clear second auxiliary value and proceed to next
            //iteration
            if (!fit) { auxS.lst.clear(); continue ; }
            //In other cases compare them, at first set
            //power raise, multiplier and primary part of secondary
            //auxiliary value. Let's assign them
            value pS, mS, primS;
            primS = AnalyzeForMultiDiv(auxS, mS, pS, state);
            bool eqLst = true;
            if (primF.lst.size() != primS.lst.size()) eqLst = false;
            else {
				for (auto pfIt = primF.lst.begin(), psIt = primS.lst.begin();
					 pfIt != primF.lst.end() && psIt != primS.lst.end(); ++pfIt, ++psIt) {
					if (pfIt->eq != psIt->eq || pfIt->matrix.getMatrix() != psIt->matrix.getMatrix()
							|| pfIt->state != psIt->state) {
						eqLst = false; break;
					}
				}
            }
            //If base parts are equal, calculate them
            if (eqLst/*primF.lst == primS.lst*/) {
                //Clear result list
                res.lst.clear();
                //Clear secondary auxiliary list
                auxS.lst.clear();
                //Erase iterators from opIt to current, thus there's
                //no need of them
                it = src.lst.erase(opIt, auxIt);
                //Calculate multiplier, and in case of error,
                //return it
                mF = Execute(opIt->eq, mF, mS);
                if (mF.state == 4) return mF;
                //Calculate raise power value, and in case
                //of error return it
                pF = Execute((opIt->eq == "*") ? "+" : "-", pF, pS);
                if (pF.state == 4) return pF;
                //If result of operation is zero, set result as 0
                if (mF.eq == "0") res = value(2, "0");
                //If result of operation equal to zero, set result
                //as 'mF' value
                else if (pF.eq == "0") res = mF;
                //In other cases generate new auxiliary value as initializer
                //list
                else {
                    //If 'mF' is not '1' digit, insert 'mF' value
                    //and multiplication operation to the end or result value
                    if (mF.eq != "1") res.lst.insert(res.lst.end(), { mF, value(0, "*") } );
                    //Insert primary part to the end of the list
                    res.lst.insert(res.lst.end(), primF.lst.begin(), primF.lst.end());
                    //If 'pF' is not '1' digit, insert power raising operation
                    //and 'pF' value
                    if (pF.eq != "1") res.lst.insert(res.lst.end(), { value(0, "^"), pF });
                }
            }
            //save current operation and proceed to next iteration
            opIt = auxIt;
            //reset fit statement
            fit = true;
            //clear secondary auxiliary list and proceed to next iteration
            auxS.lst.clear();
            continue ;
        }
        auxS.lst.push_back(*auxIt);
    }
    //Set result statement as regular equation
    res.state = state;
    //In case of aux simple regular value
    if (res.lst.empty() && res.state == 2 && !res.eq.empty())
        return MultiDivRegEq(opIt->eq, res, auxS);
    //If previous setup of auxiliary values doesn't fit
    //just return result
    if (!fit) return res;
    //In other cases compare them, at first set
    //power raise, multiplier and primary part of secondary
    //auxiliary value. Let's assign them
    value pS, mS, primS;
    primS = AnalyzeForMultiDiv(auxS, mS, pS, state);
    bool eqLst = true;
    if (primF.lst.size() != primS.lst.size()) eqLst = false;
	else {
		for (auto pfIt = primF.lst.begin(), psIt = primS.lst.begin();
			 pfIt != primF.lst.end() && psIt != primS.lst.end(); ++pfIt, ++psIt) {
				if (pfIt->eq != psIt->eq || pfIt->matrix.getMatrix() != psIt->matrix.getMatrix()
						|| pfIt->state != psIt->state) {
					eqLst = false; break;
			 }
		}
	}
    //If base parts are equal, calculate them
    if (eqLst /*primF.lst == primS.lst*/) {
        //Clear result list
        res.lst.clear();
        //Clear secondary auxiliary list
        auxS.lst.clear();
        //Erase iterators from opIt to current, thus there's
        //no need of them
        it = src.lst.erase(opIt, auxIt);
        //Calculate multiplier, and in case of error,
        //return it
        mF = Execute(opIt->eq, mF, mS);
        if (mF.state == 4) return mF;
        //Calculate raise power value, and in case
        //of error return it
        pF = Execute((opIt->eq == "*") ? "+" : "-", pF, pS);
        if (pF.state == 4) return pF;
        //If result of operation is zero, set result as 0
        if (mF.eq == "0") res = value(2, "0");
        //If result of operation equal to zero, set result
        //as 'mF' value
        else if (pF.eq == "0") res = mF;
        //In other cases generate new auxiliary value as initializer
        //list
        else {
            //If 'mF' is not '1' digit, insert 'mF' value
            //and multiplication operation to the end or result value
            if (mF.eq != "1") res.lst.insert(res.lst.end(), { mF, value(0, "*") } );
            //Insert primary part to the end of the list
            res.lst.insert(res.lst.end(), primF.lst.begin(), primF.lst.end());
            //If 'pF' is not '1' digit, insert power raising operation
            //and 'pF' value
            if (pF.eq != "1") res.lst.insert(res.lst.end(), { value(0, "^"), pF });
        }
    }
    //Return result
    return res;
}

//Matrix analyzer and simplifier for source value second
//iteration
MatrixCalc::value MatrixCalc::MatrixAnalyzeSimplifySecondIt(value &src, std::list<value>::iterator &it,
                                    const int &state, const value &aux) {
    //result value. Initially set it as auxiliary value
    value res = aux;
    //Secondary iterator
    auto auxIt = it;
    //operation iterator
    auto opIt = it;
    //brace counter
    int brace = 0;
    //Secondary auxiliary value
    value auxS;
    //power raise, multiplier and primary part of primary
    //auxiliary value. Let's assign them
    value pF, mF, primF;
    primF = AnalyzeForMultiDiv(aux, mF, pF, state);
    
    for (++auxIt; auxIt != src.lst.end(); ++auxIt) {
        //In case of open brace increment brace counter
        if (auxIt->state == 5 && auxIt->eq == "(") ++brace;
        //In case of close brace decrement brace counter
        else if (auxIt->state == 5 && auxIt->eq == ")") --brace;
        //In case of not fitting function outside braces return result
        else if (!auxIt->state && auxIt->eq != "*" && auxIt->eq != "/" && auxIt->eq != "^"
                 && auxIt->eq != res.lst.front().eq && !brace) return res;
        //In case of regular equation outside braces return result
        else if (auxIt->state == 2 && !brace) return res;
        //In case multiplication outside braces
        else if (!auxIt->state && (auxIt->eq == "*" || auxIt->eq == "/") && !brace) {
            //In other cases compare them, at first set
            //power raise, multiplier and primary part of secondary
            //auxiliary value. Let's assign them
            value pS, mS, primS;
            primS = AnalyzeForMultiDiv(auxS, mS, pS, state);
            //primS = AnalyzeForMultiDiv(auxS, mS, pS, state);
			bool eqLst = true;
			if (primF.lst.size() != primS.lst.size()) eqLst = false;
			else {
				for (auto pfIt = primF.lst.begin(), psIt = primS.lst.begin();
					 pfIt != primF.lst.end() && psIt != primS.lst.end(); ++pfIt, ++psIt) {
					if (pfIt->eq != psIt->eq || pfIt->matrix.getMatrix() != psIt->matrix.getMatrix()
							|| pfIt->state != psIt->state) {
						eqLst = false; break;
					}
				}
			}
            //If base parts are equal, calculate them
            if (eqLst) {
				//Erase computated values from source list
				//so they are already used in calculations
				it = src.lst.erase(opIt, auxIt);
				//Calculate multiplier, and in case of error,
				//return it
				mF = Execute("*", mF, mS);
				if (mF.state == 4) return mF;
				//Calculate power raising value
				pF = Execute("+", pF, pS);
				if (pF.state == 4) return pF;
				//Generate result by initializer list
				res.lst.clear();
				if (mF.eq != "1") res.lst.insert(res.lst.end(), { mF, value(0, "*") });
				//res.lst = { mF, value(0, "*"), primF, value(0, "^"), pF };
				res.lst.insert(res.lst.end(), primF.lst.begin(), primF.lst.end());
				if (pF.eq != "1") res.lst.insert(res.lst.end(), { value(0, "^"), pF });
				//If operation is divison, return result
				if (auxIt->eq == "/") return res;
				//Else set assign current iterator to
				//current iterator
				opIt = auxIt;
            }
            //Else return result
            else return res;
            //clear secondary auxiliary list
            //and proceed to next iteration
            auxS.lst.clear();
            continue;
        }
        auxS.lst.push_back(*auxIt);
    }
	//In other cases compare them, at first set
	//power raise, multiplier and primary part of secondary
	//auxiliary value. Let's assign them
	value pS, mS, primS;
	primS = AnalyzeForMultiDiv(auxS, mS, pS, state);
	//If base parts are equal, calculate them
	bool eqLst = true;
	if (primF.lst.size() != primS.lst.size()) eqLst = false;
	else {
		for (auto pfIt = primF.lst.begin(), psIt = primS.lst.begin();
			 pfIt != primF.lst.end() && psIt != primS.lst.end(); ++pfIt, ++psIt) {
			if (pfIt->eq != psIt->eq || pfIt->matrix.getMatrix() != psIt->matrix.getMatrix()
					|| pfIt->state != psIt->state) {
				eqLst = false; break;
			}
		}
	}
	if (eqLst) {
		//Erase computated values from source list
		//so they are already used in calculations
		it = src.lst.erase(opIt, auxIt);
		//Calculate multiplier, and in case of error,
		//return it
		mF = Execute("*", mF, mS);
		if (mF.state == 4) return mF;
		//Calculate power raising value
		pF = Execute("+", pF, pS);
		if (pF.state == 4) return pF;
		//Generate result by initializer list
		res.lst.clear();
		if (mF.eq != "1") res.lst.insert(res.lst.end(), { mF, value(0, "*") });
		//res.lst = { mF, value(0, "*"), primF, value(0, "^"), pF };
		res.lst.insert(res.lst.end(), primF.lst.begin(), primF.lst.end());
		if (pF.eq != "1") res.lst.insert(res.lst.end(), { value(0, "^"), pF });
		//If operation is divison, return result
		if (auxIt->eq == "/") return res;
		//Else set assign current iterator to
		//current iterator
		opIt = auxIt;
	}
	//Set result state and return it
	res.state = state;
	return res;
}

//Regular equation power raising
MatrixCalc::value MatrixCalc::PowRaiseRegEq(const value &f, const value &s) {
	//Result value
	value res;
	//'Does value have multiple elements' statements for
	//both variables ('f' and 's')
	bool mF, mS;

	//In case of first value is complex and other is simple
	if (!f.lst.empty() && s.lst.empty()) {
		//Does first value has multiple elements
		mF = DoesComplexValHaveMultiple(f);
		//If first value has multiple elements add to result
		//list open brace, 'f' value list, close brace,
		//power raise operation and second value
		if (mF) res.lst.push_back(value(5, "("));
		res.lst.insert(res.lst.end(), f.lst.begin(), f.lst.end());
		if (mF) res.lst.push_back(value(5, ")"));
		res.lst.insert(res.lst.end(), { value(0, "^"), s } );
	}
	//In case of first value is simple and other is complex
	else if (f.lst.empty() && !s.lst.empty()) {
		mS = DoesComplexValHaveMultiple(s);
		//If second value has multiple elements add to result
		//list open brace, 's' value list, close brace.
		//power raise operation and second value add to
		//the beginning of the result list
		if (mS) res.lst.push_back(value(5, "("));
		res.lst.insert(res.lst.end(), s.lst.begin(), s.lst.end());
		if (mS) res.lst.push_back(value(5, ")"));
		res.lst.insert(res.lst.begin(), { value(0, "^"), f } );
	}
	//In case of both values are complex
	else if (!f.lst.empty() && !s.lst.empty()) {
		mF = DoesComplexValHaveMultiple(f);
		mS = DoesComplexValHaveMultiple(s);

		//If one of these values have multiple elements
		//add braces as in previous cases
		if (mF) res.lst.push_back(value(5, "("));
		res.lst.insert(res.lst.end(), f.lst.begin(), f.lst.end());
		if (mF) res.lst.push_back(value(5, ")"));
		res.lst.push_back(value(0, "^"));
		if (mS) res.lst.push_back(value(5, "("));
		res.lst.insert(res.lst.end(), s.lst.begin(), s.lst.end());
		if (mS) res.lst.push_back(value(5, ")"));
	}
	//In other cases both values are simple and
	//calculate them with regular expression calculator
	else {
		//Expression calculator
		RevPolNotation pol(funcs);
		//expression itself
		std::string exp = "(" + f.eq + ") ^ (" + s.eq + ")";
		//Calculate expression, in case of error
		//return it
		pol.setInfixExpr(std::move(exp));
		if (!pol.getErrMsg().empty()) { res.state = 4; return res; }
		exp = pol.CalcIt();
		if (!pol.getErrMsg().empty()) { res.state = 4; return res; }
		//Assign calculation result to result equation
		res.eq = exp;
	}
	//Set state as regular equation, and return it
	res.state = 2;
	return res;
}

//Multiplication of complex regular expression and simple matrix
MatrixCalc::value MatrixCalc::MultiComplexRegEqSimpleMatrix(const value &r, const value &m) {
    //Result value
    value res;
    //'Does regular expression has multiple values statement' state
    bool multiEl = DoesComplexValHaveMultiple(r);
    
    //If regular expression has multiple elements
    //we should surround it with braces
    if (multiEl) res.lst.push_back(value(5, "("));
    res.lst.insert(res.lst.end(), r.lst.begin(), r.lst.end());
    if (multiEl) res.lst.push_back(value(5, ")"));
    //Now add multiplication operation and matrix value to
    //the end of result list
    res.lst.insert(res.lst.end(), { value(0, "*"), m });
    //Set state as matrix and return it
    res.state = 1;
    return res;
}

//Mutliplication of complex regular expression and complex matrix
MatrixCalc::value MatrixCalc::MultiComplexRegEqComplexMatrix(const value &r, const value &m) {
    //Result value
    value res;
    //'Does value has multiple elements' statement
    bool multiElRegEq = DoesComplexValHaveMultiple(r),
        multiElMatrix = DoesComplexValHaveMultiple(m);
    
    //If regular expression has multiple elements
    //we should surround it with braces
    if (multiElRegEq) res.lst.push_back(value(5, "("));
    res.lst.insert(res.lst.end(), r.lst.begin(), r.lst.end());
    if (multiElRegEq) res.lst.push_back(value(5, ")"));
    //Add multiplication to the end of the list
    res.lst.push_back(value(0, "*"));
    //If matrix expression has multiple elements
    //we should surround it with braces
    if (multiElMatrix) res.lst.push_back(value(5, "("));
    res.lst.insert(res.lst.end(), m.lst.begin(), m.lst.end());
    if (multiElMatrix) res.lst.push_back(value(5, ")"));
    //If both values doesn't have multiple elements
    //let's simplify this equation. And if some error occured
    //due calculations return it
    if (!multiElMatrix && !multiElRegEq) {
        res = EqAnalyzeSimplify(std::move(res));
        if (res.state == 4) return res;
    }
    //Set state as matrix and return it
    res.state = 1;
    return res;
}

//Check zero and unit values in complex equation
void MatrixCalc::CheckZeroAndUnitsInComplexValue(value &src) {
    //brace counter
    int brace = 0;
    //'Zero multiplier outside braces' statement
    //Initially it's false;
    bool zero = false;
    //Iterator savepoint.
    auto save = src.lst.begin();
    //Set of norm functions
    std::set<std::string> setNormFunc = { "lonenorm", "lpnorm", "ltwonorm", "linfnorm", "det" };
    //Set of matrix functions
    std::set<std::string> setMatrixFunc = { "inv", "transp", "det", "adj" };
    //Set of multiplier
    int state = 0;
    
    //Let's try to find 'zero' value in complex expression
    for (auto it = src.lst.begin(); it != src.lst.end(); ++it) {
        //In case of open brace, increment brace counter
        if (it->state == 5 && it->eq == "(") ++brace;
        //In case of close brace, decrement brace counter
        else if (it->state == 5 && it->eq == ")") --brace;
        //In case of zero outside braces set 'zero' as true
        else if (it->state == 2 && it->eq == "0" && !brace)
            { zero = true; break ; }
    }
    //Let's iterate over source list values
    for (auto it = src.lst.begin(); it != src.lst.end(); ++it) {
        //In case of open brace, increment brace counter
        if (it->state == 5 && it->eq == "(") ++brace;
        //In case of close brace, decrement brace counter
        else if (it->state == 5 && it->eq == ")") --brace;
        //In case of unit multiplier delete it or in case of
        //Or in case of multiplication complex regular element by zero
        else if (state == 2 && ((it->state == 2 && it->eq == "1")
                || (!it->state && (it->eq == "*" || it->eq == "/") && zero)) && !brace) {
            if (save == src.lst.begin()) ++it;
            it = src.lst.erase(save, it);
            save = it;
            if (save != src.lst.begin()) --it;
        }
        //In case of non-regular element and multiplication or
        //dividing operation, refresh 'save' iterator
        if (!it->state  && (it->eq == "*" || it->eq == "/") && (!zero || state != 2))
            save = it;
        //If function is norm and outside of braces set state
        //as regular equation
        else if (auto s = setNormFunc.find(it->eq); !it->state && s != setNormFunc.end() && !brace)
            state = 2;
        //If function is matrix funciton (like 'inversion') and outside braces
        //set state as matrix equation
        else if (auto s = setMatrixFunc.find(it->eq); !it->state && s != setNormFunc.end() && !brace)
            state = 1;
        //In case of token outside braces set state as token
        else if (it->state == 3 && !brace)
            state = 3;
    }
    if (src.lst.size() == 1) {
        if (src.lst.back().state == 1)
            src.matrix = src.lst.back().matrix;
        else if (src.lst.back().state == 2 || src.lst.back().state == 3)
            src.eq = src.lst.back().eq;
        src.state = src.lst.back().state;
        src.lst.clear();
    }
}

//Get error string
const std::string &MatrixCalc::getError() const { return error; }

//Get final value
const MatrixCalc::value MatrixCalc::getFinValue() const { return finCalc; }

//Get result of calculations as a string
const std::string MatrixCalc::getCalcResult() const { return calcResult; }
