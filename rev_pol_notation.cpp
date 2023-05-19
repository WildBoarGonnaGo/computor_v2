#include "EquationProc.h"
#include <iostream>
#include <cstring>

void RevPolTest(std::string &&test, int num) {
	std::map<std::string, Func> plug;
	RevPolNotation revpol(std::move(test), plug);

	std::cout << "Test " << num << ":" << std::endl;
	std::cout << "Infix notation: " << revpol.getInifixExpr() << std::endl;
	std::cout << "Postfix notation: " << revpol.getPosfixExpr() << std::endl;
	if (revpol.getPosfixExpr().empty()) std::cout << revpol.getErrMsg() << std::endl << std::endl;
	else std::cout << "Operation result: " << revpol.CalcIt() << std::endl << std::endl;
}

int main(void) {
	//Procedure manager
	EquationProc proc;

	std::cout << "Computor (v2) by lchantel. Enjoy..." << std::endl;
	 while (true) {
     	//Input string. To exit function type 'quit' or 'exit'
     	std::string input;
     	//input index iterator
     	int i = 0;

     	std::cout << "> ";
     	std::getline(std::cin, input);
     	if (input.empty()) continue ;
     	if (input == "quit" || input == "exit") break ;
     	while (std::isspace(input[i])) ++i;
     	//To output history, input 'history' command
     	if (!input.compare(i, std::strlen("history"), "history")) {
			//Check if there is obscure character after
			//'history' command. If there is, return error
			i += std::strlen("history");
			while (std::isspace(input[i])) ++i;
			proc.PushToHistory(input);
			if (i != input.size()) {
				//error message
				std::string error = "error: there is some obscure character after 'history' command";
				proc.PushToHistory(error);
				std::cerr << error << std::endl;
			}
     		proc.HistoryOutput();
     	}
     	//To output variables, input 'variables' command
        else if (!input.compare(i, std::strlen("variables"), "variables")) {
			//Check if there is obscure character after
			//'variables' command. If there is, return error
			i += std::strlen("variables");
			while (std::isspace(input[i])) ++i;
			proc.PushToHistory(input);
			if (i != input.size()) {
				//error message
				std::string error = "error: there is some obscure character after 'variables' command";
				proc.PushToHistory(error);
				std::cerr << error << std::endl;
			}
        	proc.VariablesOutput();
        }
        //To plot some function, input 'plot(<some_expression_here>)'
		else if (!input.compare(i, 4, "plot")) proc.PlotFunction(input);
		//Or else we make preview of some equation
		else proc.AddEquation(std::move(input));
	 }

	return 0;
}
