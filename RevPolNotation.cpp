#include "RevPolNotation.h"
#include <set>
#include <cmath>
#include <vector>
#include <stack>
#include <sstream>
#include <iomanip>
#include <functional>

//Get double number from string
std::string			RevPolNotation::GetStringNumber(const std::string &str, int &i) {
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

std::string RevPolNotation::prefixNum(const std::string &num, const std::string &oper) {
	std::string res = num;

	if ((res.size() == 1 && !res.compare("1")) || (res.size() == 2 && !res.compare("-1")))
		res.pop_back();
	else res += oper;
	return res;
}

std::string RevPolNotation::simplePrefixNum(const std::string &num) {
	std::string res = num;

	if (res.empty() || (res.size() == 1 && !res.compare("-"))) res.push_back('1');
	return res;
}

std::list<std::string> RevPolNotation::DenomElems(const std::string &src) {
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

void RevPolNotation::ComplexSumSubtractNum(std::list<std::string> &elems, const std::string &num,
						   const std::string &oper) {
	std::list<std::string>::iterator it = elems.begin();
	std::string finalNum = "0";

	//if (!num.compare("0") && num.size() == 1)
	//	return ;
	while (it != elems.end()) {
		if ((*it).find_first_not_of("-0123456789.") == std::string::npos) {
			if (!(*it).compare("-") && (*it).size() == 1) { ++it; continue; }
			if (it != elems.begin()) {
				--it;
				std::string auxOper = *it;
				it = elems.erase(it);
				finalNum = RemoveTrailZeros(Execute(auxOper, finalNum, *it));
				elems.erase(it);
				it = elems.begin();
				continue ;
			}
			finalNum = *it;
		}
		++it;
	}
	finalNum = RemoveTrailZeros(Execute(oper, finalNum, num));
	if (finalNum != "0") {
		if (finalNum.front() == '-') {
			elems.push_back("-");
			finalNum.erase(finalNum.begin());
		} else elems.push_back("+");
		elems.push_back(finalNum);
	}
}

void RevPolNotation::StrMultiplySearch(std::string &src, const std::string &token) {
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

void RevPolNotation::ComplexMultiDivNum(std::list<std::string> &elems, const std::string &num,
						const std::string &oper) {
	std::list<std::string>::iterator it = elems.begin();

	for (; it != elems.end(); ++it) {
		if ((*it).compare("+") && (*it).compare("-")) {
			std::string strNum, strAlpha;
			std::list<std::string> tmpList;
			std::string strPart;

			if ((*it).find_first_not_of("-0123456789.") != std::string::npos)
				strNum = (*it).substr(0, (*it).find_first_not_of("-0123456789."));
			else strNum = *it;
			if ((*it).find_first_not_of("-0123456789. *") != std::string::npos)
				strAlpha = (*it).substr((*it).find_first_not_of("-0123456789. *"));
			strNum = simplePrefixNum(strNum);
			strNum = prefixNum(RemoveTrailZeros(Execute(oper, strNum, num)),
							   ((!strAlpha.empty()) ? " * " : ""));
			*it = strNum + strAlpha;
		}
	}
}

std::string RevPolNotation::AlphaNum(const std::string &oper, const std::string &f, const std::string &s) {
	std::string strRes;
	std::list<std::string> lst = DenomElems(f);

	if (!oper.compare("*") || !oper.compare("/") || !oper.compare("%")) {
		if (!oper.compare("*") && !s.compare("0") && s.size() == 1) return "0";
		if (!oper.compare("/") && !s.compare("0") && s.size() == 1) {
			calcError = true;
			errmsg = "error: division by zero is undefined";
			return "";
		}
		if (s == "1") return f;
		if (s == "0" && oper == "*") return s;
		else if (s == "0" && (oper == "/" || oper == "%"))
			return DivError();
		ComplexMultiDivNum(lst, s, oper);
		for (std::string elem : lst) strRes += elem + " ";
		strRes.resize(strRes.size() - 1);
	}
	else if (!oper.compare("-") || !oper.compare("+")) {
		if (s == "0") return f;
		ComplexSumSubtractNum(lst, s, oper);
		for (std::string elem : lst) strRes += elem + " ";
		strRes.resize(strRes.size() - 1);
	}
	else if (!oper.compare("^")) {
		if (s == "0") return "1";
		if (s == "1") return f;
		bool brace = (lst.size() != 1);
		//if (!brace) brace = FindMultiOrDiv(lst.back());
		std::string f_brace = (s.front() == '-') ? "(" : "";
		std::string s_brace = (s.front() == '-') ? ")" : "";
		if (s == "0") strRes = "1";//strRes = ((brace) ? "(" : "") + f + ((brace) ? ")" : "");
		else
			strRes = ((brace) ? "(" : "") + f + ((brace) ? ")" : "") + " " + oper
					 + " " + f_brace + s + s_brace;
		if (!brace) strRes = ComplexPowRaise(strRes);
	}
	return strRes;
}

std::string RevPolNotation::NumAlpha(const std::string &oper, const std::string &f, const std::string &s) {
	std::string strRes;
	std::list<std::string> lst = DenomElems(s);

	if (!oper.compare("*")) {
		if (!f.compare("0") && f.size() == 1) return "0";
		ComplexMultiDivNum(lst, f, oper);
		for (std::string elem : lst) strRes += elem + ((elem.compare(lst.back()) ? " " : ""));
	} else if (!oper.compare("/") || !oper.compare("%")) {
		if (!f.compare("0") && f.size() == 1) return "0";
		if (lst.size() > 1) strRes = f + " " + oper + " (" + s + ")";
		else {
			std::string strNum = s.substr(0, s.find_first_not_of("-0123456789."));
			std::string strAlpha = s.substr(s.find_first_not_of("-0123456789. *"));
			strNum = RemoveTrailZeros(Execute(oper, f, simplePrefixNum(strNum)));
			strRes = strNum + " " + oper + " " + strAlpha;
		}
	}
	else if (!oper.compare("-")) {
		ComplexMultiDivNum(lst, "-1", "*");
		ComplexSumSubtractNum(lst, f, "+");
		for (std::string elem : lst) strRes += elem + " ";
		strRes.resize(strRes.size() - 1);
	} else {
		ComplexSumSubtractNum(lst, f, oper);
		for (std::string elem : lst) strRes += elem + " ";
		strRes.resize(strRes.size() - 1);
	}
	return strRes;
}

std::string RevPolNotation::NumBracketing(const std::list<std::string> &lst) {
	std::string res;
	std::list<double> nums;
	std::list<std::string>::const_iterator it = lst.begin();
	std::set<double> decPart = {0.000001, 0.00001, 0.0001, 0.001, 0.01, 0.1, 1};
	double multi = 0;

	if (lst.size() == 1) return lst.front();
	while (it != lst.end() && (*it).compare("+") && (*it).compare("-")) {
		std::string tmp = (*it).substr(0, (*it).find_first_not_of("-0123456789."));
		tmp = simplePrefixNum(tmp);

		nums.push_back(std::stod(tmp));
		++it;
	}
	nums.sort();
	for (double i = 0; i < nums.front(); i += 0.000001) {
		if (auto search = decPart.find(i); search == decPart.end()) {
			bool factor = true;
			std::list<double>::iterator numsIt;
			for (numsIt = nums.begin(); numsIt != nums.end(); ++numsIt) {
				if (std::fmod(*(numsIt), i)) factor = false;
			}
			if (factor) multi = i;
		}
	}
	if (multi) {
		res += RemoveTrailZeros(std::to_string(multi)) + " * (";
		std::list<double>::iterator itNums = nums.begin();
		for (it = lst.begin(); it != lst.end(); ++it) {
			double dTmp;
			std::string tmp = (*it).substr((*it).find_first_not_of("-0123456789 *"));

			dTmp = *(itNums++) / multi;
			if (dTmp == -1.0) res.push_back('-');
			else if (dTmp == 1.0) res += "";
			else res += std::to_string(dTmp) + " * ";
			++it;
			res += tmp + ((it != lst.end()) ? *it + " " : "");
		}
		res.push_back(')');
	}
	else {
		res.push_back('(');
		for (it = lst.begin(); it != lst.end(); ) {
			res += *it;
			++it;
			if (it != lst.end()) res += " ";
		}
		res.push_back(')');
	}
	return res;
}

std::list<std::string> RevPolNotation::PowLevel(const std::string &src) {
	std::list<std::string> res;
	std::string tmp, num, alpha;
	std::set<std::string>::iterator it;
	int size = 0;

	if (src.find_first_not_of("-123456789.* ") != std::string::npos)
		alpha = src.substr(src.find_first_not_of("-0123456789.* "));
	else {
		res.push_back(src);
		res.push_back("1");
		return res;
	}
	tmp = alpha;
	for (it = funcs.begin(); it != funcs.end(); ++it) {
		if (alpha.size() < (*it).size()) continue ;
		tmp = alpha.substr(alpha.find_first_not_of(*it));
		if ((size = alpha.size() - tmp.size())) break ;
	}
	if (tmp.front() == '(') {
		int brace = 1;
		int i = 1;
		for ( ; i < tmp.length(); ++i) {
			if (tmp[i] == '(') ++brace;
			else if (tmp[i] == ')') --brace;
			if (!brace) break ;
		}
		alpha = src.substr(0, i + size + 1);
		tmp = tmp.substr(i + 1);
		if (tmp.find_first_of("+-/") != std::string::npos) {
			res.push_back(src);
			return res;
		}
		num = "1";
		if (!tmp.empty())
			num = tmp.substr(tmp.find_first_not_of(" ^"));
	} else {
		int pos = src.find_first_not_of("-0123456789.* ");
		if (src.find_first_not_of(" ", pos) || src.find(" ", pos) != std::string::npos)
			alpha = src.substr(0, src.find_first_not_of(" ", pos) + 1);
		else alpha = src;
		if (src.find_first_of("^") != std::string::npos) {
			num = src.substr(src.find_first_of("^") + 1);
			if (num.find_first_of("(") != std::string::npos)
				num = num.substr(num.find_first_of("(") + 1);
			if (num.find_last_of(")") != std::string::npos)
				num = num.substr(0, num.find_last_of(")"));
		} else num = "1";
	}
	res.push_back(alpha);
	res.push_back(num.substr(num.find_first_not_of(" ")));
	return res;
}

std::list<std::string> RevPolNotation::ComplexDivisionAlpha(const std::string &oper,
											const std::list<std::string> &fLst,
											const std::list<std::string> &sLst) {
	std::list<std::string> res;
	std::string f, s, fNum, sNum, fAlpha, sAlpha, num, alpha, powLevel;
	std::list<std::string> fPow, sPow;

	f = NumBracketing(fLst);
	s = NumBracketing(sLst);
	fPow = PowLevel(f);
	sPow = PowLevel(s);
	fNum = fPow.front().substr(0, fPow.front().find_first_not_of("-0123456789."));
	sNum = sPow.front().substr(0, sPow.front().find_first_not_of("-0123456789."));
	fAlpha = fPow.front().substr(fPow.front().find_first_not_of("-123456789.* "));
	sAlpha = sPow.front().substr(sPow.front().find_first_not_of("-123456789.* "));
	fNum = simplePrefixNum(fNum);
	sNum = simplePrefixNum(sNum);
	num = RemoveTrailZeros(Execute(oper, fNum, sNum));
	if ((num.size() == 1 && !num.compare("0")) ||
		(!oper.compare("%") && fAlpha.size() == sAlpha.size()
		&& !fAlpha.compare(sAlpha) && fPow.back().size() == sPow.back().size()
		&& !fPow.back().compare(sPow.back()))) {
			res.push_back("0"); return res;
	}
	if (fAlpha.size() == sAlpha.size() && !fAlpha.compare(sAlpha)) {
		powLevel = RemoveTrailZeros(Execute("-", fPow.back(), sPow.back()));
		if (powLevel.compare("0"))
			alpha = fAlpha + " ^ " + powLevel;
	} else
		alpha = fAlpha + ((fPow.back().empty() || (!fPow.back().compare("1") && fPow.size() == 1)) ? "" : (" ^ " + fPow.back()))
				+ " / " + sAlpha + ((sPow.back().empty() || (!sPow.back().compare("1") && sPow.size() == 1)) ? "" : (" ^ " + sPow.back()));
	num = prefixNum(num, " * ");
	if (alpha.empty()) num = simplePrefixNum(num);
	res.push_back(num + alpha);
	return res;
}

std::list<std::string> RevPolNotation::ComplexMultiplicAlpha(const std::string &oper,
											 const std::list<std::string> &fLst,
											 const std::list<std::string> &sLst) {
	std::list<std::string>::const_iterator itF;
	std::list<std::string> res;
	std::string f, s, fNum, sNum, fAlpha, sAlpha, num, alpha, powLevel;
	std::list<std::string> fPow, sPow;

	for (itF = fLst.begin(); itF != fLst.end(); ++itF) {
		fAlpha.clear();
		fNum.clear();
		fPow.clear();
		std::string first;
		std::list<std::string>::const_iterator itS;

		first = *itF;
		if (!(*itF).compare("+") || !(*itF).compare("-")) continue ;
		if (itF != fLst.begin()) {
			--itF;
			if (!(*itF++).compare("-"))
				first = RemoveTrailZeros(Execute("*", "-1", first));
		}
		fNum = simplePrefixNum(first.substr(0, first.find_first_not_of("-0123456789.")));
		fPow = PowLevel(first);
		if (fPow.front().find_first_not_of("-123456789.* ") != std::string::npos)
			fAlpha = fPow.front().substr(fPow.front().find_first_not_of("-123456789.* "));
		for (itS = sLst.begin(); itS != sLst.end(); ++itS) {
			sAlpha.clear();
			sNum.clear();
			sPow.clear();
			std::string second;
			std::string power;
			auto postfixPowNum = [](const std::string &powNum) {
				return (!powNum.compare("1") && powNum.size() == 1 ? "" :
						" ^ " + powNum);
			};
			second = *itS;
			if (!(*itS).compare("+") || !(*itS).compare("-")) continue ;
			if (itS != sLst.begin()) {
				--itS;
				if (!(*itS).compare("-"))
					second = RemoveTrailZeros(Execute("*", "-1", second));
				++itS;
			}
			sNum = simplePrefixNum(second.substr(0, second.find_first_not_of("-0123456789.")));
			sPow = PowLevel(second);
			if (sPow.front().find_first_not_of("-123456789.* ") != std::string::npos)
				sAlpha = sPow.front().substr(sPow.front().find_first_not_of("-123456789.* "));
			second = prefixNum(RemoveTrailZeros(Execute("*", fNum, sNum)), " * ");
			if (fAlpha.empty() && sAlpha.empty()) res.push_back(simplePrefixNum(second));
			else if (fAlpha.empty() && !sAlpha.empty())
				res.push_back(second + sAlpha + postfixPowNum(sPow.back()));
			else if (!fAlpha.empty() && sAlpha.empty())
				res.push_back(second + fAlpha + postfixPowNum(fPow.back()));
			else if (fAlpha.size() == sAlpha.size() && !fAlpha.compare(sAlpha)) {
				power = RemoveTrailZeros(Execute("+", fPow.back(), sPow.back()));
				if (power.size() == 1 && !power.compare("0")) {
					if (second.find_first_of(" *") != std::string::npos)
						second = second.substr(0, second.find_first_of(" *"));
					second = simplePrefixNum(second);
					res.push_back(second);
				}
				else if (power.size() == 1 && !power.compare("1"))
					res.push_back(second + fAlpha);
				else res.push_back(second + fAlpha + " ^ " + power);
			}
			else {
				res.push_back(second + fAlpha + postfixPowNum(fPow.back()) + " * "
							  + sAlpha + postfixPowNum(sPow.back()));
			}
		}
	}
	return res;
}

std::list<std::string> RevPolNotation::ComplexMultiDivAlpha(const std::string &oper,
											const std::list<std::string> &fLst,
											const std::list<std::string> &sLst) {
	std::list<std::string>::const_iterator itF;
	std::list<std::string> res;
	std::string f, s, fNum, sNum, fAlpha, sAlpha, num, alpha, powLevel;
	std::list<std::string> fPow, sPow;

	if (!oper.compare("/") || !oper.compare("%"))
		return ComplexDivisionAlpha(oper, fLst, sLst);
	return ComplexMultiplicAlpha(oper, fLst, sLst);
}

std::string RevPolNotation::AlphaFinalSumSub(const std::list<std::string> &lst) {
	std::vector<std::string> aux(lst.begin(), lst.end());
	std::set<int> expired;
	std::string fNum, sNum, fAlpha, sAlpha, res;

	for (int i = 0; i < aux.size(); ++i) {
		fAlpha.clear();
		fNum.clear();
		if (expired.find(i) != expired.end()) continue ;
		if (aux[i].find_first_not_of("-0123456789.") != std::string::npos)
			fNum = simplePrefixNum(aux[i].substr(0, aux[i].find_first_not_of("-0123456789.")));
		else fNum = aux[i];
		if (aux[i].find_first_not_of("-0123456789.*") != std::string::npos)
			fAlpha = aux[i].substr(aux[i].find_first_not_of("-0123456789.* "));
		for (int j = i + 1; j < aux.size(); ++j) {
			sAlpha.clear();
			sNum.clear();
			if (expired.find(j) != expired.end()) continue ;
			if (aux[j].find_first_not_of("-0123456789.") != std::string::npos)
				sNum = simplePrefixNum(aux[j].substr(0, aux[j].find_first_not_of("-0123456789.")));
			else sNum = aux[j];
			if (aux[j].find_first_not_of("-0123456789.*") != std::string::npos)
				sAlpha = aux[j].substr(aux[j].find_first_not_of("-0123456789.* "));
			if (fAlpha.size() == sAlpha.size() && !fAlpha.compare(sAlpha)) {
				fNum = RemoveTrailZeros(Execute("+", fNum, sNum));
				expired.insert(j);
			}
		}
		if (!fNum.compare("0") && fNum.size() == 1) continue ;
		if (!res.empty()) {
			if (fNum.front() == '-')
			{ res += " - "; fNum.erase(fNum.begin()); }
			else res += " + ";
		}
		res += (!fAlpha.empty() ? prefixNum(fNum, " * ") + fAlpha : fNum);
	}
	if (res.empty()) res = "0";
	return res;
}

std::list<std::string> RevPolNotation::ComplexSubSumAlpha(const std::string &oper,
										  const std::list<std::string> &fLst,
										  const std::list<std::string> &sLst) {
	std::list<std::string> res;
	std::string fNum, sNum, fAlpha, sAlpha;
	std::set<int> indicies;
	int i;

	for (std::list<std::string>::const_iterator itF = fLst.begin(); itF != fLst.end(); ++itF) {
		i = 0;

		fAlpha.clear();
		fNum.clear();
		if ((!(*itF).compare("+") || !(*itF).compare("-")) && (*itF).size() == 1) continue ;
		fNum = simplePrefixNum((*itF).substr(0, (*itF).find_first_not_of("-0123456789.")));
		if ((*itF).find_first_not_of("-0123456789. *") != std::string::npos)
			fAlpha = (*itF).substr((*itF).find_first_not_of("-0123456789. *"));
		if (itF != fLst.begin()) {
			--itF;
			if ((*itF).size() == 1 && !(*itF).compare("-"))
				fNum = RemoveTrailZeros(Execute("*", "-1", fNum));
			++itF;
		}
		for (std::list<std::string>::const_iterator itS = sLst.begin(); itS != sLst.end(); ++itS) {
			sAlpha.clear();
			sNum.clear();
			if ((!(*itS).compare("+") || !(*itS).compare("-")) && (*itS).size() == 1) continue ;
			if (auto search = indicies.find(i); search != indicies.end()) continue ;
			sNum = simplePrefixNum((*itS).substr(0, (*itS).find_first_not_of("-0123456789.")));
			if ((*itS).find_first_not_of("-0123456789. 	*") != std::string::npos)
				sAlpha = (*itS).substr((*itS).find_first_not_of("-0123456789. 	*"));
			if (itS != sLst.begin()) {
				--itS;
				if ((*itS).size() == 1 && !(*itS).compare("-"))
					sNum = RemoveTrailZeros(Execute("*", "-1", sNum));
				++itS;
			}
			if (fAlpha.size() == sAlpha.size() && !fAlpha.compare(sAlpha)) {
				if (itS != sLst.begin()) indicies.insert(i - 1);
				indicies.insert(i);
				fNum = RemoveTrailZeros(Execute(oper, fNum, sNum));
			}
			++i;
		}
		if (fNum.compare("0") || fNum.size() != 1)
			res.push_back(((!fAlpha.empty()) ? prefixNum(fNum, " * ") : fNum)+ fAlpha);
	}
	i = 0;
	for (std::list<std::string>::const_iterator itS = sLst.begin(); itS != sLst.end(); ++itS) {
		if ((!(*itS).compare("+") || !(*itS).compare("-")) && (*itS).size() == 1) {
			++i;
			continue ;
		}
		if (auto search = indicies.find(i); search != indicies.end()) {
			++i;
			continue ;
		}
		if ((*itS).find_first_not_of("-0123456789.") != std::string::npos)
			sNum = simplePrefixNum((*itS).substr(0, (*itS).find_first_not_of("-0123456789.")));
		else sNum = *itS;
		if (sNum.size() == 1 && !sNum.compare("0")) continue ;
		if ((*itS).find_first_not_of("-0123456789. 	*") != std::string::npos)
			sAlpha = (*itS).substr((*itS).find_first_not_of("-0123456789. 	*"));
		if (itS != sLst.begin()) {
			--itS;
			if ((*itS).size() == 1 && !(*itS).compare("-"))
				sNum = RemoveTrailZeros(Execute("*", "-1", sNum));
			++itS;
		}
		if (!oper.compare("-")) sNum = RemoveTrailZeros(Execute("*", "-1", sNum));
		res.push_back(((!sAlpha.empty()) ? prefixNum(sNum, " * ") : sNum) + sAlpha);
		++i;
	}
	if (res.empty()) res.push_back("0");
	return res;
}

std::string RevPolNotation::AlphaAlpha(const std::string &oper, const std::string &f, const std::string &s) {
	std::list<std::string> fLst, sLst, resLst;
	std::string res;

	fLst = DenomElems(f);
	sLst = DenomElems(s);
	if (!oper.compare("*") || !oper.compare("/") || !oper.compare("%"))
		resLst = ComplexMultiDivAlpha(oper, fLst, sLst);
	else if (!oper.compare("+") || !oper.compare("-"))
		resLst = ComplexSubSumAlpha(oper, fLst, sLst);
	else {
		bool fBr = fLst.size() != 1 || FindMultiOrDiv(fLst.back());
		bool sBr = sLst.size() != 1 || FindMultiOrDiv(sLst.back());
		res += ((!fBr) ? "" : "(") + f + ((!fBr) ? "" : ")")
			   + " ^ " + ((!sBr) ? "" : "(") + s + ((!sBr) ? "" : ")");
		return res;
	}
	res = AlphaFinalSumSub(resLst);
	//if (DenomElems(res).size() == 1)
	SeekComplex(res);
	if (res.empty()) res.push_back('0');
	return res;
}

//Seek complex number in equation, and if it's necessary fix it
void RevPolNotation::SeekComplex(std::string &s) {
	std::list<std::string> lst;
	std::string tmp;
	bool cPow = false;
	auto it = lst.begin();
	int br = 0;
	auto ifDigit = [](const char &c) { return std::isdigit(c); };

	for (char c : s) {
		if (std::isspace(c) && !br) continue ;
		if (c == '(') ++br;
		else if (c == ')') --br;
		else if (baseOpers.find(c) != std::string::npos && !br) {
			lst.push_back(std::move(tmp));
			if (lst.size() == 1) it = lst.begin();
			if (!cPow && /*c != '^' && */lst.size() > 1)
				++it;
			if (*it == "i" && c == '^') cPow = true;
			if (cPow && std::find_if(lst.back().begin(), lst.back().end(), ifDigit) != it->end()) {
				int tmp = std::stoll(lst.back());

				if (!(std::fmod(tmp, 2))) {
					*it = Execute("^", *it, lst.back());
					it = lst.erase(++it); it = lst.erase(it);
					if (it == lst.end()) it = lst.begin();
					else --it; cPow = false;
				} else {
					bool neg = tmp < 0;
					lst.back() = ((neg) ? "(" : "") + lst.back() + ((neg) ? ")" : "");
				}
			}
			lst.push_back(std::string(1, c));
			if (!cPow && /*c != '^' &&*/ lst.size() > 1) ++it;
			continue ;
		}
		tmp.push_back(c);
	}
	lst.push_back(std::move(tmp));
	if (cPow && std::find_if(lst.back().begin(), lst.back().end(), ifDigit) != it->end()) {
		int tmp = std::stoll(lst.back());

		if (!(std::fmod(tmp, 2))) {
			*it = Execute("^", *it, lst.back());
			it = lst.erase(++it); it = lst.erase(it);
			if (it == lst.end()) it = lst.begin();
			else --it;
		}
		else {
			bool neg = tmp < 0;
			lst.back() = ((neg) ? "(" : "") + lst.back() + ((neg) ? ")" : "");
		}
	}
	std::string old = s;
	s.clear();
	for (std::string v : lst) {
		bool isoper = (baseOpers.find(v) != std::string::npos);
		s.append(((isoper) ? " " : "") + v + ((isoper) ? " " : ""));
	}
	if (s != old) {
		RevPolNotation kek(std::move(s), userDefFuncs);
		kek.setToken(token);
		s = kek.CalcIt();
	}
	//return s;
}

//Division error signal
std::string RevPolNotation::DivError() {
	calcError = true;
	errmsg = "error division by zero is undefined";
	return "";
}

std::string RevPolNotation::Execute(const std::string &oper, const std::string &f, const std::string &s) {
	std::string strFirstNum, strSecondNum;
	long double fNum, sNum, res = 0;
	bool ifAlphaFirst, ifAlphaSecond;
	auto ifAlpha = [](const char &c) { return std::isalpha(c); };
	//string stream for output
	std::stringstream oss;
	//complex number postfix
	std::string postCom;

	ifAlphaFirst = std::find_if(f.begin(), f.end(), ifAlpha) != f.end();
	ifAlphaSecond = std::find_if(s.begin(), s.end(), ifAlpha) != s.end();

	if (ifAlphaFirst && !ifAlphaSecond)
		return AlphaNum(oper, f, s);
	else if (!ifAlphaFirst && ifAlphaSecond)
		return NumAlpha(oper, f, s);
	else if (ifAlphaFirst && ifAlphaSecond)
		return AlphaAlpha(oper, f, s);
	fNum = std::stold(f);
	sNum = std::stold(s);

	if (!oper.compare("+")) res = fNum + sNum;
	else if (!oper.compare("-")) res = fNum - sNum;
	else if (!oper.compare("*")) res = fNum * sNum;
	else if (!oper.compare("/")) {
		if (!sNum) return DivError();
		res = fNum / sNum;
	}
	else if (!oper.compare("^")) {
		//If fNum is a negative number and
		//pow is a form of a square root
		//we deal with complex number
		if (fNum < 0 && !std::fmod((1 / sNum ), 2)) {
			//generate postfix part for
			//complex number
			fNum *= -1;
			postCom = " * i";
		}

		res = std::pow(fNum, sNum);
	}
	else if (!oper.compare("%")) {
		if (!sNum) return DivError(/*calcError, errmsg*/);
		res = std::fmod(fNum, sNum);
	}
	oss << std::fixed << std::setprecision(10) << res;
	return oss.str() + postCom;
}

//Find multiplication or division outside braces in equation
bool RevPolNotation::FindMultiOrDiv(const std::string &s) {
	//brace counter
	int brace = 0;

	for (char c : s) {
		if (c == '(') ++brace;
		else if (c == ')') --brace;
		else if ((c == '*' || c == '/') && !brace)
			return true;
	}
	return false;
}

//Seeking complex number if power raising value
std::string RevPolNotation::ComplexPowRaise(const std::string &s) {
	bool isInBrace = false, calcpow = false;
	int i = 0, brace = 0;
	std::string pow, tmp;
	std::list<std::string> multi;
	if (s[i] == '(') {
		isInBrace = true;
		++i;
	}
	auto ifDigit = [](const char &c) { return std::isdigit(c); };

	for (char c : s) {
		if (c == '(') ++brace;
		else if (c == ')') {
			if (isInBrace && !brace) continue;
			--brace;
		}
		else if ((c == '*' || c == '/') && !brace && !calcpow) {
			tmp = tmp.substr(tmp.find_first_not_of(" 	"), tmp.find_last_not_of(" 	")
							 - tmp.find_first_not_of(" 	") + 1);
			multi.insert(multi.end(), { std::move(tmp), std::string(1, c) });
			continue ;
		}
		else if (c == '^' && !brace) {
			multi.push_back(tmp.substr(tmp.find_first_not_of("	 "), tmp.find_last_not_of(" 	")
									   - tmp.find_first_not_of(" 	") + 1));
			calcpow = true; continue;
		}
		if (!calcpow) tmp.push_back(c);
		else if (!std::isspace(c)) {
			if (c == ')' || c == '(') continue ;
			pow.push_back(c);
		}
	}
	bool neg = std::stod(pow) < 0;
	auto num = multi.end();
	for (auto it = multi.begin(); it != multi.end(); ++it) {
		bool isnum = false;

		if (*it == "*" || *it == "/") continue ;
		else if (*it == "i" && !std::fmod(std::stod(pow), 2.0)) {
			*it = "-1"; isnum = true;
		}
		else if (std::find_if(it->begin(), it->end(), ifDigit) != it->end()) {
			*it = Execute("^", *it, pow); isnum = true;
		}
		else *it += std::string(" ^ ") + ((neg) ? "(" : "") + pow + ((neg) ? ")" : "");

		if (num == multi.end() && isnum) num = it;
		else if (isnum) {
			tmp = *(--it);
			it = multi.erase(it);
			*num = Execute(tmp, *num, *it);
			it = multi.erase(it);
			if (it == multi.end()) break;
			else --it;
		}
	}
	tmp.clear();
	for (std::string v : multi) {
		bool isoper = (v == "*" || v == "/");
		tmp.append(((isoper) ? " " : "") + v + ((isoper) ? " " : ""));
	}
	return tmp;
}

//Private method. It returns exposed func string, if func's
//operand contains alphabetical character
std::string RevPolNotation::UDfuncExpose(const Func &src, const std::string &forReplace) {
	//Equation itself
	std::string res = src.equation, replace = "(" + forReplace + ")";
	//Number of elements in equation
	int num;
	//Expression calculator
	RevPolNotation pol(userDefFuncs);

	if (forReplace == src.token) return res;
	//search iterator
	size_t s = 0;
	while(true) {
		if (s = res.find(src.token, s); s != std::string::npos) {
			res.replace(s, src.token.size(), replace);
			s += replace.size();
			continue ;
		}
		break ;
	}
	//Let's calculate 'res' string
	pol.setToken(token);
	pol.setInfixExpr(std::move(res));
	res = pol.CalcIt();
	if (!pol.getErrMsg().empty()) calcError = true;
	return (pol.getErrMsg().empty()) ? res : pol.getErrMsg();
}

//Private method. It executes base and user defined functions
std::string RevPolNotation::funcExecute(const std::string &oper, const std::string &var) {
	double res = 0;
	bool hasAlpha;
	auto ifAlpha = [](const char &c) { return std::isalpha(c); };
	auto search = userDefFuncs.find(oper);
	//string stream for output
	std::stringstream oss;
	//Postfix for complex number
	std::string postCom;

	hasAlpha = std::find_if(var.begin(), var.end(), ifAlpha) != var.end();
	if (hasAlpha && search == userDefFuncs.end())
		return oper + "(" + ElemsSort(var) + ")";
	else if (hasAlpha && search != userDefFuncs.end()) return UDfuncExpose(search->second, var);

	if (search != userDefFuncs.end()) {
		std::string calc = UDfuncExpose(search->second, var);
		if (calcError) {
			errmsg = calc;
			return "";
		}
		return RevPolNotation(std::move(calc), userDefFuncs).CalcIt();
	}
	if (!oper.compare("sin")) res = std::sin(std::stold(var));
	else if (!oper.compare("cos")) res = std::cos(std::stold(var));
	else if (!oper.compare("tan")) {
		res = std::tan(std::stod(var));
		if (res < -65315780005) {
			calcError = true;
			errmsg = "error: tan(pi/2) and tan(3*pi/2) are undefined value";
			return "";
		}
	}
	else if (!oper.compare("exp")) res = std::exp(std::stold(var));
	else if (!oper.compare("sqrt")) {
		res = std::stold(var);

		//If number is negative, we deal with complex number
		//and generate complex number postfix for result
		if (res < 0) {
			res *= -1;
			postCom = " * i";
		}
		res = std::sqrt(std::stold(var));
	}

	else if (!oper.compare("abs")) res = std::abs(std::stold(var));
	else if (oper == "rad") res = ToRadians(std::stold(var));
	else if (oper == "acos") {
		res = std::stold(var);
		if (res < -1 || res > 1) {
			calcError = true;
			errmsg = "error: acos: number range should be between -1 and 1: " + var;
			return "";
		}
		res = std::acos(res);
	}
	else if (oper == "asin") {
		res = std::stold(var);
		if (res < -1 || res > 1) {
			calcError = true;
			errmsg = "error: asin: number range should be between -1 and 1: " + var;
			return "";
		}
		res = std::asin(res);
	}
	else if (oper == "atan") res = std::atan(std::stold(var));
	else if (oper == "ceil") res = std::ceil(std::stold(var));
	else if (oper == "floor") res = std::floor(std::stold(var));
	else if (oper == "cosh") res = std::asin(std::stold(var));
	else if (oper == "log") {
		res = std::stold(var);
		if (res <= 0) {
			calcError = true;
			errmsg = "error: log: logarithm of negative number and zero is not supported: " + var;
			return "";
		}
		res = std::log(res);
	}
	else if (oper == "logt") {
		res = std::stold(var);
		if (res <= 0) {
			calcError = true;
			errmsg = "error: logt(log10): logarithm of negative number and zero is not supported : " + var;
			return "";
		}
		res = std::log10(res);
	}
	else if (oper == "tanh") res = std::tanh(std::stold(var));
	else if (oper == "deg") res = ToDegrees(std::stold(var));
	else if (oper == "sinh") res = std::sinh(std::stold(var));

	//return std::to_string(res);
	oss << std::fixed << std::setprecision(10) << res;
	return oss.str() + postCom;
}

std::string		RevPolNotation::ProcessPostfix() {
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
	auto markPlace = [](const std::string &src, int i) {
		std::string res = src;
		res.push_back('\n');
		res.insert(res.size(), i, ' ');
		res.push_back('^');
		return res;
	};
	//Parsing string
	for (int i = 0; i < inifixExpr.size(); ++i) {
		//Passing whitespaces
		while (std::isspace(inifixExpr[i])) ++i;
		//If character is digit
		if (std::isdigit(inifixExpr[i]) || inifixExpr[i] == '.') {
			if (!number) number = true;
			else {
				//std::cerr << "wrong order: there should be number" << std::endl;
				errmsg = markPlace(inifixExpr, i) + "\nerror: wrong order: there should be a number";
				res.clear(); return res;
			}
			res += GetStringNumber(inifixExpr,i) + " ";
			if (res.empty()) {
				//std::cerr << "wrong number syntax" << std::endl;
				errmsg = markPlace(inifixExpr, i) + "\nerror: wrong number syntax";
				res.clear(); return res;
			}
		}
		//if character is a left braced
		else if (inifixExpr[i] == '(') {
			++brace;
			//Push it into stack and announcing about a left brace
			std::string brace_str;
			//If there is nothing between left and right braces
			//there should be error
			if (inifixExpr[i + 1] == ')') {
				errmsg = markPlace(inifixExpr, i) + "\nerror: there is no content between left and right braces";
				res.clear(); return res;
			}
			brace_str.push_back(inifixExpr[i]);
			if (funcBrace == brace && funcBrace) {
				std::string tmp = oper.top();
				oper.pop();
				oper.push(brace_str);
				oper.push(tmp);
			}
			else
			oper.push(brace_str);
		}
		//Check a wrong right brace case
		else if (inifixExpr[i] == ')' && !brace) {
			//std::cerr << "extra braces found" << std::endl;
			errmsg = markPlace(inifixExpr, i) + "\nerror: extra braces found";
			res.clear(); return res;
		}
		//Check correct right brace case
		else if (inifixExpr[i] == ')') {
			//Check if ther was some function encounter in previous iteration
			//if there was error shoud be marked
			if (funcBrace == brace) {
				//non brace and space next character position
				int charPos = inifixExpr.find_first_not_of(")	 ", i);
				//check if we deal with end of the expression
				//if we do, just pass it, we check next character
				//is operator
				if (charPos != std::string::npos) {
					//If there no operator, we mark error, and stop infix expression processing
					if (auto search = baseOpers.find(inifixExpr[charPos]); search == std::string::npos) {
						errmsg = markPlace(inifixExpr, charPos) + "\nerror: there should be some base operator";
						res.clear();
						return res;
					}
				}
				--funcBrace;
			}
			//Pop every operator and function until we stuck at left brace
			while (!oper.empty() && oper.top().compare("(")) {
				res += oper.top(); oper.pop(); res.push_back(' ');
			}
			oper.pop();
			--brace;
		}
		//if character is alphabetical
		else if (std::isalpha(inifixExpr[i]) || baseOpers.find(inifixExpr[i]) != std::string::npos) {
			//char	tmp = inifixExpr[i];
			//Assign character to 'c' variable
			char c = inifixExpr[i];
			//Generating string (tmp) while we deal with alphabetical characters
			std::string	tmp;
			while (std::isalpha(inifixExpr[i]))
				tmp.push_back(inifixExpr[i++]);

			//Move carriage back if there were alpha characters
			//If there weren't, we deal with binary operator
			//and it should be pushed into 'tmp' string. If there were alpha characters
			//check if we deal with function and there is a left brace
			//as a next character
			if (inifixExpr[i] != c) {
				//Search funcs in basic functions set
				auto search = operPriority.find(tmp);
				//Search func in user defined function set
				auto uDSearch = userDefFuncs.find(tmp);
				//lambda function check operator or function and operands order
				auto errorRet = [](bool &number, const std::string &errstr,
						std::string &res, const std::string &add, int &indx) {
					if (!number) number = true;
					else {
						res.clear();
						return errstr;
					}
					res += add + " ";
					--indx;
					return std::string();
				};
				//Passing whitespaces
				while (std::isspace(inifixExpr[i])) ++i;
				//If we got pi number, check operators and operands order
				if (tmp.size() == 2 && !tmp.compare("pi")) {
					std::stringstream oss;
					//Check operation and operands order
					oss << std::setprecision(15) << M_PI;
					errmsg = errorRet(number, "\nerror: wrong order: there should be operator or function", res,
							   oss.str(), i);
					//If error occured, mark place where error occured
					if (!errmsg.empty()) errmsg.insert(0, markPlace(inifixExpr, i));
					if (res.empty()) return res;
					/*else {
						res += oss.str() + " ";
					}*/
					continue ;
				}
				//Check if paramater is a token
				else if (!token.empty() && tmp.size() == token.size() && !tmp.compare(token)) {
					//Check operations and operands order
					errmsg = errorRet(number, "\nerror: wrong order: there should be operator or function", res, tmp, i);
					//If error occured, mark place where error occured
					if (!errmsg.empty()) errmsg.insert(0, markPlace(inifixExpr, i));
					if (res.empty()) return res;
					continue ;
				}
				//Check if character is a complex number
				else if (tmp.size() == 1 && !tmp.compare("i")) {
					//Check operand, operator order
					errmsg = errorRet(number, "\nerror: wrong order: there should be operator or function", res, tmp, i);
					//If error occured, mark place where error occured
					if (!errmsg.empty()) errmsg.insert(0, markPlace(inifixExpr, i));
					if (res.empty()) return res;
					continue ;
				}
				//Check if we deal with user defined function and next character
				//is a left brace. If we deal with it, we push function into
				//operator stack, otherwise we check if we deal with base functions
				else if (uDSearch != userDefFuncs.end() && inifixExpr[i] == '(') {
					oper.push(tmp); --i;
					funcBrace = brace + 1; continue;
				}
				//Check if we deal with some base function and next character
				//is a left brace. If we deal with it, we push function into
				//operator stack, otherwise we check if we deal with base functions
				else if (search != operPriority.end() && inifixExpr[i] == '(') {
					oper.push(tmp); --i;
					funcBrace = brace + 1; continue;
				} else {
					errmsg = markPlace(inifixExpr, i) +
							"\nerror: there should be open brace after function name";
					res.clear(); return res;
				}
			}
			//Check is operator unary minus.
			if (!number && inifixExpr[i] == '-' && std::isalnum(inifixExpr[i + 1])) {
				number = true;
				inifixExpr[i] = '~';
			}
			tmp.push_back(inifixExpr[i]);
			//Check is we don't deal with digit
			if (number) number = false;
			else {
				errmsg = markPlace(inifixExpr, i) +
						"\nerror: wrong order there should be a number";
				res.clear(); return res;
			}
			//Push into postfix string all stack operator with higher priority
			while (!oper.empty() && operPriority[oper.top()] >= operPriority[tmp]) {
				res += oper.top(); oper.pop(); res.push_back(' ');
			}
			//Push operator into stack
			oper.push(tmp);
		}
		//And finaly return error if we deal with some obscure character
		else {
			errmsg = markPlace(inifixExpr, i) +
					"\nerror: obscure character";
			res.clear(); return res;
		}
		//Passing whitespaces
		while (std::isspace(inifixExpr[i + 1])) ++i;
	}
	if (brace) {
		errmsg = markPlace(inifixExpr, inifixExpr.size() - 1) +
				"\nerror: number of open and close brace should be equal";
		res.clear(); return res;
	}
	//Push the rest into string
	while (!oper.empty()) {
		res += oper.top();
		oper.pop();
		if (!oper.empty()) res.push_back(' ');
	}
	return res;
}

//Removing trailing zeros in string
std::string RevPolNotation::RemoveTrailZeros(const std::string &str) {
	std::string res = str;

	if (res.find('.') != std::string::npos) {
		//Removing trailing zeros
		res = res.substr(0, str.find_last_not_of('0') + 1);
		//If the last character is point, remove it
		if (res.find('.') == res.size() - 1) res.resize(res.size() - 1);
	}
	return res;
}

//Converting degrees to radians
long double RevPolNotation::ToRadians(const long double &val) {
	return val * M_PI / 180;
}

//Convert radians to degrees
long double RevPolNotation::ToDegrees(const long double &val) {
	return val * M_PI / 180;
}

//Sorting function conatins
std::string RevPolNotation::ElemsSort(const std::string &src) {
	//Init elems for sort
	std::list<std::string> toSort = DenomElems(src);
	//Result string
	std::string res;

	//Reduce number of elements, so there will be only elements
	for (auto it = toSort.begin(); it != toSort.end(); ++it) {
		//If there is '+' or '-' multiply 1 or -1 correspondigly and next element,
		//then erase '+' or '-' element
		if (*it == "+" || *it == "-") {
			//'1' or '-1' multiplier
			std::string one = (*it == "+") ? "1" : "-1";
			it = toSort.erase(it);
			*it = RemoveTrailZeros(Execute("*", "1", *it));
		}
	}
	//Sort elements in ascension order
	toSort.sort(std::greater<std::string>());
	//Generate result string
	for (auto it = toSort.begin(); it != toSort.end(); ++it) {
		if (it == toSort.begin()) res.append(*it);
		else {
			//In case when first character is '-', remove it
			//and add subtraction in string with element
			if (it->at(0) == '-') {
				it->erase(0, 1);
				res.append(" - " + *it);
			}
			//Else add summation in string with element
			else
				res.append(" + " + *it);
		}
	}
	return res;
}

RevPolNotation::RevPolNotation(std::map<std::string, Func> &userDefFuncsRef) : baseOpers("+-/*^%~"), userDefFuncs(userDefFuncsRef) {
	operPriority["("] = 0;
	operPriority["+"] = 1;
	operPriority["-"] = 1;
	operPriority["*"] = 2;
	operPriority["/"] = 2;
	operPriority["%"] = 2;
	operPriority["^"] = 3;
	operPriority["~"] = 5;

	funcs = { "sin", "cos", "tan", "exp", "sqrt", "abs", "rad",
			  "acos", "asin", "atan", "ceil", "floor", "cosh",
			  "log", "logt", "tanh", "deg", "sinh" };
	for (std::string var : funcs)
		operPriority[var] = 0;
}


RevPolNotation::RevPolNotation(std::string &&init_expr, std::map<std::string, Func> &userDefFuncsRef)
	: baseOpers("+-/*^%~"), userDefFuncs(userDefFuncsRef) {
	operPriority["("] = 0;
	operPriority["+"] = 1;
	operPriority["-"] = 1;
	operPriority["*"] = 2;
	operPriority["/"] = 2;
	operPriority["%"] = 2;
	operPriority["^"] = 3;
	operPriority["~"] = 5;

	inifixExpr = std::move(init_expr);
	funcs = { "sin", "cos", "tan", "exp", "sqrt", "abs", "rad",
			  "acos", "asin", "atan", "ceil", "floor", "cosh",
			  "log", "logt", "tanh", "deg", "sinh" };
	for (std::string var : funcs)
		operPriority[var] = 0;
	if (!token.empty())
		StrMultiplySearch(inifixExpr, token);
	StrMultiplySearch(inifixExpr, "i");
	StrMultiplySearch(inifixExpr, "pi");
	postfixExpr = ProcessPostfix();
}

//Get infix notation
const std::string	&RevPolNotation::getInifixExpr() { return inifixExpr; }

//Get postfix notation
const std::string	&RevPolNotation::getPosfixExpr() { return postfixExpr; }

//Set new infix expression for further calculations
void				RevPolNotation::setInfixExpr(std::string &&move) {
	calcError = false;
	inifixExpr.clear(); postfixExpr.clear();
	inifixExpr = std::move(move);
	if (!token.empty())
		StrMultiplySearch(inifixExpr, token);
	StrMultiplySearch(inifixExpr, "i");
	StrMultiplySearch(inifixExpr, "pi");
	postfixExpr = ProcessPostfix();
}

//Calculation of postfix equation
std::string RevPolNotation::CalcIt() {
	//Stack of operands
	std::stack<std::string>		nums;
	//Result string
	std::string					res;
	//String parser
	std::string parser;
	//reset calcError for recalculcations;
	calcError = false;

	//Processing the postfix expression string
	for (int i = 0; i < postfixExpr.length(); ++i) {
		//push character into the parser string
		parser.push_back(postfixExpr[i]);
		//Check whether this number or no
		if (std::isdigit(postfixExpr[i]) || postfixExpr[i] == '.') {
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
		}
		//check if function is in the list of user defined strings
		else if (auto search = userDefFuncs.find(parser); search != userDefFuncs.end()
			&& !std::isalpha(postfixExpr[i + 1])) {
			//Check whether stack is empty.  If there is none, we get zero value
			//If there is some value we pop value from stack.
			std::string tmp = (nums.empty()) ? 0 : nums.top();
			if (!nums.empty()) nums.pop();
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

//Get string token value
const std::string	&RevPolNotation::getToken() { return token; }

//Set new value for string 'token'
void				RevPolNotation::setToken(const std::string &newToken) { token = newToken; }

//Get error message
const std::string	&RevPolNotation::getErrMsg() const { return errmsg; }
