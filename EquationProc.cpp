#include "EquationProc.h"
#include "computor.h"
#include <iostream>

EquationProc::EquationProc() { }

//Add equation: either function or variable, or some equation.
void EquationProc::AddEquation(std::string &&equation) {
	int				equalSignPos, state, questMark;
	std::string		sample, errMsg, curToken, entityName, oldValue;
	RevPolNotation	parser(funcs);
	//We don't know, if we deal with matrix or not, thus set signal
	//as false
	isMatrix = false;
	//Lambda function. it returns position if there is some base function
	//else it returns -1
	auto ifBaseFuncIn = [](const std::string &src, std::string &error) {
		std::set<std::string> baseFuncs = {"sin", "cos", "tan", "exp", "sqrt", "abs"};
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
	sample = history.back().substr(history.back().find_first_not_of("	> "));
	if (EqualSignsNum(sample, equalSignPos) != 1) {
		history.push_back("error: equation must have only one equal sign");
		return ;
	}
	//Check if there is some question mark
	questMark = QuestionMarkPreview(sample, errMsg, curToken);
	//If there is some error, push it to history and exit
	if (!questMark) {
		history.push_back(errMsg);
		return ;
	}
	//If we deal with simple computational part
	if (questMark == 1) {
		equalSignPos = sample.find('=') - 1;
		while (std::isspace(sample[equalSignPos])) --equalSignPos;
		sample = sample.substr(0, equalSignPos + 1);
		parser.setInfixExpr(std::move(sample));
		history.push_back(parser.CalcIt());
		return ;
	}
	//If we deal with complex computational part
	if (questMark == 2) {
		//Result parsed equation
		//std::string res, aux;
		int delim = sample.find('=');
		while (sample[delim] == '=' || std::isspace(sample[delim])) --delim;
		//Left part of source equation
		std::string leftEq = sample.substr(0, delim + 1);
		//Right part of equation without question mark
		std::string rightEq = sample.substr(sample.find_first_not_of(" = 	", sample.find('=')));
		delim = rightEq.size();
		while (rightEq[delim - 1] == '?' || std::isspace(rightEq[delim - 1])) --delim;
		rightEq.resize(delim);
		//Parse left and right equations
		parser.setToken(curToken);
		parser.setInfixExpr(std::move(leftEq));
		leftEq = parser.CalcIt();
		//if (ifBaseFuncIn(leftEq, errMsg)) { history.push_back(errMsg); return ; }
		parser.setInfixExpr(std::move(rightEq));
		rightEq = parser.CalcIt();
		leftEq.append(" = ");
		leftEq.append(rightEq);
		//Check if equation has some base function
		if ((delim = ifBaseFuncIn(leftEq, errMsg)) != -1) {
			RetError(errMsg, std::string(errMsg), delim, leftEq);
			history.push_back(errMsg);
			return;
		}
		//let's solve equation
 		computor solve(leftEq, curToken);
		history.push_back(solve.to_string());
		return ;
	}
	//Parsing variable or function
	state = EntityDefine(errMsg, sample, entityName, oldValue);
	//if error occured
	if (!state) {
		history.push_back(errMsg);
		return ;
	}
	//Variable case calculation
	else if (state == 1) {
		sample = vars[entityName];
		if (!InitEquationParse(sample, errMsg, 0, entityName, oldValue)) {
			history.push_back(errMsg);
			return ;
		}
		parser.setInfixExpr(std::move(sample));
		vars[entityName] = parser.CalcIt();
		history.push_back(vars[entityName]);
	}
	//Function case calculation
	else {
		parser.setToken(funcs[entityName].token);
		sample = funcs[entityName].equation;
		if (!InitEquationParse(sample, errMsg, 1, entityName, oldValue)) {
			history.push_back(errMsg);
			return ;
		}
		parser.setInfixExpr(std::move(sample));
		funcs[entityName].equation = parser.CalcIt();
		if (funcs[entityName].equation.empty() && !parser.getErrMsg().empty()) {
			funcs.erase(entityName);
			history.push_back(parser.getErrMsg());
		}
		else history.push_back(funcs[entityName].equation);
	}
}

//Method to define string represents a variable or a function
//it returns integer value represents next states: 1 - we're dealing with variable
//2 - we're dealing with function; 0 - error
int EquationProc::EntityDefine(std::string &error, const std::string &src,
							   std::string &res, std::string &oldValue) {
	int i = 0, isfunc = 0;
	Func func;
	//Equation string
	std::string eq;
	//Seeking name of variables (function) and name of token
	//passing whitespaces
	auto indx = [](const std::string &src, std::string &dst, int i) {
		while (std::isspace(src[i])) ++i;
		for ( ; i < src.size(); ++i) {
			if (!std::isalpha(src[i])) break ;
			dst.push_back(src[i]);
		}
		while (std::isspace(src[i])) ++i;
		return i;
	};
	/*auto RetError = [](std::string &error, const std::string &errMsg,
			const int &i, const std::string &src) {
		error = src;
		error.push_back('\n');
		error.insert(error.size(), i - 1, ' ');
		error.push_back('^');
		error += errMsg;
		return 0;
	};*/


	i = indx(src, res, i);
	if (src[i] == '(') {
		isfunc = 1;
		funcs[res] = func;//func.name = std::move(res);
		//func = src.substr(0,  i++);
		i = indx(src, funcs[res].token, i + 1);
		if (src[i++] != ')') {
			funcs.erase(res);
			return RetError(error, "\nerror: function token must consist of only alphabetical characters",
							i, src);
		}
		else if (funcs[res].token.empty()/*func.token.empty()*/) {
			funcs.erase(res);
			return RetError(error, "\nerror: token can't be empty", i - 1, src);
		}
	}
	i = src.find_first_not_of( " 	", i);
	if (src[i] != '=') {
		if (isfunc) funcs.erase(res);
		return RetError(error, "\nerror: equal sign is not in proper place", i, src);
	}
	//Assigning equation string
	eq = src.substr(src.find_first_not_of(" =", i));
	//Search matrix, or squarebraces in  in function or variable
	//If there is, set isMatrix variable as true
	isMatrix = true;
	for (std::map<std::string, Matrix>::iterator it = matricies.begin(); it != matricies.end(); ++it) {
		if (auto search = eq.find(it->first); search != std::string::npos) {
			isMatrix = true; break;
		}
	}
	if (eq.find('[') != std::string::npos || eq.find(']') != std::string::npos)
		isMatrix = true;
	//If we deal with function
	if (isfunc) {
		funcs[res].equation = src.substr(src.find_first_not_of(" =", i));
		funcs[res].isMatrix = isMatrix;
		return 2;
	}
	//If we deal with matricies
	if (isMatrix && !isfunc) {
		//If there is some old matrix value, fix it
		if (auto search = matricies.find(res); search != matricies.end())
			oldValue = matricies[res].getMatrix();

	}
	//If we deal with variables
	if (auto search = vars.find(res); search != vars.end())
		oldValue = vars[res];
	vars[res] = src.substr(src.find_first_not_of(" =", i));
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
	std::set<std::string> baseFuncs = { "sin", "cos", "tan", "exp", "sqrt", "abs" };
	std::string parse;
	for (int i = 0; i < src.size(); ++i) {
		if (std::isalpha(src[i])) {
			while (std::isalpha(src[i]))
				parse.push_back(src[i++]);
			if ((parse.size() == 1 && !parse.compare("i")) || (parse.size() == 2 && !parse.compare("pi")))
				{ parse.clear(); continue ; }
			if (src[i] == '(') {
				if (auto search = vars.find(parse); search != vars.end())
					return RetError(error, "\nerror: this is variable, not a function", i, src);
				if (auto search = matricies.find(parse); search != matricies.end())
					return RetError(error, "\nerror: this is a matrix, not a function", i, src);
				if (auto search = funcs.find(parse); search == funcs.end())
					return RetError(error, "\nerror: there is no such function", i, src);
			} else {
				if (auto search = baseFuncs.find(parse); search != baseFuncs.end())
					return RetError(error, "\nerror: this is function, not a variable", i, src);
				if (auto search = funcs.find(parse); search != funcs.end())
					return RetError(error, "\nerror: this is function, not a variable", i, src);
				if (auto search = vars.find(parse); search == vars.end())
					return RetError(error, "\nerror: there is no such variable", i, src);
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
	error.insert(error.size(), i - 1, ' ');
	error.push_back('^');
	error += errMsg;
	return 0;
}

/*Private submethod. It calculate number of question marks in
source string. If number is greater than 1, we return 0.
If there is no queston mark, we stop scenario and return 3. In case of
one character we're parsing source string. If there is
some error we return 0, if question mark is right after
the equal sign we return 1, if question is after equal sign
and some piece of equation - return 2*/
int EquationProc::QuestionMarkPreview(std::string &src, std::string &error, std::string &token) {
	std::set<std::string> baseFuncs = { "sin", "cos", "tan", "exp", "sqrt", "abs" };
	int count = 0, order = 0;
	std::string parse;

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
					return RetError(error, "\nerror: this is variable, not a function", i, src);
				if (auto search = matricies.find(parse); search != matricies.end())
					return RetError(error, "\nerror: this is a matrix, not a function", i, src);
				if (auto search = funcs.find(parse); search == funcs.end())
					return RetError(error, "\nerror: there is no such function", i, src);
			} else {
				if (auto search = baseFuncs.find(parse); search != baseFuncs.end())
					return RetError(error, "\nerror: this is function, not a variable", i, src);
				if (auto search = funcs.find(parse); search != funcs.end())
					return RetError(error, "\nerror: this is function, not a variable", i, src);
				if (auto search = vars.find(parse); search == vars.end() && !token.empty())
					return RetError(error, "\nerror: there is no such variable", i, src);
				else if (auto search = vars.find(parse); search == vars.end() && token.empty())
					token = parse;
				else if (auto search = matricies.find(parse); search != matricies.end())
					isMatrix = true;
				else
					src.replace(src.find(parse), parse.size(), vars[parse]);
			}
			parse.clear();
			//If character is left or right squarebrace, we deal with matrix
			if (src[i] == ']' || src[i] == '[')
				isMatrix = true;
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
	}
	return (order == 4) ? 1 : 2;
}

//Private method. It parse string in terms of variable and function existence
int EquationProc::InitEquationParse(std::string &src, std::string &error,
									int isfunc, std::string &name, const std::string &oldValue) {
	std::string parse, errorPrefix;
	std::set<std::string> baseFuncs = { "sin", "cos", "tan", "exp", "sqrt", "abs" };

	for (int i = 0; i < src.size(); ++i) {
		if (std::isalpha(src[i])) {
			while (std::isalpha(src[i]))
				parse.push_back(src[i++]);
			if ((parse.size() == 1 && !parse.compare("i")) || (parse.size() == 2 && !parse.compare("pi")))
				{ parse.clear(); continue ; }
			if (src[i] == '(') {
				errorPrefix =  name + "(" + funcs[name].token + ")" + " = ";
				if (isfunc && parse.size() == name.size() &&!parse.compare(name))
					return RetError(error, "\nerror: recursive function call is unexceptable!",
									i + errorPrefix.size(), errorPrefix + src);
				if (auto search = vars.find(parse); search != vars.end())
					return RetError(error, "\nerror: this is variable, not a function: " + parse,
									i + errorPrefix.size(), errorPrefix + src);
				if (auto search = matricies.find(parse); search != matricies.end())
					return RetError(error, "\nerror: this is a matrix, not a function", i, src);
				if (auto search = baseFuncs.find(parse); search != baseFuncs.end())
					{ parse.clear(); continue ; }
				if (auto search = funcs.find(parse); search == funcs.end())
					return RetError(error, "\nerror: there is no such function: " + parse,
									i + errorPrefix.size(), errorPrefix + src);
			}
			else {
				errorPrefix = name + " = ";
				if (isfunc && funcs[name].token.size() == parse.size() && !funcs[name].token.compare(parse))
					{ parse.clear(); continue; }
				if (auto search = funcs.find(parse); search != funcs.end())
					return RetError(error, "\nerror: this is a function, not a variable: " + parse,
									i + errorPrefix.size(), errorPrefix + src);
				if (auto search = baseFuncs.find(parse); search != baseFuncs.end())
					return RetError(error, "\nerror: this is a function, not a variable: " + parse,
									i + errorPrefix.size(), errorPrefix + src);
				if (!isfunc && parse.size() == name.size() && !parse.compare(name) && !oldValue.empty())
					src.replace(src.find(parse), parse.size(), oldValue);
				else if (!isfunc && parse.size() == name.size() && !parse.compare(name) && oldValue.empty())
					return RetError(error, "\nerror: there is no previous declaration of '" + name + "' variable",
									i + errorPrefix.size(), errorPrefix + src);
				if (auto search = matricies.find(parse); search != matricies.end())
					isMatrix = true;
				else if (auto search = vars.find(parse); search != vars.end())
					src.replace(src.find(parse), parse.size(), vars[parse]);
				else
					return RetError(error, "\nerror: there is no such variable: " + parse,
									i + errorPrefix.size(), errorPrefix + src);
			}
			parse.clear();
			//If character is left or right squarebrace, we deal with matrix
			if (src[i] == ']' || src[i] == '[')
				isMatrix = true;
		}
	}
	return 1;
}

//Output operation history into the output stream
void EquationProc::HistoryOutput() {
	for (std::string var : history)
		std::cout << var << std::endl;
}
//Output list of all available variables
void EquationProc::VariablesOutput() {
	std::map<std::string, std::string>::iterator it = vars.begin();

	for ( ; it != vars.end(); ++it) {
		std::cout << it->first << " = " << it->second << std::endl;
	}
}