#include "AQ_Inductor.hpp"

//Private functions
std::string AQ_Inductor::stripFileExtenstion(std::string s){
	bool foundDot = false;
	int i;
	for(i = s.length()-1; i >= 0; i--)
		if(s[i] == '.'){
			foundDot = true;
			break;
		}
	if(foundDot)
		return s.substr(0, i);
	else
		return s;
}

/*
	foreach complex in star
		if any complex covers case, return true
		testing if complex covers case:
			assume it covers it
			foreach attribute of case
				foreach selector in the complex
					if attributes match and values match, that means its not covered
*/
bool AQ_Inductor::isCovered(int caseIdx, Star star){
	for (unsigned int i = 0; i < star.complexes.size(); i++){								//foreach complex in star
		bool coveredByComplex = true;
		for (int j = 0; (j < dp->n_attributes) && coveredByComplex; j++){				//foreach attribute of case
			for (unsigned int k = 0; k < star.complexes[i].size(); k++){ 				//foreach selector complex
				if (star.complexes[i][k].attrName == dp->attributeNames[j] &&
						star.complexes[i][k].negValue == dp->dataTable[caseIdx][j])
					coveredByComplex = false;
			}
		}
		if (coveredByComplex)
			return true;
	}
	//not covered by any complexes:
	return false;
}

Star AQ_Inductor::calcPartialStar(int posCase, std::vector<int> negCases){
	Star partialStar;
	for (unsigned int i = 0; i < negCases.size(); i++){ //foreach negative case
		//find g(pos[i] | neg[k])

		//check if neg case is already covered
		if (i == 0 || isCovered(negCases[i], partialStar)){ //have to add i==0 check since we need something in the partial star before we start discriminating on basis of coverage
			Star caseStar; //star for a single pos and single neg case
			for (int j = 0; j < dp->n_attributes; j++){ //foreach attribute
				if (dp->dataTable[posCase][j] != dp->dataTable[negCases[i]][j])
					caseStar.addSelector(dp->attributeNames[j], dp->dataTable[negCases[i]][j]);
			}
			/*debug:*/ if(debug) {std::cout << "G(" << posCase << "|" << negCases[i] << ")\n";
			/*debug:*/      		  std::cout << "caseStar: "; caseStar.print();}
			partialStar = Star(partialStar, caseStar);
			/*debug:*/ if(debug) {std::cout << "partialStar: "; partialStar.print(); std::cout << std::endl;}
			/*TEST (BIG IF TRUE):*/ partialStar.reduce(1); //can I reduce at this level?
														//assuming reduce is arbitrary I really think nothing is lost be recducing early like this
		}
		else if (debug)
			std::cout << "Negative case " << negCases[i] << " isn't covered, no need to calculate G(" << posCase << "|" << negCases[i] << ")\n";
	}
	return partialStar;
}

bool AQ_Inductor::calcConsistency(){
	for (unsigned int i = 0; i < dp->dataTable.size(); i++){
		for (unsigned int j = 0; j < i; j++){
			bool attrsAreIdentical = true;
			for (int k = 0; (k < dp->n_attributes) && attrsAreIdentical; k++)
				if (dp->dataTable[i][k] != dp->dataTable[j][k])
					attrsAreIdentical = false;
			if (attrsAreIdentical && dp->dataTable[i][dp->n_attributes] != dp->dataTable[j][dp->n_attributes])
				return false;
		}
	}
	return true;
}

//Public functions
AQ_Inductor::AQ_Inductor(std::string fileName, bool debug = false){
  this->fileName = fileName;
	this->debug = debug;

	//could chnage dp from pointer to reference now that it is outside of try block. lots to edit though, do it later
	dp = new DataParser(fileName, debug); //took out of try/catch block so that it would be caught deeper down
  dp->buildTable();
	dp->discretizeData();

	//not sure where to put this:
	isConsistent = calcConsistency();
}

AQ_Inductor::~AQ_Inductor(){
	delete dp;

  //should be opened/closed all in the write funcs, but just in case...
  if(fs.is_open())
    fs.close();
}

void AQ_Inductor::runAQ(int maxstar){
	if (maxstar < 1){
    throw std::string("Maxstar must be 1 or higher.\n");
    return;
  }

  //run partial for each concept
	for (unsigned int i = 0; i < dp->conceptNames.size(); i++){ //foreach concept
		Star conceptStar;
		std::vector<int> positiveCases = dp->conceptCases[i];
		std::vector<int> negativeCases;
		//initialize negative cases
		for (unsigned int j = 0; j < dp->dataTable.size(); j++){
			bool isNegative = true;
			for (unsigned int k = 0; k < positiveCases.size(); k++)
				if (j == (unsigned)positiveCases[k]){
					isNegative = false;
					break;
				}
			if (isNegative)
				negativeCases.push_back(j);
		}

		if (debug){		//debug print
			std::cout << "Concept " << i << ", \nPositive cases: ";
			for (unsigned int j = 0; j < positiveCases.size(); j++)
				std::cout << positiveCases[j] << " ";
			std::cout << "\nNegative cases: ";
			for (unsigned int j = 0; j < negativeCases.size(); j++)
				std::cout << negativeCases[j] << " ";
			std::cout << "\n\n";
		}

		//actual backbone of AQ alg
		for (unsigned int j = 0; j < positiveCases.size(); j++){ //foreach positive case
			//check if pos case is already covered
			if (!isCovered(positiveCases[j], conceptStar)){ //<-TODO
				Star partial = calcPartialStar(positiveCases[j], negativeCases);
				partial.reduce(maxstar);
				conceptStar.concat(partial);
			}
			else if (debug)
				std::cout << "Positive case " << positiveCases[j] << " is already covered, no need to calculate G(" << positiveCases[j] << "|U)\n";
		}

		conceptStars.push_back(conceptStar);
	}
}

void AQ_Inductor::writeWithNeg(){
  //open file (create/overwrite)
  fs.open(stripFileExtenstion(fileName) + ".with.negation.rul", std::ifstream::out);
  if(!fs){
    throw std::string("Ugh... something went wrong. Insufficient permission maybe?\n");
    return;
  }

  //write
	if (!isConsistent)
  	fs << "! The input data set is inconsistent\n";

	//loop through conceptStars and convert to rule
	for (unsigned int i = 0; i < dp->conceptNames.size(); i++){	//foreach concept star
		for (unsigned int j = 0; j < conceptStars[i].complexes.size(); j++){ //foreach complex
			for (unsigned int k = 0; k < conceptStars[i].complexes[j].size(); k++){ //foreach selector
				fs << "(" << conceptStars[i].complexes[j][k].attrName << ", not " <<  conceptStars[i].complexes[j][k].negValue << ") ";
				if (k+1 != conceptStars[i].complexes[j].size())
					fs << "& ";
			}
			fs << "-> (" << dp->decisionName << ", " << dp->conceptNames[i] << ")\n";
		}
	}

  fs.close();
}

void AQ_Inductor::writeWithoutNeg(){
  //open file (create/overwrite)
  fs.open(stripFileExtenstion(fileName) + ".without.negation.rul", std::ifstream::out);
  if(!fs){
    throw std::string("Ugh... something went wrong. Insufficient permission maybe?\n");
    return;
  }

  //write
	if(!isConsistent)
  	fs << "! The input data set is inconsistent\n";

	// //loop through conceptStars and convert to rule
	// for (unsigned int i = 0; i < conceptStars.size(); i++){ //foreach concept
	// 	for (unsigned int j = 0; j < conceptStars[i].size(); j++){ //foreach star (within concept)
	// 		for (unsigned int k = 0; k < conceptStars[i][j].complexes.size(); k++){ //foreach complex
	// 			for (unsigned int l = 0; l < conceptStars[i][j].complexes[k].size(); l++){ //foreach selector
	//
	// 				// fs << "(" << conceptStars[i][j].complexes[k][l].attrName << ", not " <<  conceptStars[i][j].complexes[k][l].negValue << ") ";
	// 				// if (l+1 != conceptStars[i][j].complexes[k].size())
	// 				// 	fs << "& ";
	// 			}
	// 			fs << "-> (" << dp->decisionName << ", " << dp->conceptNames[i] << ")\n";
	// 		}
	// 	}
	// }

  fs.close();
}
