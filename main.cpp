#include <iostream>
#include "AQ_Inductor.hpp"

int main(int argc, char* argv[]){
	bool debug = false;
	if (argc > 1){
		std::string arg = std::string(argv[1]);
		if(arg == "debug"){
			std::cout << "Program launched with 'debug' flag. Debug information will log to console.\n";
			debug = true;
		}
	}


	AQ_Inductor* aqi;

	bool error;
	do {
		error = false;
		try{
			std::cout << "Please enter the name for the input data file (including path and extension, e.g. \"data/test.txt\"):\n";
			std::string fileName;
			std::cin >> fileName;

			aqi = new AQ_Inductor(fileName, debug);

		} catch (std::string excep){
			std::cerr << "Error: " << excep << "Please retry:\n\n";
			error = true;
		}
	} while (error);

	do {
		error = false;
		try{
			std::cout << "Please enter the value of MAXSTAR:\n";
			int maxstar;
			std::cin >> maxstar;

			aqi->runAQ(maxstar);
			std::cout << "Done running AQ.\n";
			aqi->writeWithNeg();
			std::cout << "Done writing negated ruleset.\n";
			aqi->writeWithoutNeg();
			std::cout << "Done writing non-negated ruleset.\n";

		} catch (std::string excep){
			std::cerr << "Error: " << excep << "\nPlease retry:\n";
			error = true;
		}
	} while (error);

	std::cout << "Exiting.\n";

	delete aqi;
	return 0;
}
