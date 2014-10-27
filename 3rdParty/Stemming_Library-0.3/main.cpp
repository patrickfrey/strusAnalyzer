//just include the stemmer file that you need
#include "english_stem.h"
#include "french_stem.h"
#include <string>
#include <iostream>

int main()
{
	//the word to be stemmed
	std::string word("documentation");

	//create an instance of a "english_stem" class.  Currently
	//only "char" strings are supported, so use the syntax below.
	stemming::english_stem<char, std::char_traits<char> > StemEnglish;
	std::cout << "(English) Original text:\t" << word.c_str() << std::endl;
	//the "english_stem" has it's operator() overloaded, so you can
	//treat your class instance like it's a function.  In this case,
	//pass in the std::string to be stemmed.  Note that this alters
	//the original std::string, so when the call is done the string will
	//be stemmed.
	StemEnglish(word);
	//now the variable "word" should equal "document"
	std::cout << "(English) Stemmed text:\t" << word.c_str() << std::endl;
	//try a similar word that should have the same stem
	word = "documenting";
	std::cout << "(English) Original text:\t" << word.c_str() << std::endl;
	StemEnglish(word);
	//now the variable "word" should equal "document"
	std::cout << "(English) Stemmed text:\t" << word.c_str() << std::endl;

	stemming::french_stem<char, std::char_traits<char> > StemFrancais;
	word = "intégralement";
	std::cout << "(French) Original text:\t" << word.c_str() << std::endl;
	StemFrancais(word);
	//now the variable "word" should equal "intégral"
	std::cout << "(French) Stemmed text:\t" << word.c_str() << std::endl;

	return 0;
}