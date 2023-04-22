#include "EquationProc.h"
#include "computor.h"
#include "MatrixCalc.h"
#include <iostream>

EquationProc::EquationProc() {
	gp << "set term qt title 'lchantel - computorv2'" << std::endl;
}

//Add equation: either function or variable, or some equation.
void EquationProc::AddEquation(std::string &&equation) {
	int				equalSignPos, state, questMark;
	std::string		sample, errMsg, curToken, entityName, oldValue;
	RevPolNotation	parser(funcs);
	//We don't know, if we deal with matrix or not,
	//so set signal as false
	isMatrix = false;
	isFunc = false;
	//Clear expression strings for further calculations
	regEqStr.clear(); matrixEq.clear();
	//Lambda function. it returns position if there is some base function
	//else it returns -1
	auto ifBaseFuncIn = [](const std::string &src, std::string &error) {
		std::set<std::string> baseFuncs = { "sin", "cos", "tan", "exp", "sqrt", "abs", "rad",
										   "acos", "asin", "atan", "ceil", "floor", "cosh",
										   "log", "logt", "tanh", "deg", "sinh" };
		for (std::set<std::string>::iterator it = baseFuncs.begin(); it != baseFuncs.end(); ++it) {
			int search = src.find(*it);
			if (search != std::string::npos) {
				error = "\nerror: 2-nd degree equation: '" + *it + "' function found";
				return search;
			}
		}
		return -1;
	};

	history.push_back(std::move(equation));
	//sample = history.back();
	if (equation.find('>') == std::string::npos)
		history.back().insert(0, ">> ");
	sample = history.back().substr(history.back().find_first_not_of("	> "));
	if (EqualSignsNum(sample, equalSignPos) != 1) {
		history.push_back("error: equation must have only one equal sign");
		std::cerr << history.back() << std::endl;
		return ;
	}
	//Check if there is some question mark
	questMark = QuestionMarkPreview(sample, errMsg, curToken);
	//If there is some error, push it to history and exit
	if (!questMark) {
		history.push_back(errMsg);
		std::cerr << history.back() << std::endl;
		return ;
	}
	//If we deal with simple computational part
	if (questMark == 1) {
		equalSignPos = sample.find('=') - 1;

		while (std::isspace(sample[equalSignPos])) --equalSignPos;
		sample = sample.substr(0, equalSignPos + 1);
		//If we deal with matricies calculate is as matrix
		if (isMatrix) {
			MatrixCalc calc(funcs, matricies, sample);
			if (!(RetExprError(calc.getError()))) return ;
			sample = calc.CalcIt();
			if (!(RetExprError(calc.getError()))) return ;
			else {
				history.push_back(calc.getCalcResult());
				std::cout << history.back() << std::endl;
			}
			return ;
		}
		//Set token for parser
		parser.setToken(curToken);
		//Calculate expression. In case of error return it
		parser.setInfixExpr(std::move(sample));
		if (!(RetExprError(parser.getErrMsg()))) return ;
		history.push_back(parser.CalcIt());
		std::cout << history.back() << std::endl;
		return ;
	}
	//If we deal with complex computational part
	if (questMark == 2) {
		//If there is some matrix in comptutational part
		//return error
		if (isMatrix) {
			history.push_back(std::string("error: binary function calculation: there is a matrix function\n")
			 + "or matrix variable, which is not permitted in this case");
			return ;
		}
		//Result parsed equation
		int delim = sample.find('=');
		while (sample[delim] == '=' || std::isspace(sample[delim])) --delim;
		//Left part of source equation
		std::string leftEq = sample.substr(0, delim + 1);
		//Right part of equation without question mark
		std::string rightEq = sample.substr(sample.find_first_not_of(" = 	", sample.find('=')));
		delim = rightEq.size();
		while (rightEq[delim - 1] == '?' || std::isspace(rightEq[delim - 1])) --delim;
		rightEq.resize(delim);
		//Set token for parser
		parser.setToken(curToken);
		//Parse left and right equations and in case of error
		//return it
		parser.setInfixExpr(std::move(leftEq));
		if (!(RetExprError(parser.getErrMsg()))) return ;
		leftEq = parser.CalcIt();
		if (!(RetExprError(parser.getErrMsg()))) return ;
		parser.setInfixExpr(std::move(rightEq));
		if (!(RetExprError(parser.getErrMsg()))) return ;
		rightEq = parser.CalcIt();
		if (!(RetExprError(parser.getErrMsg()))) return ;
		leftEq.append(" - (");
		leftEq.append(rightEq);
		leftEq.push_back(')');
		//Check if equation has some base function
		if ((delim = ifBaseFuncIn(leftEq, errMsg)) != -1) {
			RetError(errMsg, std::string(errMsg), delim, leftEq);
			history.push_back(errMsg);
			std::cerr << history.back() << std::endl;
			return;
		}
		//Let's calculate final equation
		parser.setInfixExpr(std::move(leftEq));
		leftEq = parser.CalcIt();
		leftEq.append(" = 0");
		//let's solve equation
 		computor solve(leftEq, curToken);
		history.push_back(solve.to_string());
		std::cout << history.back() << std::endl;
		return ;
	}
	//Parsing variable or function
	state = EntityDefine(errMsg, sample, entityName, oldValue);
	//If we deal with matricies, set sample as a matrix equation (matrixEq)
	//else set sample as a regular equation (regEqStr)
	if (!matrixEq.empty()) sample = matrixEq;
	else sample = regEqStr;
	//if error occured
	if (!state) {
		history.push_back(errMsg);
		std::cerr << errMsg << std::endl;
		return ;
	}
	isFunc = (state == 2) ? true : false;
	//Check error in equations, and if there are, return it
	if (!InitEquationParse(sample, errMsg, entityName, oldValue)) {
		history.push_back(errMsg);
		std::cerr << history.back() << std::endl;
		//Remove function or variable from database
		if (isFunc) funcs.erase(entityName);
		else vars.erase(entityName);
		return ;
	}
	//Variable case calculation
	else if (state == 1) {
		//Calculate expression, in case of error return it
		parser.setInfixExpr(std::move(sample));
		if (!(RetExprError(parser.getErrMsg()))) { vars.erase(entityName); return ; }
		vars[entityName] = parser.CalcIt();
		if (!(RetExprError(parser.getErrMsg()))) { vars.erase(entityName); return ; }
		history.push_back(vars[entityName]);
		std::cout << history.back() << std::endl;
	}
	//Function case calculation
	else if (state == 2) {
		parser.setToken(funcs[entityName].token);
		parser.setInfixExpr(std::move(sample));
		if (!(RetExprError(parser.getErrMsg()))) { funcs.erase(entityName); return ; }
		funcs[entityName].equation = parser.CalcIt();
		if (!(RetExprError(parser.getErrMsg()))) { funcs.erase(entityName); return ; }
		history.push_back(funcs[entityName].equation);
		std::cout << history.back() << std::endl;
	}
	//Matrix case calculation
	else {
		matrixEq.clear();
		//Matrix expression calculator
		MatrixCalc calc(funcs, matricies, sample);
		//In case of error, result old value if
		if (!calc.getError().empty()) {
			history.push_back(calc.getError());
			if (!oldValue.empty()) {
				if (!oldValueIsMatrix) vars[entityName] = oldValue;
			}
			std::cerr << history.back() << std::endl;
			return ;
		}
		//Calculate result, in case of calculation error, return it
		std::string resTmp = calc.CalcIt();
		if (!calc.getError().empty()) {
			history.push_back(calc.getError());
			if (!oldValue.empty()) {
				if (!oldValueIsMatrix) vars[entityName] = oldValue;
			}
			std::cerr << history.back() << std::endl;
			return ;
		}
		//If calculation was succesfull, check type of variable
		//In case of regular value
		if (calc.getFinValue().state == 2) {
			vars[entityName] = resTmp;
			history.push_back(vars[entityName]);
		}
		//In case of matrix
		else {
			matricies[entityName] = calc.getFinValue().matrix;
			history.push_back(matricies[entityName].toString());
		}
		std::cout << history.back() << std::endl;
	}
}

//Method to define string represents a variable or a function
//it returns integer value represents next states: 1 - we're dealing with variable
//2 - we're dealing with function; 3 - if we deal with matrix; 0 - error
int EquationProc::EntityDefine(std::string &error, const std::string &src,
							   std::string &res, std::string &oldValue) {
	int i = 0, isfunc = 0;
	Func func;
	//Equation string
	std::string eq;
	//Set of base matrix functions
	std::set<std::string> baseMatrixFuncs = { "inv", "transp", "lonenorm", "ltwonorm", "linfnorm",
											  "lpnorm", "det", "adj" };
	//Seeking name of variables (function) and name of token
	//passing whitespaces
	auto indx = [](const std::string &src, std::string &dst, int i) {
		while (std::isspace(src[i])) ++i;
		for ( ; i < src.size(); ++i) {
			if (!std::isalpha(src[i])) break ;
			dst.push_back(src[i]);
		}
		return i;
	};

	i = indx(src, res, i);
	//If digit occured return erro
	//Using 'plot' name is not permitted, it's already
	//taken for plotting function
	if (res == "plot") {
		return RetError(error, "\nerror: 'plot' name for function or variable is not permitted",
						0, src);
	}
	//if name is equal to 'i' return error
	if (res == "i")
		return RetError(error, "\nerror: function or variables name can't be 'i', it's complex number",
						0, src);
	if (res == "pi")
		return RetError(error, "\nerror: function or variables name can't be 'pi', it's math constant",
						0, src);
	while (std::isspace(src[i])) ++i;
	//If name of variable or function is empty, return error
	if (res.empty() && src[i] == '=')
		return RetError(error, "\nerror: no name for function or variable is defined",
						0, src);
	//If some obscure character found return error
	if (src[i] != '(' && src[i] != '=')
		return RetError(error, "\nerror: function or variable name must consist of alphabetical characters only",
						i, src);
	if (src[i] == '(') {
		//Let's fix brace position
		int brace = 0;
		isfunc = 1;
		//if number of braces doesn't fit, return error
		for (int j = i; j < src.size(); ++j) {
			if (src[j] == '(') ++brace;
			else if (src[j] == ')') --brace;
			if (!brace) break;
		}
		if (brace)
			return RetError(error, "\nerror: there is no right brace for function token",
							i, src);
		funcs[res] = func;//func.name = std::move(res);
		//func = src.substr(0,  i++);
		i = indx(src, funcs[res].token, i + 1);
		if (src[i] != ')') {
			funcs.erase(res);
			return RetError(error, "\nerror: function token must consist of only alphabetical characters",
							i, src);
		}
		else if (funcs[res].token.empty()/*func.token.empty()*/) {
			funcs.erase(res);
			return RetError(error, "\nerror: token can't be empty", i - 1, src);
		}
		++i;
	}
	i = src.find_first_not_of( " 	", i);
	if (src[i] != '=') {
		if (isfunc) funcs.erase(res);
		return RetError(error, "\nerror: equal sign is not in proper place", i, src);
	}
	//Assigning equation string
	eq = src.substr(src.find_first_not_of(" =	", i));
	//Lets check if equation contains any information, and if it's not
	//return error.
	if (eq.empty()) {
		if (isfunc) funcs.erase(res);
		return RetError(error, "\nerror: actual expression is empty", i, src);
	}
	isMatrix = false;
	//Search squarebraces in function or variable
	//If there is, we deal with matrix, so set isMatrix variable as true
	if (eq.find('[') != std::string::npos || eq.find(']') != std::string::npos)
		isMatrix = true;
	//Search matrix, in function or variable
	//If there is, set isMatrix variable as true
	//If isMatrix is already true, break the loop
	for (std::map<std::string, Matrix>::iterator it = matricies.begin(); it != matricies.end(); ++it) {
		if (isMatrix) break ;
		if (auto search = eq.find(it->first); search != std::string::npos) {
			isMatrix = true; break;
		}
	}
	//Search matrix function in function or variable
	//If there is, set isMatrix variable as true
	//If isMatrix is already true, break the loop
	for (std::set<std::string>::iterator it = baseMatrixFuncs.begin(); it != baseMatrixFuncs.end(); ++it) {
		if (isMatrix) break;
		if (auto search = eq.find(*it); search != std::string::npos) {
			isMatrix = true; break ;
		}
	}
	//If we deal with function
	if (isfunc) {
		//If there is matrix in function, return error
		if (isMatrix) {
			funcs.erase(res);
			return RetError(error,
							"\nerror: " + res + ": matricies function and variable in function is not supported",
							i, src);
		}
		funcs[res].equation = eq;
		funcs[res].isMatrix = isMatrix;
		regEqStr = funcs[res].equation;
		return 2;
	}
	//If there is some old matrix value, assign current matrix value to old value
	//and if equation has old value, we deal with matricies
	if (auto search = matricies.find(res); search != matricies.end()) {
		oldValue = matricies[res].getMatrix();
		if (eq.find(oldValue) != std::string::npos || eq.find(res) != std::string::npos)
			{ isMatrix = true; oldValueIsMatrix = true; }
	}
	//If there is some old regular value value, assign current variable to old value
	else if (auto search = vars.find(res); search != vars.end())
		{ oldValue = vars[res]; oldValueIsMatrix = false; }
	//If we deal with matricies
	if (isMatrix) {
		//If there is regular value with 'res' name, delete it
		if (auto search = vars.find(res); search != vars.end()) vars.erase(res);
		matrixEq = src.substr(src.find_first_not_of(" =", i));
		return 3;
	}
	//If we deal with variables
	vars[res] = src.substr(src.find_first_not_of(" =", i));
	regEqStr = vars[res];
	return 1;
}

//Private submethod. Calculate number of equal signs in string
int EquationProc::EqualSignsNum(const std::string &src, int &equalSignPos) {
	int i = 0, count = 0;

	while (i < src.size()) {
		if (src.find('=', i) != std::string::npos) {
			++count;
			i = src.find('=', i) + 1;
		} else break;
	}
	equalSignPos = i;
	return count;
}

//Private method. It parse computational string
int EquationProc::ParseComputeStr(std::string &src, std::string &error) {
	std::set<std::string> baseFuncs = { "sin", "cos", "tan", "exp", "sqrt", "abs", "rad",
										"acos", "asin", "atan", "ceil", "floor", "cosh",
										"log", "logt", "tanh", "deg", "sinh" };
	std::string parse;
	for (int i = 0; i < src.size(); ++i) {
		if (std::isalpha(src[i])) {
			while (std::isalpha(src[i]))
				parse.push_back(src[i++]);
			if ((parse.size() == 1 && !parse.compare("i")) || (parse.size() == 2 && !parse.compare("pi")))
				{ parse.clear(); continue ; }
			if (src[i] == '(') {
				if (auto search = vars.find(parse); search != vars.end())
					return RetError(error, "\nerror: this is variable, not a function", i - 1, src);
				if (auto search = matricies.find(parse); search != matricies.end())
					return RetError(error, "\nerror: this is a matrix, not a function", i - 1, src);
				if (auto search = funcs.find(parse); search == funcs.end())
					return RetError(error, "\nerror: there is no such function", i - 1, src);
			} else {
				if (auto search = baseFuncs.find(parse); search != baseFuncs.end())
					return RetError(error, "\nerror: this is function, not a variable", i - 1, src);
				if (auto search = funcs.find(parse); search != funcs.end())
					return RetError(error, "\nerror: this is function, not a variable", i - 1, src);
				if (auto search = vars.find(parse); search == vars.end())
					return RetError(error, "\nerror: there is no such variable", i - 1, src);
				if (auto search = matricies.find(parse); search != matricies.end())
					isMatrix = true;
				else
					src.replace(src.find(parse), parse.size(), vars[parse]);
			}
			parse.clear();
		}
	}
	return 1;
}

//Private method. It generates error string and mark error
//position in source string
int EquationProc::RetError(std::string &error, const std::string &errMsg,
			 const int &i, const std::string &src) {
	error = src;
	error.push_back('\n');
	error.insert(error.size(), i, ' ');
	error.push_back('^');
	error += errMsg;
	return 0;
}

int EquationProc::RetExprError(const std::string &error) {
	if (!error.empty()) {
		history.push_back(error);
		std::cerr << history.back() << std::endl;
		return 0;
	}
	return 1;
}
//Expose radian function for plotting function
void EquationProc::PlotExposeRad(std::string &src) {
	//until there will be no 'rad' function in source
	//string, exchange it
	while (src.find("rad") != std::string::npos) {
		//parser index, position of equation beginning
		//and brace count
		int i = src.find("rad") + 4, brace = 1;
		int begin = i;
		//string for exchange
		std::string exch;

		while (brace) {
			if (src[i] == '(') ++brace;
			else if (src[i] == ')') --brace;
			++i;
		}
		exch = "pi * (" + src.substr(begin, i - begin - 1) + ") / 180";
		src.replace(src.find("rad"), i - src.find("rad"), exch);
	}
}

//Expose degree function for plotting function
void EquationProc::PlotExposeDeg(std::string &src) {
	//until there will be no 'deg' function in source
	//string, exchange it
	while (src.find("deg") != std::string::npos) {
		//parser index, position of equation beginning
		//and brace count
		int i = src.find("deg") + 4, brace = 1;
		int begin = i;
		//string for exchange
		std::string exch;

		while (brace) {
			if (src[i] == '(') ++brace;
			else if (src[i] == ')') --brace;
			++i;
		}
		exch = "180 * (" + src.substr(begin, i - begin - 1) + ") / pi";
		src.replace(src.find("deg"), i - src.find("deg"), exch);
	}
}

/*Private submethod. It calculate number of question marks in
source string. If number is greater than 1, we return 0.
If there is no queston mark, we stop scenario and return 3. In case of
one character we're parsing source string. If there is
some error we return 0, if question mark is right after
the equal sign we return 1, if question is after equal sign
and some piece of equation - return 2*/
int EquationProc::QuestionMarkPreview(std::string &src, std::string &error, std::string &token) {
	//Base functions for regular expressions
	std::set<std::string> baseFuncs = { "sin", "cos", "tan", "exp", "sqrt", "abs", "rad",
										"acos", "asin", "atan", "ceil", "floor", "cosh",
										"log", "logt", "tanh", "deg", "sinh" };
	//Base functions for matricies
	std::set<std::string> baseMatrixFuncs = { "inv", "transp", "lonenorm", "ltwonorm", "linfnorm",
											  "lpnorm", "det", "adj" };
	int count = 0, order = 0;
	std::string parse;
	//clear token
	token.clear();
	for (int i = 0; i < src.size(); ++i) {
		if (src[i] == '?') ++count;
		if (count > 1)
			return RetError(error, "\nerror: found some extra question mark", i, src);
	}
	if (!count)  return 3;
	for (int i = 0; i < src.size(); ++i) {
		//If we got some question mark and after if we get
		//some non-space character, return error
		if (order && !(order % 2) && !std::isspace(src[i]))
			return RetError(error, "\nerror: extra non-space character after question mark", i, src);
		//If it's alphabetical character, check if we deal with variable
		//or function
		if (std::isalpha(src[i])) {
			while (std::isalpha(src[i]))
				parse.push_back(src[i++]);
			if ((parse.size() == 1 && !parse.compare("i")) || (parse.size() == 2 && !parse.compare("pi")))
				{ parse.clear(); continue ; }
			if (src[i] == '(') {
				if (auto search = vars.find(parse); search != vars.end())
					return RetError(error, "\nerror: this is variable, not a function", i - 1, src);
				if (auto search = matricies.find(parse); search != matricies.end())
					return RetError(error, "\nerror: this is a matrix, not a function", i - 1, src);
				//If some matrix function is found, we deal with matricies
				if (auto search = baseMatrixFuncs.find(parse); search != baseMatrixFuncs.end())
					isMatrix = true;
				else if (auto search = baseFuncs.find(parse); search != baseFuncs.end()) {
					parse.clear(); continue ;
				}
				else if (auto search = funcs.find(parse); search == funcs.end())
					return RetError(error, "\nerror: there is no such function", i - 1, src);
			} else {
				if (auto search = baseFuncs.find(parse); search != baseFuncs.end())
					return RetError(error, "\nerror: this is function, not a variable", i - 1, src);
				if (auto search = baseMatrixFuncs.find(parse); search != baseMatrixFuncs.end())
					{ return RetError(error, "\nerror: this is function, not a variable", i - 1, src); }
				if (auto search = funcs.find(parse); search != funcs.end())
					return RetError(error, "\nerror: this is function, not a variable", i - 1, src);
				//If some matrix is found, we deal with matricies
				if (auto search = matricies.find(parse); search != matricies.end())
					isMatrix = true;
				else if (auto search = vars.find(parse); search == vars.end() && token.empty())
					{ token = parse; parse.clear(); continue; }
				else if (auto search = vars.find(parse); search == vars.end() && !token.empty() && parse != token)
					return RetError(error, "\nerror: there is no such variable", i, src);
				else if (auto search = vars.find(parse); search == vars.end() && token.empty())
					token = parse;
				else if (!token.empty() && parse == token) { parse.clear(); continue ; }
				else
					src.replace(src.find(parse), parse.size(), vars[parse]);
			}
			parse.clear();
		}
		//If it's equal sign
		else if (src[i] == '=') {
			++order;
			i = src.find_first_not_of(" =	", i);
			//Return 1, if question mark is right after equal sign
			if (src[i] == '?') order = 4;
			else --i;
		}
		//If question mark is before equal sign return error
		else if (src[i] == '?' && !order)
			return RetError(error, "\nerror: wrong place of question mark", i, src);
		//Else we increment order variable
		else if (src[i] == '?') ++order;
		//If character is left or right squarebrace, we deal with matrix
		else if (src[i] == ']' || src[i] == '[')
			isMatrix = true;
	}
	//If we deal with matricies and there is token, return error
	if (!token.empty() && isMatrix)
		{ error = "error: " + src + ": there can be no token in matrix equation"; return 0; }
	return (order == 4) ? 1 : 2;
}

//Private method. It parse string in terms of variable and function existence
int EquationProc::InitEquationParse(std::string &src, std::string &error,
									std::string &name, const std::string &oldValue) {
	std::string parse, errorPrefix;
	//Set of base functions for regular values
	std::set<std::string> baseFuncs = { "sin", "cos", "tan", "exp", "sqrt", "abs", "rad",
										"acos", "asin", "atan", "ceil", "floor", "cosh",
										"log", "logt", "tanh", "deg", "sinh" };
	//Set of base functions for matricies
	std::set<std::string> baseMatrixFuncs = { "inv", "transp", "lonenorm", "ltwonorm", "linfnorm",
											  "lpnorm", "det", "adj" };
	//Set error prefix, depends on if we deal with function or not
	errorPrefix = ((isFunc) ? name + "(" + funcs[name].token + ")" + " = " : name + " = ");

	for (int i = 0; i < src.size(); ++i) {
		if (std::isalpha(src[i])) {
			while (std::isalpha(src[i]))
				parse.push_back(src[i++]);
			if ((parse.size() == 1 && !parse.compare("i")) || (parse.size() == 2 && !parse.compare("pi")))
				{ parse.clear(); continue ; }
			if (src[i] == '(') {
				//errorPrefix =  name + "(" + funcs[name].token + ")" + " = ";
				if (isFunc && parse.size() == name.size() &&!parse.compare(name))
					return RetError(error, "\nerror: recursive function call is unexceptable!",
									i + errorPrefix.size() - 1, errorPrefix + src);
				if (auto search = vars.find(parse); search != vars.end())
					return RetError(error, "\nerror: this is variable, not a function: " + parse,
									i + errorPrefix.size() - 1, errorPrefix + src);
				if (auto search = matricies.find(parse); search != matricies.end())
					return RetError(error, "\nerror: this is a matrix, not a function: " + parse,
									i + errorPrefix.size() - 1, errorPrefix + src);
				if (auto search = baseFuncs.find(parse); search != baseFuncs.end())
					{ parse.clear(); continue ; }
				if (auto search = baseMatrixFuncs.find(parse); search != baseMatrixFuncs.end() && !isFunc)
					{ isMatrix = true; parse.clear(); continue ; }
				if (auto search = funcs.find(parse); search == funcs.end())
					return RetError(error, "\nerror: there is no such function: " + parse,
									i + errorPrefix.size() - 1, errorPrefix + src);
				if (auto search = baseMatrixFuncs.find(parse); search != baseMatrixFuncs.end() && isFunc)
					return RetError(error, "\nerror: " + parse + ": matrix function in function equation is not supported:",
									i + errorPrefix.size() - 1, errorPrefix + src);
			}
			else {
				//errorPrefix = name + " = ";
				if (isFunc && funcs[name].token.size() == parse.size() && !funcs[name].token.compare(parse))
					{ parse.clear(); continue; }
				if (auto search = funcs.find(parse); search != funcs.end())
					return RetError(error, "\nerror: this is a function, not a variable: " + parse,
									i + errorPrefix.size() - 1, errorPrefix + src);
				if (auto search = baseFuncs.find(parse); search != baseFuncs.end())
					return RetError(error, "\nerror: this is a function, not a variable: " + parse,
									i + errorPrefix.size() - 1, errorPrefix + src);
				if (!isFunc && parse.size() == name.size() && !parse.compare(name) && !oldValue.empty())
					src.replace(src.find(parse), parse.size(), oldValue);
				else if (!isFunc && parse.size() == name.size() && !parse.compare(name) && oldValue.empty())
					return RetError(error, "\nerror: there is no previous declaration of '" + name + "' variable",
									i + errorPrefix.size() - 1, errorPrefix + src);
				if (auto search = matricies.find(parse); search != matricies.end() && isFunc)
					return RetError(error, "\nerror: " + parse + ": matrix variable in function equation is not supported:",
									i + errorPrefix.size() - 1, errorPrefix + src);
				if (auto search = matricies.find(parse); search != matricies.end())
					isMatrix = true;
				else if (auto search = vars.find(parse); search != vars.end())
					src.replace(src.find(parse), parse.size(), vars[parse]);
				else
					return RetError(error, "\nerror: there is no such variable: " + parse,
									i + errorPrefix.size() - 1, errorPrefix + src);
			}
			parse.clear();
			//If character is left or right squarebrace, we deal with matrix
			if (src[i] == ']' || src[i] == '[')
				isMatrix = true;
			//If we deal with matricies and functions in the same time
			//return error
			if (isFunc && (src[i] == '[' || src[i] == ']')) {
				std::string braceToken; braceToken.push_back(src[i]);
				return RetError(error, "\nerror: " + braceToken
								+ ": matrix variable in function equation is not supported:",
								i + errorPrefix.size(), errorPrefix + src);
			}
		}
	}
	return 1;
}

//Output operation history into the output stream
void EquationProc::HistoryOutput() {
	std::cout << std::endl;
	for (std::string var : history)
		std::cout << var << std::endl;
	std::cout << std::endl;
}

//Output list of all available variables
void EquationProc::VariablesOutput() {
	//Variables iterator
	std::map<std::string, std::string>::iterator it = vars.begin();
	//Matricies iterator
	std::map<std::string, Matrix>::iterator itMap = matricies.begin();

	std::cout << std::endl;
	//Regular variables output
	std::cout << "Variables:" << std::endl;
	for ( ; it != vars.end(); ++it)
		std::cout << it->first << " = " << it->second << std::endl;
	//Matricies output
	std::cout << "Matricies:" << std::endl;
	for ( ; itMap != matricies.end(); ++itMap)
		std::cout << itMap->first << ':' << std::endl << itMap->second.toString();
	std::cout << std::endl;
}

//Plotting function
void EquationProc::PlotFunction(const std::string &src) {
	 	//Function string for further plotting
        std::string toPlot = src;
        //index iterator and parser
        int parseIt;
        /*place of the character after left
        sbrace and place of the right brace*/
        int begin, end;
        //number of braces due parsing
        int braceNum = 0;
		//Expression calculator
		RevPolNotation pol(funcs);
		//error string message
		std::string error;

		history.push_back(toPlot);
		history.back().insert(0, ">> ");
        parseIt = toPlot.find("plot") + 4;
        //Check is there is left squarebrace
        if (toPlot[parseIt] != '(') {
                std::cerr << "error: there is no left brace" << std::endl;
                return ;
        }
        begin = ++parseIt;
        ++braceNum;
        //Check is there right squarebrace, and there is none
        //return error
        for ( ; parseIt < toPlot.size(); ++parseIt) {
                if (toPlot[parseIt] == '(') ++braceNum;
                if (toPlot[parseIt] == ')') --braceNum;
                if (!braceNum) break ;
        }
        if (braceNum) {
                std::cerr << "error: there is no right square brace for plot function"
                        << std::endl;
                return ;
        }
        end = parseIt;
        /*Check if there is no obscure character
        after the right squarebrace, and if there
        is, return error*/
        while (std::isspace(toPlot[++parseIt])) ;
        if (parseIt != toPlot.size()) {
                std::cerr << "error: there is some obscure chararcter after right squarebrace"
                        << std::endl;
                return ;
        }
		//Expression string
		toPlot = toPlot.substr(begin, end - begin);
		//Set plot function
		std::string funcName = "plot";
		funcs["plot"].equation = toPlot;
		funcs["plot"].token = "x";
		isFunc = true;
		//Preview of equation before plotting
		if (!InitEquationParse(toPlot, error, funcName, "")) {
			history.push_back(error);
			std::cerr << history.back() << std::endl;
			return;
		}
		//If everything is allright, let's calculate expression
		//In case of error, return it
		pol.setToken(funcs["plot"].token);
		pol.setInfixExpr(std::move(toPlot));
		if (!pol.getErrMsg().empty()) {
			history.push_back(pol.getErrMsg());
			std::cerr << history.back() << std::endl;
			return ;
		}
		toPlot = pol.CalcIt();
		if (!pol.getErrMsg().empty()) {
			history.push_back(pol.getErrMsg());
			std::cerr << history.back() << std::endl;
			return ;
		}
		//After parsing and calculating expression
		//we should delete temporary 'plot' function
		funcs.erase("plot");
		//Change all power character '^' to their
        //gnuplot alternative - '**'
        while (toPlot.find("^") != std::string::npos) {
                //'^' character position
                int i = toPlot.find("^");
                toPlot.replace(i, 1, "**");
        }
        //Exchange 'rad' function in function
        //string, that should be plotted
        PlotExposeRad(toPlot);
		PlotExposeDeg(toPlot);
        //plot fixed string
        gp << "plot " << toPlot << std::endl;
}

//Push to history
void EquationProc::PushToHistory(const std::string &src) {
	//Result string
	std::string res = src;

	res.insert(0, ">> ");
	history.push_back(res);
}
