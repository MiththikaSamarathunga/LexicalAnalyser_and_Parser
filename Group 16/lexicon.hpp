#ifndef LEXICON_HPP
#define LEXICON_HPP
#include <cstring>
#include <fstream>

using namespace std;

extern string NT; 

extern const string UNDEFINED;
extern const string STRING;
extern const string IDENTIFIER;
extern const string PUNCTUATION;
extern const string OPERATOR;
extern const string KEYWORD;
extern const string INTEGER;

extern const string GAMMA_STD_LABEL;
extern const string LAMBDA_STD_LABEL;

extern const string FCN_FORM_LABEL;

extern const char *eolCharArray;
extern const char *punctuationArray;
extern const char *stringAllowedCharArray;
extern const char *operatorArray;
extern const char *stringAllowedEscapeCharArray;

extern string nextTokenType;


void readIntegerToken(ifstream &file);
bool isStringAllowedChar(char c);
bool checkIfEOF(ifstream &file);
bool isOperator(char c);
void readPunctuationChar(ifstream &file);
void resolveIfCommentOrOperator(ifstream &file);
void readIdentifierToken(ifstream &file);
void scan(ifstream &file);
void readStringToken(ifstream &file);
bool isPunctuation(char c);
void readOperatorToken(ifstream &file);


#endif //RPAL_INTERPRETER_LEXICON_H
