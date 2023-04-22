#include "computor.h"
#include <cctype>
#include <cstring>
#include <utility>
#include <cmath>

//Parametrized constructor. The parameters are
//source string with equation and it's token
computor::computor(const std::string &src, const std::string &eqToken) : degree(0),
		polys(new std::vector<double>(3)), mirror(1),
		to_pass(src), res(std::string()), token(eqToken),
		errMsg(std::string()) {
	if (eqToken.empty()) {
		res.append("error: token is not defined");
		return ;
	}
	if (to_pass.empty())
		return ;
	it = to_pass.begin();
	degree = 2;
	parse();
	if (!errMsg.empty()) return ;
	res.append("Polynomial degree: ");
	res.append(std::to_string(degree));
	res.push_back('\n');
	if (!degree) cheq();
	if (degree == 1) solution1d();
	if (degree == 2) solution2d();
	if (degree > 2)
		res.append("The polynomial degree is strictly greater than 2, I can't solve");
}

//Passing whitespaces while passing source string
void computor::pass() {
	while (it != to_pass.end() && isblank(*it)) ++it;
}

//Removing trailing zeros in string
//represents number
void del_trailer(std::string &src) {
	int end;

	end = src.size() - 1;
	while (src[end] == '0' || src[end] == '.') --end;
	src.resize(end + 1);
}

//Returns syntax error if it occurs
void computor::syntax_err() {
	unsigned long pos;

	pos = it - to_pass.begin();
	errMsg.append(to_pass + "\n");
	for (unsigned long i = 0; i < pos; ++i)
		errMsg.push_back(' ');
	errMsg.append("^\n");
	errMsg.append("syntax error");
}

//Get double value by parsing iterator
double computor::get_number() {
	double rain;
	bool point;
	std::string number;

	point = false;
	number = std::string();
	rain = 0;
	while (it != to_pass.end() && ((std::isdigit(*it)
									|| (*it == '.' && point == false)))) {
		if (*it == '.' && !point) point = true;
		number.push_back(*it);
		++it;
	}
	if (!number.empty()) rain = std::stod(number);
	return rain;
}

//Get integer number of pow level. If number with a floating point
//it returns error
int computor::get_int_number() {
	int rain;
	std::string number;

	pass();
	number = std::string();
	rain = 0;
	while (it != to_pass.end() && std::isdigit(*it)) {
		if (*it == '.') syntax_err();
		number.push_back(*it);
		++it;
	}
	if (!number.empty()) rain = std::stoi(number);
	return rain;
}

//Removing trailing zeros in string
//represents number
void computor::del_trailer() {
	int size;

	size = res.size() - 1;
	while (res[size] == '0' || res[size] == '.')
		if (res[size--] == '.') break ;
	res.resize(size + 1);
}

//Parsing source string, seeking errors and getting
//corresponding polynominal values
void computor::fract() {
	unsigned long aux_degree;
	double arg;
	int multi;
	bool if_arg;
	int pos;

	pass();
	multi = 1;
	if_arg = true;
	if (it == to_pass.end()) return ;
	if (*it == '=') {
		mirror = -1;
		++it;
	}
	pass();
	if (*it == '-') {
		multi = -1;
		++it;
	}
	else if (*it == '+') ++it;
	pass();
	if (!std::isdigit(*it) && *it != '.') {
		arg = 1.0;
		if_arg = false;
	}
	else arg = get_number();
	pass();
	if (*it == '+' || *it == '-') {
		polys->operator[](0) += arg * mirror * multi;
		fract();
		return ;
	}
	if (*it == '=') {
		polys->operator[](0) += arg * mirror * multi;
		fract();
		return ;
	}
	if (*it && *it != '*' && if_arg) syntax_err();
	++it;
	if (!*it || !errMsg.empty()) return ;
	if (!if_arg) --it;
	pass();
	pos = it - to_pass.begin();
	if (*it && to_pass.compare(pos, token.size(), token)/**(it++) != 'X'*/)
		syntax_err();
	++it;
	if (!*it || !errMsg.empty()) return ;
	pass();
	//aux_degree = (*(it++) == '^') ? get_int_number() : 1;
	if (*it == '^') { ++it; aux_degree = get_int_number();}
	else aux_degree = 1;
	if (aux_degree > polys->size() - 1) {
		polys->resize(aux_degree + 1);
		degree = aux_degree;
	}
	polys->operator[](aux_degree) += arg * mirror * multi;
	fract();
}

//Parse source and creating a new for result output
void computor::parse() {
	fract();
	if (!errMsg.empty()) { res = errMsg; return; }
	int final_degree = degree;

	res.append("Reduced form: ");
	for (int i = degree; i > -1; --i) {
		if (polys->operator[](i)) {
			if (i != final_degree) {
				res.push_back((polys->operator[](i) < 0) ? '-' : '+');
				res.push_back(' ');
			}
			if (i == final_degree && polys->operator[](i) < 0)
				res.push_back('-');
			//if multiplyer of polynom is 1
			if ((polys->operator[](i) == 1 || polys->operator[](i) == -1) && i > 0) {
				res.append(token + "^");
				res.append(std::to_string(i));
				del_trailer();
				res.push_back(' ');
			}
			else {
				res.append((i == degree && polys->operator[](i) > 0)
						   ? std::to_string(polys->operator[](i))
						   : std::to_string(abs(polys->operator[](i))));
				del_trailer();
				res.push_back(' ');
				if (i > 0) {
					res.append("* " + token + "^");
					res.append(std::to_string(i));
					del_trailer();
					res.push_back(' ');
				}
			}
		}
		if (!polys->operator[](final_degree) && final_degree) --final_degree;
	}
	res.append("= 0\n");
	degree = final_degree;
}

//Adding solution to result string
//in case of zero discriminant
void computor::solution1d() {
	double sol;

	sol = -1.0 * polys->operator[](0) / polys->operator[](1);
	res.append("The solution is:\n");
	res.append(std::to_string(sol));
	del_trailer();
}

//Adding solution to result string
//in case of non-zero discriminant
void computor::solution2d() {
	double dis;

	dis = polys->operator[](1) * polys->operator[](1)
		  - 4 * polys->operator[](2) * polys->operator[](0);
	if (dis >= 0) posdis(dis);
	else negdis(dis);
}

//Positive discriminant scenario
void computor::posdis(double dis) {
	double sol[2] = {0};

	sol[0] = (-1.0 * polys->operator[](1) + sqrt(dis)) /
			 (2 * polys->operator[](2));
	sol[1] = (-1.0 * polys->operator[](1) - sqrt(dis)) /
			 (2 * polys->operator[](2));
	res.append("The solution is:\n");
	res.append(std::to_string(sol[1]));
	del_trailer();
	if (dis) {
		res.push_back('\n');
		res.append(std::to_string(sol[0]));
		del_trailer();
	}
}

//Negative discriminant scenario
void computor::negdis(double dis) {
	double sol[2] = {0};

	sol[0] = (-1.0 * polys->operator[](1)) / (2 * polys->operator[](2));
	sol[1] = sqrt(abs(dis)) / (2 * polys->operator[](2));
	res.append("The solution is:\n");
	res.append(std::to_string(sol[0]));
	del_trailer();
	res.append(" + ");
	res.append(std::to_string(sol[1]));
	del_trailer();
	res.append(" * i\n");
	res.append(std::to_string(sol[0]));
	del_trailer();
	res.append(" - ");
	res.append(std::to_string(sol[1]));
	del_trailer();
	res.append(" * i");
}

void computor::cheq() {
	if (polys->operator[](0)) {
		res.append(std::to_string(polys->operator[](0)));
		del_trailer();
		res.append(" is not equal to 0");
	} else
		res.append("0 is equal to 0, of course. Yep!");
}

//Return result as string
const std::string& computor::to_string() {
	return res;
}

//Destructor
computor::~computor() {
	delete polys;
}