#ifndef DataParser_HPP
#define DataParser_HPP

#include <iostream>
#include <fstream>
#include <vector>

class DataParser {
	private:
		std::ifstream fs;

		//gets the next non-comment line, throws an exception on eof
		std::string gitGudLine();

		//returns a substring of the first sequence of chars separated by whitespace, modifies line to remove returned substring
		//returns empty string if no word in string
		std::string gitGudWord(std::string &line);

		//just for debugging
		void printTable();

	public:
		int n_attributes;
		std::string* attributeNames;
		std::string decisionName;
		std::vector<std::string*> dataTable;
		std::vector<std::string>* attributeValues; //grouped in same order as attributeNames
		std::vector<std::string> conceptNames;
		std::vector<std::vector<int>> conceptCases; //grouped in same order as conceptNames

		DataParser(std::string fileName);
		~DataParser();

		std::string* parseRow();

		void buildTable(bool debug);

		int getNumOfAttributes();

};

#endif /* DataParser_HPP */
