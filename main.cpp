#include <iostream>
#include <list>
#include <string>
#include <csignal>



int main() {
	//std::cout << "Hello, World!" << std::endl;
	//return 0;

	std::list<std::string>				inputs;
	std::list<std::string>::iterator	it;
	std::list<std::string>::iterator	end;


	inputs.push_back("Kono");
	if (!inputs.empty()) { it = inputs.begin(); end = inputs.begin(); }
	inputs.push_back("Dio");
	++end;
	inputs.push_back("Da");
	++end;


	for ( ; it != inputs.end(); ++it)
		std::cout << *it << ' ';
	std::cout << std::endl;
	for ( ; ; --end) {
		std::cout << *end << ' ';
		if (end == inputs.begin()) break ;
	}

	std::string input_data;

	inputs.clear();
	while (1) {
		std::cout << "> ";
		std::getline(std::cin, input_data);

		std::cout << input_data << std::endl;
	}
	std::cout << std::endl;
	return 0;
}
