#include "DataParser.hpp"

//Private functions
std::string DataParser::gitGudLine(){
	std::string currentLine;
	do{
		getline(fs, currentLine);
		if(fs.eof())
			throw std::string("Cannot get next line, EOF.");
	} while (currentLine[0] == '!');
	return currentLine;
}

//returns a substring of the first sequence of chars separated by whitespace, modifies line to remove returned substring
//returns empty string if no word in string
std::string DataParser::gitGudWord(std::string &line){
	bool found = false;
	int beginPos;
	for(unsigned int i = 0; i < line.length(); i++){
		if(!found && !isspace(line[i])){
			found = true;
			beginPos = i;
		}
		else if (found && isspace(line[i])){
			std::string retStr = line.substr(beginPos, i-beginPos);
			line = line.substr(i);
			return retStr;
		}
	}
	if(found){
		std::string retStr = line.substr(beginPos);
		line.clear();
		return retStr;
	}

	line.clear();
	return std::string();
}

void DataParser::printTable(){
	//data table
	std::cout << "dataTable:\n(";
	for(int i = 0; i < n_attributes; i++)
		std::cout << attributeNames[i] << ",";
	std::cout << decisionName << ")\n";
	for(unsigned int i = 0; i < dataTable.size(); i++) {
		std::cout << "|";
		for(int j = 0; j < n_attributes+1; j++)
			std::cout << dataTable[i][j] << "|";
		std::cout << "\n";
	}
	//attributeValues
	std::cout << "attributeValues:\n";
	for(int i = 0; i < n_attributes; i++){
		std::cout << "|";
		for(unsigned int j = 0; j < attributeValues[i].size(); j++)
			std::cout << attributeValues[i][j] << "|";
		std::cout << "\n";
	}

	//conceptCases
	std::cout << "conceptCases:\n";
	for(unsigned int i = 0; i < conceptCases.size(); i++){
		std::cout << "|";
		for(unsigned int j = 0; j < conceptCases[i].size(); j++)
			std::cout << conceptCases[i][j] << "|";
		std::cout << "\n";
	}
}


//Public functions
DataParser::DataParser(std::string fileName){
	//open file
	fs.open(fileName, std::ifstream::in);
	if(!fs){
		throw std::string("Invalid file name.\n");
		return;
	}

	std::string inpStr = gitGudLine();

	//we will just assume first line is a series of 'a's and one d at the end, as per the project inpStructions
	n_attributes = (inpStr.length()-5)/2; //we just need to calc the number of 'a's
	attributeNames = new std::string[n_attributes];

	inpStr = gitGudLine();
	inpStr = inpStr.substr(2, inpStr.length()-4); //trim off brackets
	for(int i = 0; i < n_attributes; i++)
		attributeNames[i] = gitGudWord(inpStr);
	decisionName = gitGudWord(inpStr);

	attributeValues = new std::vector<std::string>[n_attributes];
}


DataParser::~DataParser(){
	//close file
	if(fs.is_open())
		fs.close();

	//delete stuff
	delete[] attributeNames;
	delete[] attributeValues;
	//delete string arrays in vectors
	for(unsigned int i = 0; i < dataTable.size(); i++)
		delete[] dataTable[i];
}


//returns nullpointer if eof
std::string* DataParser::parseRow(){
	std::string inpStr;
	try{
		inpStr = gitGudLine();
	} catch (std::string e) {
		return nullptr;
	}

	std::string* row = new std::string[n_attributes+1];

	for(int i = 0; i < n_attributes; i++){
		row[i] = gitGudWord(inpStr);
		if(row[i].empty()){
			delete[] row;
			return nullptr;
		}
	}
	row[n_attributes] = gitGudWord(inpStr);
	if(row[n_attributes].empty()){
		delete[] row;
		return nullptr;
	}

	return row;
}

//NOTE: technically, if file was extended, this could be re-called to add to dataTable
void DataParser::buildTable(bool debug = false){
	//loop through rows from file read in
	for (std::string* row = parseRow(); row != nullptr; row = parseRow()){
		///add to table
		dataTable.push_back(row);

		//add to list of possible attribute values
		for (int i = 0; i < n_attributes; i++){
			bool exists = false;
			for (unsigned int j = 0; j < attributeValues[i].size(); j++)
				if (row[i] == attributeValues[i][j]){
					exists = true;
					break;
				}
			if (!exists)
				attributeValues[i].push_back(row[i]);
		}

		//update concept values and cases
		unsigned int conceptNum;
		bool exists = false;
		for (conceptNum = 0; conceptNum < conceptNames.size(); conceptNum++){
			if (conceptNames[conceptNum] == row[n_attributes]){
				exists = true;
				break;
			}
		}
		if (!exists){
			conceptNames.push_back(row[n_attributes]);
			//conceptNum++;
			conceptCases.resize(conceptNum+1);
		}
		conceptCases[conceptNum].push_back(dataTable.size()-1);
	}

	/*DEBUG:*/ if(debug) printTable();
}


int DataParser::getNumOfAttributes(){
	return n_attributes;
}
