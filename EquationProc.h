#ifndef EQUATIONPROC_H
# define EQUATIONPROC_H
#include "RevPolNotation.h"
#include "Matrix.h"
#include <map>

class EquationProc {
	//History of all operations
	std::list<std::string>	history;
	//Set of all variables represented
	std::map<std::string, std::string>	vars;
	//Set of all defined functions
	std::map<std::string, Func>	funcs;
	//Set of all defined matricies
	std::map<std::string, Matrix> matricies;
	//Is computational part of equation matrix
	bool isMatrix;

	//Method to define string represents a variable or a function
	int EntityDefine(std::string &error, const std::string &src,
					 std::string &res, std::string &oldValue);
	//Private submethod. Calculate number of equal signs in string
	int EqualSignsNum(const std::string &src, int &equalSignPos);
	/*Private submethod. It calculate number of question marks in
	source string. If number is greater than 1, we return 0.
	If there is no queston mark, we stop scenario and return 3. In case of
	one character we're parsing source string. If there is
	some error we return 0, if question mark is right after
	the equal sign we return 1, if question is after equal sign
	and some piece of equation - return 2*/
	int QuestionMarkPreview(std::string &src, std::string &error, std::string &token);
	//Private method. It parse string in terms of variable and function existence
	int InitEquationParse(std::string &src, std::string &error,
						  int isfunc, std::string &name, const std::string &oldValue);
	//Private method. It parse computational string
	int ParseComputeStr(std::string &src, std::string &error);
	//Private method. It generates error string and mark error
	//position in source string
	int RetError(std::string &error, const std::string &errMsg,
				 const int &i, const std::string &src);
public:
	EquationProc();
	//EquationProc(std::string &&equation);
	//Add equation: either function or variable, or some equation.
	void AddEquation(std::string &&equation);
	//Output operation history into the output stream
	void HistoryOutput();
	//Output list of all available variables
	void VariablesOutput();
};

#endif