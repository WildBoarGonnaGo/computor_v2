#include <iostream>
#include <string>
#include <map>
#include <cctype>
#include <stack>
#include <cmath>
#include <vector>

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
	double Execute(const std::string &oper, const double &f, const double &s) {

		if (!oper.compare("+")) return f + s;
		else if (!oper.compare("-")) return f - s;
		else if (!oper.compare("*")) return f * s;
		else if (!oper.compare("/")) return f / s;
		else if (!oper.compare("^")) return std::pow(f, s);
		else return 0;
	}

	double funcExecute(const std::string &oper, const double &var) {
		if (!oper.compare("sin")) return std::sin(var);
		else if (!oper.compare("cos")) return std::cos(var);
		else if (!oper.compare("tan")) return std::tan(var);
		else if (!oper.compare("exp")) return std::exp(var);
		else if (!oper.compare("sqrt")) return std::sqrt(var);
		return 0;
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
					if (!number && !tmp.compare("pi")) {
						number = true;
						res += std::to_string(M_PI) + " ";
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
				else tmp.push_back(c);
				//Проверка является ли оператор унарным минусом
				if (!number && !tmp.compare("-") && !std::isspace(inifixExpr[i + 1])) {
					number = true;
					inifixExpr[i] = '~';
				}
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

	double CalcIt() {
		//Стэк чисел
		std::stack<double>		nums;
		//Результат
		double					res;
		//Строка состоящая из бинарных операторов
		std::string				opers("+ / - * ^");
		//Список функций
		std::vector<std::string> funcs = { "sin", "cos", "tan", "exp", "sqrt" };
		//Парсер-каретка
		std::string parser;

		//Проходим по строке
		for (int i = 0; i < postfixExpr.length(); ++i) {
			//Берем значение символа
			//char c = postfixExpr[i];
			parser.push_back(postfixExpr[i]);
			//Проверяем на то номер это или нет
			if (std::isdigit(postfixExpr[i]) || postfixExpr[i] == '.') {
				parser.clear();
				std::string number = GetStringNumber(postfixExpr, i);
				//Заносим в стэк преобразованную в Double строку
				nums.push(std::stod(number));
			}
			// если строка есть в спике функций
			else if (funcs.find(parser) != std::string::npos) {
				//Проверяем пуст ли стэк, если да то задаем нулевое значение
				//если нет, то выталкиваем значение из стэка
				double tmp = (nums.empty()) ? 0 : nums.top();
				if (!nums.empty()) nums.pop();
				nums.push(funcExecute(parser, tmp));
				parser.clear();
			}
			// Если символ есть в списке операторов
			else if (opers.find(parser) != std::string::npos) {
				//Проверяем, является ли данный оператор унарным
				if (!parser.compare("~")) {
					//Проверяем пуст ли стэк, если да, то задаем нулевое значение
					//если нет, то выталкиваем значение из стэка
					double tmp = (nums.empty()) ? 0 : nums.top();
					if (!nums.empty()) nums.pop();
					//Вставляем новое значение в стэк
					nums.push(Execute("-", 0, tmp));
					//Переходим к следующей итерации цикла
					parser.clear();
					continue ;
				}

				//Обозначем переменные для подсчета
				double first, second;
				//Получаем значение стэка в обратном порядке
				if (!nums.empty()) { second = nums.top(); nums.pop(); }
				else second = 0;

				if (!nums.empty()) { first = nums.top(); nums.pop(); }
				else first = 0;
				//Получаем результат операций и заносим в стэк
				nums.push(Execute(parser, first, second));
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
	std::cout << "Test 14:" << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl;
	std::cout << "Operation result: " << revpol.CalcIt() << std::endl << std::endl;
	return 0;
}


