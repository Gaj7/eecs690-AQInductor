#ifndef Star_HPP
#define Star_HPP

#include <string>
#include <vector>
#include <iostream> //just for debug print func

struct selector_t {
	std::string attrName;
	std::string negValue;
};

class Star {
	private:
		//checks if c1 is a superset of c2
		bool isSuperset(std::vector<selector_t> c1, std::vector<selector_t> c2);

		//recursive helper function for invert
		Star recExpand(unsigned int selIdx, std::vector<std::string> selectorNames, std::vector<std::vector<std::string>> selectorValues);

	public:
		std::vector<std::vector<selector_t>> complexes;

		Star();
		Star(Star s1, Star s2, bool = true); //creates star from disjunction

    void addSelector(std::string attrName, std::string negValue);
		void reduce(int maxstar);
		void concat(Star s, bool = true); //union is a keyword apparently
		void simplify(); //moved from private to public because I want AQ to have more control over when to simplify

		//returns a new star which is an inversion of *this
		Star invert(std::vector<std::string> attributeNames, std::vector<std::vector<std::string>> attributeValues);

		//debug:
		void print();

};

#endif /* Star_HPP */
