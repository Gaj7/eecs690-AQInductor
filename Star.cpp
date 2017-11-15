#include "Star.hpp"

//Private Functions:
bool Star::isSuperset(std::vector<selector_t> c1, std::vector<selector_t> c2){
  bool isSuperset = true; //true until contradiction is found

  //loop through c2 looking for elements that aren't in c1
  for (unsigned int i = 0; i < c2.size(); i++){
    bool found = false;
    for (unsigned int j = 0; j < c1.size(); j++)
      if (c2[i].attrName == c1[j].attrName && c2[i].negValue == c1[j].negValue){
        found = true;
        break;
      }
    if (!found){
      isSuperset = false;
      break;
    }
  }

  return isSuperset;
}

//eliminate redundant supersets
void Star::simplify(){
  //NOTE: vector erase function is not efficient AT ALL, but no choice as redundant complexes could exist any place in the list -- maybe complexes should be reimplemented as linked lists?
  for (unsigned int i = 0; i < complexes.size(); i++){   //foreach complex in c -- note: for loop checks complexes.size() at each loop which is good since the size will be changing
    for (unsigned int j = 0; j < i; j++){                 //foreach earlier complex in c (we want to compare every pair)
      //we need to test in both directions - if c1 is a superset of c2, or c2 ... of c1
      if (isSuperset(complexes[i], complexes[j])){
        complexes.erase(complexes.begin()+i);         //deletes ith element and then shifts array accordingly
        break;        //**very important**            //need to break here since ith element is being deleted, need to jump to next i
      }
      else if (isSuperset(complexes[j], complexes[i]))
        complexes.erase(complexes.begin()+j);         //deletes jth element and then shifts array accordingly
    }
  }

  // //eliminate redundant supersets
  // //NOTE: vector erase function is not efficient AT ALL, but no choice as redundant complexes could exist any place in the list -- maybe complexes should be reimplemented as linked lists?
  // for (unsigned int i = 0; i < complexes.size(); i++){   //foreach complex in c -- note: for loop checks complexes.size() at each loop which is good since the size will be changing
  //   unsigned int j = 0;
  //   while (j < i){                 //foreach earlier complex in c (we want to compare every pair)
  //     //we need to test in both directions - if c1 is a superset of c2, or c2 ... of c1
  //     if (isSuperset(complexes[i], complexes[j])){
  //       complexes.erase(complexes.begin()+i);         //deletes ith element and then shifts array accordingly
  //       break;        //**very important**            //need to break here since ith element is being deleted, need to jump to next i
  //     }
  //     else if (isSuperset(complexes[j], complexes[i]))
  //       complexes.erase(complexes.begin()+j);         //deletes jth element and then shifts array accordingly
  //     else
  //       j++;                                          //we don't want to increment j after an erase since the element at the jth position will now be different
  //   }
  // }
}

//Public functions:

//default constructor
Star::Star(){
}

/*
  if s1 is empty, set equal to s2 (and vice versa)

  foreach complex c1 in s1
    foreach complex c2 in s2
      create a complex in this star by concatenating c1 and c2, and eliminating redundancy

  at some point, loop through new star comparing complexes and identify and remove redundant complexes (superset of other complexes)
*/
Star::Star(Star s1, Star s2){
  //special cases if s1 or s2 are empty
  if(s1.complexes.empty()){
    complexes = s2.complexes;
    return;
  }
  if(s2.complexes.empty()){
    complexes = s1.complexes;
    return;
  }

  //disjunct complexes
  for (unsigned int i = 0; i < s1.complexes.size(); i++){    //foreach complex c1 in s1
    for (unsigned int j = 0; j < s2.complexes.size(); j++){  //foreach complex c2 in s2
      //create a complex c from by combining c1 and c2
      std::vector<selector_t> complex;
      for (unsigned int k = 0; k < s1.complexes[i].size(); k++)  //foreach selector s in c1
        complex.push_back(s1.complexes[i][k]);                   //add s to c
      for (unsigned int k = 0; k < s2.complexes[j].size(); k++){ //foreach selector s in c2
        bool redundant = false;
        for (unsigned int l = 0; l < s1.complexes[i].size(); l++)
          if (s1.complexes[i][l].attrName == s2.complexes[j][k].attrName &&
              s1.complexes[i][l].negValue == s2.complexes[j][k].negValue)
            redundant = true;
        if (!redundant)
          complex.push_back(s2.complexes[j][k]);                   //add s to c if not redundant
      }
      //can i check for redundancy before adding?
      complexes.push_back(complex);                                //add c to complexes
    }
  }

  //eliminate redundant supersets
  simplify();
}

void Star::addSelector(std::string attrName, std::string negValue){
  selector_t selector = {attrName, negValue};
  std::vector<selector_t> complex;
  complex.push_back(selector);
  complexes.push_back(complex);
}

void Star::reduce(int maxstar){
  if(complexes.size() > (unsigned)maxstar)
    complexes.resize(maxstar); //just shortens to length of maxstar, arbitrarily cutting off latter complexes
}

void Star::concat(Star s){
  for (unsigned int i = 0; i < s.complexes.size(); i++)
    complexes.push_back(s.complexes[i]);

  //eliminates redundant supersets
  simplify();
}

Star Star::invert(std::vector<std::string> attributeNames, std::vector<std::vector<std::string>> attributeValues){
  Star inverted;
  for (unsigned int i = 0; i < complexes.size(); i++){      //foreach complex
    //generate list of values for each selector
    std::vector<std::vector<std::string>> selectorValues;
    std::vector<std::string> selectorNames;
    for (unsigned int j = 0; j < complexes[i].size(); j++){ //foreach selector
      unsigned int attrIdx;
      for (attrIdx = 0; attributeNames[attrIdx] != complexes[i][j].attrName; attrIdx++); //NOTE: absolutely zero protection here. If the complex somehow doesn't have an attrName that exists in attributeNames, the program WILL seg fault

      bool attrAlreadyExists = false;
      unsigned int selIdx;
      for (selIdx = 0; selIdx < selectorNames.size(); selIdx++)
        if (selectorNames[selIdx] == attributeNames[attrIdx]){
          attrAlreadyExists = true;
          break;
        }

      if (attrAlreadyExists){
        //selectorValues[selIdx].erase(complexes[i][j].negValue);
        for (unsigned int k = 0; k < selectorValues[selIdx].size(); k++)
          if (selectorValues[selIdx][k] == complexes[i][j].negValue)
              selectorValues[selIdx].erase(selectorValues[selIdx].begin() + k);
      }
      else{
        selectorValues.resize(selectorValues.size()+1);
        selectorNames.push_back(attributeNames[attrIdx]);
        for (unsigned int k = 0; k < attributeValues[attrIdx].size(); k++)
          if (attributeValues[attrIdx][k] != complexes[i][j].negValue)
            selectorValues[j].push_back(attributeValues[attrIdx][k]);
      }
    }

    //debug print selector names/values
    std::cout << "Rule " << i << ": \n";
    for (unsigned int x = 0; x < selectorValues.size(); x++){
      std::cout << selectorNames[x] << ": ";
      for (unsigned int y = 0; y < selectorValues[x].size(); y++)
        std::cout<< selectorValues[x][y] << " ";
      std::cout << '\n';
    } std::cout << '\n';

    // //expand complex into multiple complexes using list of selector values
    // unsigned int numComplexes = 1;
    // for (unsigned int j = 0; j < selectorValues.size(); j++)
    //   numComplexes *= selectorValues[j].size();
    // unsigned int oldSize = inverted.complexes.size();
    // inverted.complexes.resize(oldSize + numComplexes);
    // for (unsigned int j = 0; j < selectorValues.size(); j++){ //foreach selector
    //   for (unsigned int k = 0; k < selectorValues[j].size(); k++){
    //     selector_t selector = {selectorNames[j], selectorValues[j][k]};
    //     unsigned int reps = numComplexes/selectorValues.size();
    //     for (unsigned int l = 0; l < reps; l++)
    //       inverted.complexes[oldSize + k*reps + l].push_back(selector);
    //   }
    // }
  }

  return inverted;
}

void Star::print(){
  std::cout << "{";
  for (unsigned int i = 0; i < complexes.size(); i++){
    for (unsigned int j = 0; j < complexes[i].size(); j++){
      std::cout << "(" << complexes[i][j].attrName << ", !" << complexes[i][j].negValue << ")";
      if(j+1 != complexes[i].size())
        std::cout << "&";
    }
    if(i+1 != complexes.size())
      std::cout << ", ";
  }
  std::cout << "}\n";
}
