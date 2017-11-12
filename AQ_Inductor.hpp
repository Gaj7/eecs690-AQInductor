#ifndef AQ_Inductor_HPP
#define AQ_Inductor_HPP

#include <fstream>
#include <vector>
#include "DataParser.hpp"
#include "Star.hpp"

class AQ_Inductor {
	private:
		//Variables
		std::fstream fs;
    std::string fileName;
		bool debug;

    DataParser* dp;
		bool isConsistent = true;

		std::vector<Star> conceptStars;

    //Functions
    std::string stripFileExtenstion(std::string s);
		bool isCovered(int caseIdx, Star star);
		Star calcPartialStar(int posCase, std::vector<int> negCases);
		bool calcConsistency();


	public:
		AQ_Inductor(std::string fileName, bool debug);
		~AQ_Inductor();

    void runAQ(int maxstar);
    void writeWithNeg();
    void writeWithoutNeg();

};

#endif /* AQ_Inductor_HPP */
