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

//std::stod works for extracting doubles, but it doesn't give any feedback as to whether or not the passed string is *exactly* a number with no other extraneous characters. For example, it might read 25 from "25..30" when we don't even want to consider that string a number
bool DataParser::isNum(std::string s){
	bool foundDot = false;

	for (unsigned int i = 0; i < s.length(); i++){
		if (s[i] == '-'){
			if (i != 0)
				return false;
		}
		else if (s[i] == '.'){
			if (foundDot)
				return false;
			else
				foundDot = true;
		}
		else if (s[i] != '0' && s[i] != '1' && s[i] != '2' && s[i] != '3' && s[i] != '4' && s[i] != '5' && s[i] != '6' && s[i] != '7' && s[i] != '8' && s[i] != '9')
			return false;
	}

	return true;
}

int DataParser::discretizeAttribute(int attrIdx){
	if (debug) std::cout << "Attribute " << attributeNames[attrIdx] << " flagged for discretization\n";
	return 0;
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
	std::cout << std::endl;
}


//Public functions
DataParser::DataParser(std::string fileName, bool debug=false){
	//open file
	fs.open(fileName, std::ifstream::in);
	if(!fs){
		throw std::string("Invalid file name.\n");
		return;
	}

	std::string inpStr = gitGudLine();

	//we will just assume first line is a series of 'a's and one d at the end, as per the project inpstructions
	n_attributes = (inpStr.length()-5)/2; //we just need to calc the number of 'a's
	attributeNames.resize(n_attributes);

	inpStr = gitGudLine();
	inpStr = inpStr.substr(2, inpStr.length()-4); //trim off brackets
	for(int i = 0; i < n_attributes; i++)
		attributeNames[i] = gitGudWord(inpStr);
	decisionName = gitGudWord(inpStr);

	attributeValues.resize(n_attributes);
	this->debug = debug;
}


DataParser::~DataParser(){
	//close file
	if(fs.is_open())
		fs.close();
}


//returns empty vector if eof
std::vector<std::string> DataParser::parseRow(){
	std::string inpStr;
	try{
		inpStr = gitGudLine();
	} catch (std::string e) {
		return std::vector<std::string>(0);  //empty vector
	}

	std::vector<std::string> row(n_attributes+1);

	for(int i = 0; i < n_attributes; i++){
		row[i] = gitGudWord(inpStr);
		if(row[i].empty())
			return std::vector<std::string>(0);  //empty vector
	}
	row[n_attributes] = gitGudWord(inpStr);
	if(row[n_attributes].empty())
		return std::vector<std::string>(0);  //empty vector

	return row;
}

//NOTE: technically, if file was extended, this could be re-called to add to dataTable
void DataParser::buildTable(){
	//loop through rows from file read in
	for (std::vector<std::string> row = parseRow(); !row.empty(); row = parseRow()){
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

//NOTE: only checks one value per attribute for discretization. It is assumed that an attribute with numbers for values will have only numbers for values
void DataParser::discretizeData(){
	for (unsigned int i = 0; i < attributeValues.size(); i++){	//note this loop will check attributeValues.size() each iteration, so we can safely loop through even while changing the size
		if (isNum(attributeValues[i][0])){
			i += discretizeAttribute(i); //increments counter according to number of added attributes. No need to check already discretized attributes.
		}
	}

	if (debug){
		std::cout << "Discretized data table\n";
		printTable();
	}
}
