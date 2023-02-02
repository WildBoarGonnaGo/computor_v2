#include <iostream>
#include <string>
#include <map>
#include <cctype>
#include <stack>
#include <cmath>
#include <set>
#include <algorithm>

class RevPolNotation {
	std::string			inifixExpr;
	std::string			postfixExpr;
	std::map<std::string, int>	operPriority;

	std::string			GetStringNumber(const std::string &str, int &i) {
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
		//Сдвигаем каретку
		--i;
		if (!res.compare(".")) res.clear();
		return res;
	}

	//Приватный метод для выполнения операторов
	std::string Execute(const std::string &oper, const std::string &f, const std::string &s) {
		double fNum, sNum, res = 0;
		bool ifAlphaFirst, ifAlphaSecond;
		auto ifAlpha = [](const char &c) { return std::isalpha(c); };

		ifAlphaFirst = std::find_if(f.begin(), f.end(), ifAlpha) != f.end();
		ifAlphaSecond = std::find_if(s.begin(), s.end(), ifAlpha) != s.end();


		if (ifAlphaFirst && ifAlphaSecond) return f + " " + oper + " " + s;
		fNum = std::stod(f);
		sNum = std::stod(s);

		if (!oper.compare("+")) res = fNum + sNum;
		else if (!oper.compare("-")) res = fNum - sNum;
		else if (!oper.compare("*")) res = fNum * sNum;
		else if (!oper.compare("/")) res = fNum / sNum;
		else if (!oper.compare("^")) res = std::pow(fNum, sNum);

		return std::to_string(res);
	}

	std::string funcExecute(const std::string &oper, const std::string &var) {
		double res = 0;
		bool hasAlpha;
		auto ifAlpha = [](const char &c) { return std::isalpha(c); };

		hasAlpha = std::find_if(var.begin(), var.end(), ifAlpha) != var.end();
		if (hasAlpha) return oper + "(" + var + ")";

		if (!oper.compare("sin")) res = std::sin(std::stod(var));
		else if (!oper.compare("cos")) res = std::cos(std::stod(var));
		else if (!oper.compare("tan")) res = std::tan(std::stod(var));
		else if (!oper.compare("exp")) res = std::exp(std::stod(var));
		else if (!oper.compare("sqrt")) res = std::sqrt(std::stod(var));
		else if (!oper.compare("abs")) res = std::abs(std::stod(var));

		return std::to_string(res);
	}

	std::string		ProcessPostfix() {
		//Результирующая строка
		std::string				res;
		//Стэк операторов в виде строк
		std::stack<std::string>	oper;
		//контролируем скобки
		int 					brace = 0;
		//контролируем очередность данных
		bool					number = false;
		//Строка состоящая из бинарных операторов
		std::string				opers("+ / - * ^");
		//Перебираем строку
		for (int i = 0; i < inifixExpr.size(); ++i) {
			//Если символ - число
			if (std::isdigit(inifixExpr[i]) || inifixExpr[i] == '.') {
				if (!number) number = true;
				else {
					std::cerr << "wrong order: there should be number" << std::endl;
					res.clear(); return res;
				}
				res += GetStringNumber(inifixExpr,i) + " ";
				if (res.empty()) {
					std::cerr << "wrong number syntax" << std::endl;
					res.clear(); return res;
				}
			}
			//Если символ - октрывающаяся скобка
			else if (inifixExpr[i] == '(') {
				//заносим в стэк и сигнализируем об открытых скобках
				std::string brace_str;
				brace_str.push_back(inifixExpr[i]);
				oper.push(brace_str); ++brace;
			}
			//Проверяем случай неправильной закрывающей скобки
			else if (inifixExpr[i] == ')' && !brace) {
				std::cerr << "extra spaces found" << std::endl;
				res.clear(); return res;
			}
			//Проверяем случай правильной закрывающей скобки
			else if (inifixExpr[i] == ')') {
				//Заносим в выходную строку из стэка все вплоть до открывающейся скобки
				while (!oper.empty() && oper.top().compare("(")) {
					res += oper.top(); oper.pop(); res.push_back(' ');
				}
				oper.pop();
			}
			//если символ - буква
			else if (std::isalpha(inifixExpr[i]) || opers.find(inifixExpr[i]) != std::string::npos) {
				//char	tmp = inifixExpr[i];
				//Фиксируем символ в переменной c
				char c = inifixExpr[i];
				//Формируем строку пока у нас идут буквенные символы
				std::string	tmp;
				while (std::isalpha(inifixExpr[i]))
					tmp.push_back(inifixExpr[i++]);

				//Сдвигаем каретку назад, если были буквы
				//Если их не было, значит перед нами бинарный оператор
				//и его следует добавить в tmp. Если были буквы, то
				//проверяем что перед нами функция и проверяем также, чтобы
				//были открывающиеся скобки
				if (inifixExpr[i] != c) {
					//Ищем функцию среди строк
					auto search = operPriority.find(tmp);
					//Пропускаем пробельные символы
					while (std::isspace(inifixExpr[i])) ++i;
					//Если у нас число pi, то проверяем очередность операторов
					if (!tmp.compare("pi")) {
						//Проверяем очередность операций и чисел
						if (!number) number = true;
						else {
							std::cerr << "wrong order: there should be number" << std::endl;
							res.clear(); return res;
						}
						//Формируем строку
						res += std::to_string(M_PI) + " ";
						--i;
						continue ;
					}
					//Если символ - комплексное число
					else if (!tmp.compare("i")) {
						//Проверяем очередность операций и чисел
						if (!number) number = true;
						else {
							std::cerr << "wrong order: there should be number" << std::endl;
							res.clear(); return res;
						}
						//Формируем строку
						res += tmp + " ";
						--i;
						continue ;
					}
					//Если функция присутствует и следующий символ - октрывающая скобка
					//то мы функцию заносим в стэк операторов, в противном случае
					//выводится ошибка и возвращается пустая строка
					else if (search != operPriority.end() && inifixExpr[i] == '(') {
						oper.push(tmp); --i; continue ;
					} else {
						std::cerr << "error: there should be open brace after function name" << std::endl;
						res.clear(); return res;
					}
				}
				//Проверка является ли оператор унарным минусом
				if (!number && inifixExpr[i] == '-' && !std::isspace(inifixExpr[i + 1])) {
					number = true;
					inifixExpr[i] = '~';
				}
				tmp.push_back(inifixExpr[i]);
				//Проверка на то, что перед нами не число
				if (number) number = false;
				else {
					std::cerr << "wrong order: there should be sign" << std::endl;
					res.clear(); return res;
				}
				//Заносим в выходную строку все операторы стэка имеющие более высокий приоритет
				while (!oper.empty() && operPriority[oper.top()] >= operPriority[tmp]) {
					res += oper.top(); oper.pop(); res.push_back(' ');
				}
				//заносим в стэк оператор
				oper.push(tmp);
			}
			//пропускаем пробельные символы
			while (std::isspace(inifixExpr[i + 1])) ++i;
		}
		//заносим оставшиеся операторы в строку
		while (!oper.empty()) {
			res += oper.top();
			oper.pop();
			if (!oper.empty()) res.push_back(' ');
		}
		return res;
	}

	//Removing trailing zeros in string
	std::string RemoveTrailZeros(const std::string &str) {
		std::string res = str;

		if (res.find('.') != std::string::npos) {
			//Removing trailing zeros
			res = res.substr(0, str.find_last_not_of('0') + 1);
			//If the last character is point, remove it
			if (res.find('.') == res.size() - 1) res.resize(res.size() - 1);
		}
		return res;
	}
public:
	RevPolNotation() = delete;
	RevPolNotation(std::string &&init_expr) {
		operPriority["("] = 0;
		operPriority["+"] = 1;
		operPriority["-"] = 1;
		operPriority["*"] = 2;
		operPriority["/"] = 2;
		operPriority["^"] = 3;
		operPriority["sin"] = 4;
		operPriority["cos"] = 4;
		operPriority["tan"] = 4;
		operPriority["exp"] = 4;
		operPriority["sqrt"] = 4;
		operPriority["~"] = 5;

		inifixExpr = std::move(init_expr);
		postfixExpr = ProcessPostfix();
	}
	const std::string	&getInifixExpr() { return inifixExpr; }
	const std::string	&getPosfixExpr() { return postfixExpr; }
	void				setInfixExpr(std::string &&move) {
		inifixExpr.clear(); postfixExpr.clear();
		inifixExpr = std::move(move);
		postfixExpr = ProcessPostfix();
	}



	std::string CalcIt() {
		//Стэк чисел
		std::stack<std::string>		nums;
		//Результат
		std::string					res;
		//Строка состоящая из бинарных операторов
		std::string				opers("+ / - * ^ ~");
		//Список функций
		std::set<std::string> funcs = { "sin", "cos", "tan", "exp", "sqrt" };
		//Парсер-каретка
		std::string parser;

		//Проходим по строке
		for (int i = 0; i < postfixExpr.length(); ++i) {
			//Берем значение символа
			//char c = postfixExpr[i];
			parser.push_back(postfixExpr[i]);
			//Проверяем на то число это или нет
			if (std::isdigit(postfixExpr[i]) || postfixExpr[i] == '.') {
				parser.clear();
				std::string number = GetStringNumber(postfixExpr, i);
				//Заносим в стэк преобразованную в Double строку
				nums.push(number);
			}
			//проверяем на то комплексное число или нет
			else if (!parser.compare("i") && !std::isalpha(postfixExpr[i + 1])) {
				nums.push(parser);
				parser.clear();
			}
			// если строка есть в спике функций
			else if (auto search = funcs.find(parser); search != funcs.end()) {
				//Проверяем пуст ли стэк, если да то задаем нулевое значение
				//если нет, то выталкиваем значение из стэка
				std::string tmp = (nums.empty()) ? 0 : nums.top();
				if (!nums.empty()) nums.pop();
				nums.push(RemoveTrailZeros(funcExecute(parser, tmp)));
				parser.clear();
			}
			// Если символ есть в списке операторов
			else if (opers.find(parser) != std::string::npos) {
				//Проверяем, является ли данный оператор унарным
				if (!parser.compare("~")) {
					//Проверяем пуст ли стэк, если да, то задаем нулевое значение
					//если нет, то выталкиваем значение из стэка
					std::string tmp = (nums.empty()) ? "" : nums.top();
					if (!nums.empty()) nums.pop();
					//Вставляем новое значение в стэк
					nums.push(RemoveTrailZeros(Execute("-", "0", tmp)));
					//Переходим к следующей итерации цикла
					parser.clear();
					while (std::isspace(postfixExpr[i + 1])) ++i;
					continue ;
				}

				//Обозначем переменные для подсчета
				std::string first, second;
				//Получаем значение стэка в обратном порядке
				if (!nums.empty()) { second = nums.top(); nums.pop(); }
				else second = "";

				if (!nums.empty()) { first = nums.top(); nums.pop(); }
				else first = "";
				//Получаем результат операций и заносим в стэк
				nums.push(RemoveTrailZeros(Execute(parser, first, second)));
				parser.clear();
			}
			//пропускаем пробельные символы
			while (std::isspace(postfixExpr[i + 1])) ++i;
		}
		res = nums.top();
		nums.pop();
		return res;
	}
};


int main(void) {
	//std::cout << "Kekw, Nigga!" << std::endl;
	// return 0;

	//1 + 2 -> 1 2 +
	std::string test_1 = "1 + 2";
	RevPolNotation revpol(std::move(test_1));
	std::cout << "Test 1: " << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl;
	std::cout << "Operation result: " << revpol.CalcIt() << std::endl << std::endl;

	//1 + 2 - 3 -> 1 2 + 3 -
	test_1 = "1 + 2 - 3";
	revpol.setInfixExpr(std::move(test_1));
	std::cout << "Test 2: " << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl;
	std::cout << "Operation result: " << revpol.CalcIt() << std::endl << std::endl;

	//1 * 2 / 3 -> 1 2 * 3 /
	test_1 = "1 * 2 / 3";
	revpol.setInfixExpr(std::move(test_1));
	std::cout << "Test 3: " << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl;
	std::cout << "Operation result: " << revpol.CalcIt() << std::endl << std::endl;

	//1 + 2 * 3 -> 1 2 3 * +
	test_1 = "1 + 2 * 3";
	revpol.setInfixExpr(std::move(test_1));
	std::cout << "Test 4: " << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl;
	std::cout << "Operation result: " << revpol.CalcIt() << std::endl << std::endl;

	//1 * 2 + 3 -> 1 2 * 3 +
	test_1 = "1 * 2 + 3";
	revpol.setInfixExpr(std::move(test_1));
	std::cout << "Test 5: " << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl;
	std::cout << "Operation result: " << revpol.CalcIt() << std::endl << std::endl;

	//1 * (2 + 3) -> 1 2 3 + *
	test_1 = "1 * (2 + 3)";
	revpol.setInfixExpr(std::move(test_1));
	std::cout << "Test 6: " << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl;
	std::cout << "Operation result: " << revpol.CalcIt() << std::endl << std::endl;

	//(1 + 2) * (3 - 4) -> 1 2 + 3 4 - *
	test_1 = "(1 + 2) * (3 - 4)";
	revpol.setInfixExpr(std::move(test_1));
	std::cout << "Test 7: " << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl;
	std::cout << "Operation result: " << revpol.CalcIt() << std::endl << std::endl;

	//((1 + 2) * 3) - 4 -> 1 2 + 3 * 4 -
	revpol.setInfixExpr("((1 + 2) * 3) - 4");
	std::cout << "Test 8: " << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl;
	std::cout << "Operation result: " << revpol.CalcIt() << std::endl << std::endl;

	//1 + 2 * (3 - 4 / (5 + 6)) -> 1 2 3 4 5 6 + / - * +
	revpol.setInfixExpr("1 + 2 * (3 - 4 / (5 + 6))");
	std::cout << "Test 9: " << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl;
	std::cout << "Operation result: " << revpol.CalcIt() << std::endl << std::endl;

	revpol.setInfixExpr("-2.345 + 6.876");
	std::cout << "Test 10: " << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl;
	std::cout << "Operation result: " << revpol.CalcIt() << std::endl << std::endl;

	revpol.setInfixExpr("15/(7-(1+1))*3-(2+(1+1))*15/(7-(200+1))*3");//-(2+(1+1))+15/(7-(1+1))*3-(2+(1+1)))");
	std::cout << "Test 11: " << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl;
	std::cout << "Operation result: " << revpol.CalcIt() << std::endl << std::endl;

	revpol.setInfixExpr("15/(7-(1+1))*3-(2+(1+1))*15/(7-(200+1))*3-(2+(1+1))");//+15/(7-(1+1))*3-(2+(1+1)))");
	std::cout << "Test 12:" << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl;
	std::cout << "Operation result: " << revpol.CalcIt() << std::endl << std::endl;

	revpol.setInfixExpr("15/(7-(1+1))*3-(2+(1+1))*15/(7-(200+1))*3-(2+(1+1))+15/(7-(1+1))*3");//-(2+(1+1)))");
	std::cout << "Test 13:" << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl;
	std::cout << "Operation result: " << revpol.CalcIt() << std::endl << std::endl;

	revpol.setInfixExpr("15/(7-(1+1))*3-(2+(1+1))*15/(7-(200+1))*3-(2+(1+1))*(15/(7-(1+1))*3-(2+(1+1))+15/(7-(1+1))*3-(2+(1+1)))");
	std::cout << "Test 13:" << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl;
	std::cout << "Operation result: " << revpol.CalcIt() << std::endl << std::endl;

	revpol.setInfixExpr("pi");
	std::cout << "Test 14:" << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl;
	std::cout << "Operation result: " << revpol.CalcIt() << std::endl << std::endl;

	revpol.setInfixExpr("sin(pi)");
	std::cout << "Test 15:" << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl;
	std::cout << "Operation result: " << revpol.CalcIt() << std::endl << std::endl;

	revpol.setInfixExpr("sin(pi / 2)");
	std::cout << "Test 16:" << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl;
	std::cout << "Operation result: " << revpol.CalcIt() << std::endl << std::endl;

	revpol.setInfixExpr("1 + 2 * sin(pi / 2) + 3 * sqrt(3^2 + 4^2) + cos(pi) + 5");
	std::cout << "Test 16:" << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl;
	std::cout << "Operation result: " << revpol.CalcIt() << std::endl << std::endl;

	revpol.setInfixExpr("i");
	std::cout << "Test 17:" << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl;
	std::cout << "Operation result: " << revpol.CalcIt() << std::endl << std::endl;

	revpol.setInfixExpr("2 * i + i * (3 * (3 + 2)) + 1 + 9 + 11");
	std::cout << "Test 18:" << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl;
	//std::cout << "Operation result: " << revpol.CalcIt() << std::endl << std::endl;

	//std::string test = "52.38428dsfaeafsfz23123123";
	//std::cout << test.substr(0, test.find_first_not_of("0123456789.")) << std::endl;
	return 0;
}


