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
	//std::cout << "Kekw, Nigga!" << std::endl;
	// return 0;

	RevPolTest("1 + 2", 1);
	RevPolTest("1 + 2 - 3", 2);
	RevPolTest("1 * 2 / 3", 3);
	RevPolTest("1 + 2 * 3", 4);
	RevPolTest("1 * 2 + 3", 5);
	RevPolTest("1 * (2 + 3)", 6);
	RevPolTest("(1 + 2) * (3 - 4)", 7);
	RevPolTest("((1 + 2) * 3) - 4", 8);
	RevPolTest("1 + 2 * (3 - 4 / (5 + 6))", 9);
	RevPolTest("-2.345 + 6.876", 10);
	RevPolTest("15/(7-(1+1))*3-(2+(1+1))*15/(7-(200+1))*3", 11);
	RevPolTest("15/(7-(1+1))*3-(2+(1+1))*15/(7-(200+1))*3-(2+(1+1))", 12);
	RevPolTest("15/(7-(1+1))*3-(2+(1+1))*15/(7-(200+1))*3-(2+(1+1))+15/(7-(1+1))*3", 13);
	RevPolTest("15/(7-(1+1))*3-(2+(1+1))*15/(7-(200+1))*3-(2+(1+1))*(15/(7-(1+1))*3-(2+(1+1))+15/(7-(1+1))*3-(2+(1+1)))", 14);
	RevPolTest("pi", 15);
	RevPolTest("sin(pi)", 16);
	RevPolTest("sin(pi / 2)", 17);
	RevPolTest("1 + 2 * sin(pi / 2) + 3 * sqrt(3^2 + 4^2) + cos(pi) + 5", 18);
	RevPolTest("i", 19);
	RevPolTest("2 * i + i * (3 * (3 + 2)) + 1 + 9 * 11 + 12 * 24^2", 20);
	RevPolTest("i * i", 21);
	RevPolTest("i ^ 2 * i ^ 2", 22);
	RevPolTest("i ^ (-1) * i", 23);
	RevPolTest("i / i", 24);
	RevPolTest("5 * i ^ 3 / (2 * i)", 25);
	RevPolTest("5 * i ^ 3 / 2 * i", 26);
	RevPolTest("(i + 1) * (i + 1)", 27);
	RevPolTest("(i + 1) * (i + 1)", 28);
	RevPolTest("(i - 1) * (i + 1)", 29);
	RevPolTest("(i + 1) ^ 2 * (i + 1)", 30);
	RevPolTest("(i + 1) * (i + 1) * (i + 1)", 31);
	RevPolTest("sin(i) + sin(i) + i ^ 2 + i ^ 2", 32);
	RevPolTest("sin(i) - sin(i) + i ^ 2 + i ^ 2", 33);
	RevPolTest("1 / 0", 34);
	RevPolTest("tan(pi/2)", 35);
	RevPolTest("tan(3 * pi / 2)", 36);
	RevPolTest("tan(3pi/2) + 5i + 7", 37);
	RevPolTest("tan(3pi/4) + 5i + 7", 38);
	RevPolTest("5%4", 39);
	RevPolTest("-5%4", 40);
	RevPolTest("2 + 4 *2 - 5 %4 + 2 * (4 + 5)", 41);
	RevPolTest("4 % 2", 42);
	RevPolTest("sin(pi)cos(pi)", 43);
	RevPolTest("sin()cos()", 44);
	RevPolTest("sin(-)", 45);

	//Testing equation proc class
	EquationProc proc;
	//Testing variable declaration functionality
	proc.AddEquation("> varA = 2");
	proc.AddEquation("> varB = 4.242");
	proc.AddEquation("> varC = -4.3");
	proc.AddEquation("> varA = 2*i + 3");
	proc.AddEquation("> varB = -4i - 4");
	proc.AddEquation("> x = 2");
	proc.AddEquation("> y = x");
	proc.AddEquation("> varA = 2 + 4 *2 - 5 %4 + 2 * (4 + 5)");
	proc.AddEquation("> varB = 2 * varA - 5 %4");
	proc.AddEquation("> varC = 2 * varA - varB");

	//Testing function declaration functionality
	proc.AddEquation("> funA(x) = 2*x^5 + 4x^2 - 5*x + 4");
	proc.AddEquation("> funB(y) = 43 * y / (4 % 2 * y)");
	proc.AddEquation("> funC(z) = -2 * z - 5");
	proc.AddEquation("> a = 2 * 4 + 4");
	proc.AddEquation("> funA(x) = 2 * 4 + x");
	proc.AddEquation("> funB(x) = 4 -5 + (x + 2)^2 - 4");
	proc.AddEquation("> funC(x) = 4x + 5 - 2");

	//Testing computational part of program
	proc.AddEquation("> a + 2 = ?");
	proc.AddEquation("> funA(2) + funB(4) = ?");
	proc.AddEquation("> funC(3) = ?");

	//Testing variables with functions
	proc.AddEquation("> varD = funA(varC)");

	//Testing binary equation
	proc.AddEquation("> funA(x) = x^2 + 2x + 1");
	proc.AddEquation("> y = 0");
	proc.AddEquation("> funA(z) = y ?");

	//Testing matrix equations
	proc.AddEquation("> varA = [[2,3];[4,3]]");
	proc.AddEquation("> varE = [ [ 3 , 4 ] ]");
	proc.AddEquation("> varF = []");
	proc.AddEquation("> varF = [[[[[");
	proc.AddEquation("> varF = ]]]]]");
	proc.AddEquation("> varF = [ [ 3 , 4 ] [ 5 ]]");
	proc.AddEquation("> varF = [ [ 3 , 4 ]    ; [ 5 ]]");
	proc.AddEquation("> varF = [ [ -1, -2, 1 ] ; [ 5, 9, -8 ]] + [[10,-25,98];[3,0,-14]]");
	proc.AddEquation("> varG = [ [ -1, -2, 1 ] ; [ 5, 9, -8 ]]");
	proc.AddEquation("> varH = [[10,-25,98];[3,0,-14]]");
	proc.AddEquation("> varI = varG - varH");
	proc.AddEquation("> varJ = [[-1, -2, 7] ; [4, 9, 0]]");
	proc.AddEquation("> 3 * varJ = ?");
	proc.AddEquation("> -5 * varJ = ?");
	proc.AddEquation("> -varJ = ?");
	proc.AddEquation("> varK = [[-1, 2, -3, 0];[5,4,-2,1];[-8,11,-10,-5]]");
	proc.AddEquation("> varL = [[-9,3];[6,20];[7,0];[12,-4]]");
	proc.AddEquation("> varK * varL = ?");
	proc.AddEquation("> [[6,3];[-17,-2]] * [[4,9];[-6,90]] = ?");
	proc.AddEquation("> transp([[-1,2,4,0,7];[3,-5,24,9,-3];[-10,-8,-2,-4,11]]) = ?");
	proc.AddEquation("> [[1,2];[-1,-3]] ^ 2 = ?");
	proc.AddEquation("> [[1,2];[-1,-3]] ^ 6 = ?");
	proc.AddEquation("> varJ = [[1, 0, -1, 2];[3,-2,5,0];[-1,4,-3,6]]");
	proc.AddEquation("> varK = [[-9,1,0];[2,-1,4];[0,-2,3];[1,5,0]]");
	proc.AddEquation("> varL = [[-5, -20, 13];[10,12,9];[3,-15,8]]");
	proc.AddEquation("> 2 * varJ * varK - 3 * transp(varL) + 7 * [[1,0,0];[0,1,0];[0,0,1]] = ?");
	proc.AddEquation("> varM = lonenorm([[1,0,-1,2,3,2,5,0,-1,4,-3,6]])");
	proc.AddEquation("> varM = ltwonorm([[1,0,-1,2,3,2,5,0,-1,4,-3,6]])");
	proc.AddEquation("> varM = det(varL)");
	proc.AddEquation("> inv(varL) = ?");

	//History of operations output
	std::cout << "History of operations:" << std::endl;
	proc.HistoryOutput();
	//Output of all variables
	std::cout << std::endl << "Output of all declared variables:" << std::endl;
	proc.VariablesOutput();

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


