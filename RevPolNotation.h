#ifndef REVPOLNOTATION_H
# define REVPOLNOTATION_H
# include <string>
# include <map>
# include <list>
# include <set>

//struct that represents next function attributes:
//function token and equation
typedef struct sFunc {
	std::string token;
	std::string equation;
	//Does function have any matricies. If yes, isMatrix is true
	bool isMatrix;
	//Is token a matrix 0 - not defined, 1 - matrix,
	//2 - regular variable
	int tokenIsMatrix;

} Func;

class RevPolNotation {
	std::string					inifixExpr;
	std::string					postfixExpr;
	std::map<std::string, int>	operPriority;
	bool						calcError;
	//variable name in case of function declaration
	std::string					varName;
	//set of functions. In initial state has some base functions
	std::set<std::string>		funcs;
	const std::string			baseOpers;
	std::string					token;
	//Error message string
	std::string					errmsg;
	//Map of user defined functions. A reference value
	std::map<std::string, Func> &userDefFuncs;

	//Get double number from string
	std::string	GetStringNumber(const std::string &str, int &i);
	//Private submethod. Invoking prefix number for alpha postfix
	std::string prefixNum(const std::string &num, const std::string &oper);
	//Private submethod.  Invoking 1 or -1 in case of some basic operation
	std::string simplePrefixNum(const std::string &num);
	//String parser to get denominator.
	//It returns list of summed and subtracked elements.
	std::list<std::string> DenomElems(const std::string &src);
	//Complex sum and subtraction of string and number
	void ComplexSumSubtractNum(std::list<std::string> &elems, const std::string &num,
							   const std::string &oper);
	//Submethod. Searching for multiplying parts and corrects them
	void StrMultiplySearch(std::string &src, const std::string &token);
	//Complex multiply or division of string and number
	void ComplexMultiDivNum(std::list<std::string> &elems, const std::string &num,
							const std::string &oper);
	//Private submethod. With this submethod, we're processing operands 'f' and 's'
	//in case, when f string has some alphabetical character and s - pure number
	std::string AlphaNum(const std::string &oper, const std::string &f, const std::string &s);
	//Private submethod. With this submethod, we're processing operands 'f' and 's'
	//in case, when f - pure number and s has alphabetical character
	std::string NumAlpha(const std::string &oper, const std::string &f, const std::string &s);
	//Private method. Number bracketing for complex equations
	std::string NumBracketing(const std::list<std::string> &lst);
	//Private method. It return list with alphanum part and pow part. In case of complex
	//expression it returns
	std::list<std::string> PowLevel(const std::string &src);
	//Private submethod. It manages division of both alphanumerical variables
	std::list<std::string> ComplexDivisionAlpha(const std::string &oper,
												const std::list<std::string> &fLst,
												const std::list<std::string> &sLst);
	//Private submethod. It manages multiplication of both alphanumerical variables
	std::list<std::string> ComplexMultiplicAlpha(const std::string &oper,
												 const std::list<std::string> &fLst,
												 const std::list<std::string> &sLst);
	//Private method. It manages multiplication and division of both alphanumerical variables
	std::list<std::string> ComplexMultiDivAlpha(const std::string &oper,
												const std::list<std::string> &fLst,
												const std::list<std::string> &sLst);
	//Private submethod. Form final string for AlphaAlpha equation
	std::string AlphaFinalSumSub(const std::list<std::string> &lst);
	//Private submethod. It manages subtract and summing elements
	std::list<std::string> ComplexSubSumAlpha(const std::string &oper,
											  const std::list<std::string> &fLst,
											  const std::list<std::string> &sLst);
	//Private submethod. It manages case with both alphanumerical variables
	std::string AlphaAlpha(const std::string &oper, const std::string &f, const std::string &s);
	//Private method. It executes base operators
	std::string Execute(const std::string &oper, const std::string &f, const std::string &s);
	//Private method. It returns exposed func string, if func's
	//operand contains alphabetical character
	std::string UDfuncExpose(const Func &src, const std::string &forReplace);
	//Private method. It executes base and user defined functions
	std::string funcExecute(const std::string &oper, const std::string &var);
	//Making a postfix string for further calculations
	std::string	ProcessPostfix();
	//Removing trailing zeros in string
	std::string RemoveTrailZeros(const std::string &str);
	//Converting degrees to radians
	long double ToRadians(const long double &val);
	//Convert radians to degrees
	long double ToDegrees(const long double &val);
	//Sorting function conatins
	std::string ElemsSort(const std::string &src);
public:
	RevPolNotation() = delete;
	RevPolNotation(std::map<std::string, Func> &userDefFuncsRef);
	RevPolNotation(std::string &&init_expr, std::map<std::string, Func> &userDefFuncsRef);
	//Get infix notation
	const std::string	&getInifixExpr();
	//Get postfix notation
	const std::string	&getPosfixExpr();

	//Set new infix expression for further calculations
	void				setInfixExpr(std::string &&move);
	//Calculation of postfix equation
	std::string CalcIt();
	//Get string token value
	const std::string	&getToken();
	//Set new value for string 'token'
	void				setToken(const std::string &newToken);
	//Get error message
	const std::string	&getErrMsg() const;
};

#endif
