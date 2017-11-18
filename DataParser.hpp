#ifndef DataParser_HPP
#define DataParser_HPP

#include <iostream>
#include <fstream>
#include <vector>
#include <queue>

class DataParser {
	private:
		bool debug;
		std::ifstream fs;

		//gets next whitespace separated string not in a comment
		std::string gitGudWord();

		bool isNum(std::string s);
		std::string doubleToStr(double num);
		int discretizeAttribute(int attrIdx);

		//just for debugging
		void printTable();

	public:
		int n_attributes;
		std::vector<std::string> attributeNames;
		std::string decisionName;
		std::vector<std::vector<std::string>> dataTable;
		std::vector<std::vector<std::string>> attributeValues; //grouped in same order as attributeNames
		std::vector<std::string> conceptNames;
		std::vector<std::vector<int>> conceptCases; //grouped in same order as conceptNames

		DataParser(std::string fileName, bool debug);
		~DataParser();

		std::vector<std::string> parseRow();

		void buildTable();
		void discretizeData();

};

#endif /* DataParser_HPP */
