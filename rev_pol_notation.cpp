#include <iostream>
#include <string>
#include <map>
#include <cctype>
#include <stack>
#include <cmath>

class RevPolNotation {
	std::string			inifixExpr;
	std::string			postfixExpr;
	std::map<char, int>	operPriority;

	std::string			GetStringNumber(int &i) {
		std::string 	res;

		for (; i < inifixExpr.length(); ++i) {
			char	tmp;
			int		dot = 0;

			tmp = inifixExpr[i];
			if (isdigit(tmp)) res.push_back(tmp);
			else if (tmp == '.' && !dot) { res.push_back(tmp); ++dot; }
			else if (tmp == '.' && dot) { res.clear(); return res; }
			else break ;
		}
		//Сдвигаем каретку
		--i;
		return res;
	}

	//Приватный метод для выполнения операторов
	double Execute(const char &oper, const double &f, const double &s) {

		switch(oper) {
		case '+': return f + s;
			break ;
		case '-': return f - s;
			break ;
		case '*': return f * s;
			break ;
		case '/': return f / s;
			break ;
		case '^': return std::pow(f, s);
			break ;
		default: return 0;
			break ;
		}
	}

	std::string		ProcessPostfix() {
		//Результирующая строка
		std::string			res;
		//Стэк операторов в виде символов
		std::stack<char>	oper;
		//контролируем скобки
		int 				brace = 0;
		//контролируем очередность данных
		bool				number = false;
		//Перебираем строку
		for (int i = 0; i < inifixExpr.size(); ++i) {
			//Если символ - число
			if (std::isdigit(inifixExpr[i])) {
				if (!number) number = true;
				else {
					std::cerr << "wrong order: there should be number" << std::endl;
					res.clear(); return res;
				}
				res += GetStringNumber(i) + " ";
				if (res.empty()) {
					std::cerr << "wrong number syntax" << std::endl;
					res.clear(); return res;
				}
			}
			//Если символ - октрывающаяся скобка
			else if (inifixExpr[i] == '(') {
				//заносим в стэк и сигнализируем об открытых скобках
				oper.push(inifixExpr[i]); ++brace;
			}
			//Проверяем случай неправильной закрывающей скобки
			else if (inifixExpr[i] == ')' && !brace) {
				std::cerr << "extra spaces found" << std::endl;
				res.clear(); return res;
			}
			//Проверяем случай правильной закрывающей скобки
			else if (inifixExpr[i] == ')') {
				//Заносим в выходную строку из стэка все вплоть до открывающейся скобки
				while (!oper.empty() && oper.top() != '(') {
					res.push_back(oper.top()); oper.pop(); res.push_back(' ');
				}
				oper.pop();
			}
			//проверяем содержится ли символ в списке операторов
			else if (auto search = operPriority.find(inifixExpr[i]); search != operPriority.end()) {
				//если да, то сначала проверяем
				if (number) number = false;
				else {
					std::cerr << "wrong order: there should be sign" << std::endl;
					res.clear(); return res;
				}
				char	tmp = inifixExpr[i];
				//Является ли оператор унарным минусом
				if (tmp == '-' && !std::isspace(inifixExpr[i + 1]))
					//если да - то заменить его на тильду
					inifixExpr[i] = '~';
				//Заносим в выходную строку все операторы стэка имеющие более высокий приоритет
				while (!oper.empty() && operPriority[oper.top()] >= operPriority[tmp]) {
					res.push_back(oper.top()); oper.pop(); res.push_back(' ');
				}
				//заносим в стэк оператор
				oper.push(tmp);
			}
			//пропускаем пробельные символы
			while (std::isspace(inifixExpr[i + 1])) ++i;
		}
		//заносим оставшиеся операторы в строку
		while (!oper.empty()) {
			res.push_back(oper.top());
			oper.pop();
			if (!oper.empty()) res.push_back(' ');
		}
		return res;
	}
public:
	RevPolNotation() = delete;
	RevPolNotation(std::string &&init_expr) {
		operPriority['('] = 0;
		operPriority['+'] = 1;
		operPriority['-'] = 1;
		operPriority['*'] = 2;
		operPriority['/'] = 2;
		operPriority['^'] = 3;
		operPriority['~'] = 4;

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
		std::stack<double> nums;

		//Проходим по строке
		for (int i = 0; i < postfixExpr.length(); ++i) {
			//Берем значение символа
			char c = postfixExpr[i];
			//Проверяем на то номер это или нет
			if (std::isdigit(c) || c == '.') {
				std::string number = GetStringNumber(i);
				//Заносим в стэк преобразованную в Double строку
				nums.push(std::stod(number));
			}
			// Если символ есть в списке операторов
			else if (auto search = operPriority.find(c); search != operPriority.end()) {
				//Проверяем, является ли данный оператор унарным

			}
		}
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
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl << std::endl;

	//1 + 2 - 3 -> 1 2 + 3 -
	test_1 = "1 + 2 - 3";
	revpol.setInfixExpr(std::move(test_1));
	std::cout << "Test 2: " << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl << std::endl;

	//1 * 2 / 3 -> 1 2 * 3 /
	test_1 = "1 * 2 / 3";
	revpol.setInfixExpr(std::move(test_1));
	std::cout << "Test 3: " << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl << std::endl;

	//1 + 2 * 3 -> 1 2 3 * +
	test_1 = "1 + 2 * 3";
	revpol.setInfixExpr(std::move(test_1));
	std::cout << "Test 4: " << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl << std::endl;

	//1 * 2 + 3 -> 1 2 * 3 +
	test_1 = "1 * 2 + 3";
	revpol.setInfixExpr(std::move(test_1));
	std::cout << "Test 5: " << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl << std::endl;

	//1 * (2 + 3) -> 1 2 3 + *
	test_1 = "1 * (2 + 3)";
	revpol.setInfixExpr(std::move(test_1));
	std::cout << "Test 6: " << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl << std::endl;

	//(1 + 2) * (3 - 4) -> 1 2 + 3 4 - *
	test_1 = "(1 + 2) * (3 - 4)";
	revpol.setInfixExpr(std::move(test_1));
	std::cout << "Test 7: " << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl << std::endl;

	//((1 + 2) * 3) - 4 -> 1 2 + 3 * 4 -
	revpol.setInfixExpr("((1 + 2) * 3) - 4");
	std::cout << "Test 8: " << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl << std::endl;

	//1 + 2 * (3 - 4 / (5 + 6)) -> 1 2 3 4 5 6 + / - * +
	revpol.setInfixExpr("1 + 2 * (3 - 4 / (5 + 6))");
	std::cout << "Test 9: " << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl << std::endl;
	return 0;
}


