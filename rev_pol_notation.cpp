#include <iostream>
#include <string>
#include <map>
#include <cctype>
#include <stack>

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
		return res;
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
					res += oper.top() + ' '; oper.pop();
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
					res += oper.top() + ' '; oper.pop();
				}
				//заносим в стэк оператор
				oper.push(tmp);
			}
			//пропускаем пробельные символы
			while (std::isspace(inifixExpr[i + 1])) ++i;
		}
		//заносим оставшиеся операторы в строку
		while (!oper.empty()) { res += oper.top() + ' '; oper.pop(); }
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
};


int main(void) {
	std::cout << "Kekw, Nigga!" << std::endl;
	return 0;

	std::string test_1 = "1 + 2";

}


