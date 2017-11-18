#include "DataParser.hpp"

//Private functions

//gets next whitespace separated string not in a comment
std::string DataParser::gitGudWord(){
	std::string buf;
	while (true){
		fs >> buf;
		if(fs.eof()){
			throw std::string("Cannot get next line, EOF.");
			return "";
		}
		else if (buf[0] == '!')
			getline(fs, buf);
		else
			return buf;
	}
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

//trim result from std::to_string
std::string DataParser::doubleToStr(double num){ //!seg faulting on some cases // no longer seg faulting, but seems to hang for a little bit at random intervals
	std::string str = std::to_string(num);
	//std::cout << "\n" << str << "\n";

	unsigned int trimPos;
	for (trimPos = str.length()-1; trimPos > 0; trimPos--)
		if(str[trimPos] != '0' && str[trimPos] != '.')
			break;

	return str.substr(0, trimPos+1);
}

int DataParser::discretizeAttribute(int attrIdx){
	if (debug) std::cout << "Attribute " << attributeNames[attrIdx] << " flagged for discretization\n";

	//store attibute column
	std::vector<double> attrColumn(dataTable.size());
	for (unsigned int i = 0; i < dataTable.size(); i++)
		attrColumn[i] = std::stod(dataTable[i][attrIdx]);

	//find cutpoints
	std::priority_queue<double, std::vector<double>, std::greater<double>> min_q;
	for (unsigned int i = 0; i < attributeValues[attrIdx].size(); i++)
		min_q.push(std::stod(attributeValues[attrIdx][i]));

	//std::vector<double> cutpoints(min_q.size()-1);
	std::vector<double> cutpoints;
	double lowest, highest;

	lowest = min_q.top();
	double prevValue = lowest;
	min_q.pop();
	while(!min_q.empty()){
		cutpoints.push_back((prevValue + min_q.top())/2);
		prevValue = min_q.top();
		min_q.pop();
	}
	highest = prevValue;

	if (debug){ //debug print
		std::cout << "cutpoints: ";
		for (unsigned int i = 0; i < cutpoints.size(); i++)
			std::cout << cutpoints[i] << " ";
		std::cout << std::endl;
	}

	//NOTE: number of attributes = cutpoints.size()

	//expand and overwrite attributeNames
	std::string oldName = attributeNames[attrIdx];
	attributeNames.insert(attributeNames.begin()+attrIdx+1, cutpoints.size()-1, "");
	for (unsigned int i = 0; i < cutpoints.size(); i++)
		attributeNames[attrIdx+i] = oldName + "_" + doubleToStr(cutpoints[i]);

	//expand and overwrite attributeValues
	attributeValues.insert(attributeValues.begin()+attrIdx+1, cutpoints.size()-1, std::vector<std::string>());
	for (unsigned int i = 0; i < cutpoints.size(); i++){
		std::vector<std::string> vec(2);
		vec[0] = doubleToStr(lowest) + ".." + doubleToStr(cutpoints[i]);
		vec[1] = doubleToStr(cutpoints[i]) + ".." + doubleToStr(highest);
		attributeValues[attrIdx+i] = vec;
	}

	//expand and overwrite dataTable
	for (unsigned int i = 0; i < dataTable.size(); i++){
		dataTable[i].insert(dataTable[i].begin()+attrIdx+1, cutpoints.size()-1, "");
		for (unsigned int j = 0; j < cutpoints.size(); j++){
			if (attrColumn[i] < cutpoints[j])
				dataTable[i][attrIdx+j] = doubleToStr(lowest) + ".." + doubleToStr(cutpoints[j]);
			else
				dataTable[i][attrIdx+j] = doubleToStr(cutpoints[j]) + ".." + doubleToStr(highest);
		}
	}

	n_attributes += cutpoints.size()-1;
	return cutpoints.size()-1;
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

	std::string buf = "";
	while (buf != "[") {fs >> buf;} //move fs to beginning of datatable

	std::vector<std::string> nameVector;
	buf = gitGudWord();
	while (buf != "]"){
		nameVector.push_back(buf);
		buf = gitGudWord();
	};

	n_attributes = (signed)nameVector.size() -1;
	attributeValues.resize(n_attributes);
	attributeNames.resize(n_attributes);
	for (int i = 0; i < n_attributes; i++)
		attributeNames[i] = nameVector[i];
	decisionName = nameVector[n_attributes];

	this->debug = debug;
}


DataParser::~DataParser(){
	//close file
	if(fs.is_open())
		fs.close();
}


//returns empty vector if eof
std::vector<std::string> DataParser::parseRow(){
	std::vector<std::string> row(n_attributes+1);
	for (int i = 0; i < n_attributes+1; i++){
		try{
			row[i] = gitGudWord();
		} catch (std::string e) {
			return std::vector<std::string>(0);  //empty vector
		}
	}
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
		std::cout << "Discretized data table:\n";
		printTable();
	}
}
