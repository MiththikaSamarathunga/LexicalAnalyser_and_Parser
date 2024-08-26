#include "lexicon.hpp"
#include "iostream"
#include <fstream>
#include <cstring>

using namespace std;

string NT;
const string IDENTIFIER = "IDENTIFIER";
const string STRING = "STRING";
const string OPERATOR = "OPERATOR";
const string INTEGER = "INTEGER";
const string KEYWORD = "KEYWORD";
const string UNDEFINED = "UNDEFINED";
const string PUNCTUATION = "PUNCTUATION";

const char *operatorArray = "+-*<>&.@/:=~|$!#%^_[]{}\"`?";

const char *stringAllowedCharArray = "();, ";
const char *stringAllowedEscapeCharArray = "tn\\\'";

const char *eolCharArray = "\r\n";

const char *punctuationArray = "();,";

string nextTokenType = UNDEFINED;

/*
    Checks if the end of the file has been reached using the good() function and peek()
    function. The good() function checks if the stream is in a good state, and the peek()
    function looks ahead to the next character without extracting it. If the stream is
    in a good state and the next character is the end-of-file character, then the
    function returns true, indicating that the end of the file has been reached.
    Otherwise, the function returns false.
*/
bool checkIfEOF(ifstream &file)
{
    if (!file.good() || file.peek() == EOF)
    {
        return true;
    }
    return false;
}

/*
    Reads an identifier token from the input stream. An identifier token is a sequence of
    letters, digits, or underscores that does not start with a digit. If the end of the
    file is reached unexpectedly, the function terminates the program. If the identifier
    token matches one of the keywords, the `nextTokenType` variable is set to `KEYWORD`.
    Otherwise, the `nextTokenType` variable is set to `IDENTIFIER`. The identifier token
    is stored in the `NT` string.
*/
void readIdentifierToken(ifstream &file)
{
    if (checkIfEOF(file))
    {
        exit(0);
    }
    nextTokenType = IDENTIFIER;
    char x;                  // get the next character in input stream
    char peek = file.peek(); // peek and store the next character in input stream

    while (isdigit(peek) || isalpha(peek) || peek == '_')
    {
        file.get(x);
        NT += x;
        peek = file.peek();
    }
    if (NT.compare("rec") == 0 || NT.compare("where") == 0 || NT.compare("in") == 0 || NT.compare("and") == 0 ||
        NT.compare("let") == 0 || NT.compare("fn") == 0 || NT.compare("or") == 0 || NT.compare("not") == 0 ||
        NT.compare("gr") == 0 || NT.compare("ge") == 0 || NT.compare("ls") == 0 || NT.compare("le") == 0 ||
        NT.compare("eq") == 0 || NT.compare("ne") == 0 || NT.compare("within") == 0 || NT.compare("true") == 0 ||
        NT.compare("false") == 0 || NT.compare("nil") == 0 || NT.compare("dummy") == 0 || NT.compare("aug") == 0)
    {
        nextTokenType = KEYWORD;
    }
}

void readIntegerToken(ifstream &file)
{
    if (checkIfEOF(file))
    {
        exit(0);
    }
    nextTokenType = INTEGER;
    char x;                  // get the next character in input stream
    char peek = file.peek(); // peek and store the next character input stream

    while (isdigit(peek))
    {
        file.get(x);
        NT += x;
        peek = file.peek();
    }
}

/*
    Checks if the given character is an operator. The function takes a character `c` as
    an argument and returns `true` if `c` is an operator, and `false` otherwise. The
    function uses the `strchr` function to check if `c` is in the `operatorArray` string,
    which contains all the operator characters.
*/
bool isOperator(char c)
{
    if (strchr(operatorArray, c))
        return true;
    else
        return false;
}

void readOperatorToken(ifstream &file)
{
    // Check if the end of the file has been reached
    if (checkIfEOF(file))
    {
        exit(0);
    }
    nextTokenType = OPERATOR;

    // Get the next character in the stream and store it in the `x` variable
    char x;  

    char peek = file.peek(); // Peek and store the next character in the stream in the `peek` variable

    while (isOperator(peek))
    {
        file.get(x);
        NT += x;
        peek = file.peek();
    }
}

bool isPunctuation(char c)
{
    // Check if the character is in the punctuation array
    if (strchr(punctuationArray, c))
        return true;
    else
        return false;
}

void readPunctuationChar(ifstream &file)
{
    if (checkIfEOF(file))
    {
        exit(0);
    }
    nextTokenType = PUNCTUATION;
    char x;                  // get the next character in input stream
    char peek = file.peek(); // peek and store the next character input stream
    if (isPunctuation(peek))
    {
        file.get(x);
        NT += x;
    }
}

bool isStringAllowedChar(char c)
{
    // Check if the character is in the stringAllowedCharArray, or if it's a digit, alphabetic character, or an operator
    if (strchr(stringAllowedCharArray, c) || isdigit(c) || isalpha(c) || isOperator(c))
        return true;
    else
        return false;
}

bool isEscapeCharInString(ifstream &file, char &peek)
{
    char x; // get the next character in stream in this
    // peek and store the next character in stream in this
    if (peek == '\\')
    {
        file.get(x);
        NT += x; // Add the escape backslash to the string token (as per the reference implementation)
        peek = file.peek();
        if (strchr(stringAllowedEscapeCharArray, peek))
        {
            file.get(x);
            NT += x;
            // Move to the next character in the file stream
            peek = file.peek();
            return true;
        }
        else
        {
            // If the next character is not an allowed escape character, print an error message and throw an exception
            cout << "\n\nERROR! Expected an escape character, but " << peek << " happened! Parser will DIE now!\n\n";
            throw exception();
        }
    }
    else
        return false;     // If the current character is not a backslash, return false, indicating that no escape character was found
}

void readStringToken(ifstream &file)
{
    if (checkIfEOF(file))
    {
        exit(0);
    }
    nextTokenType = STRING;
    char x;                  // get the next character in stream in this
    char peek = file.peek(); // peek and store the next character in stream in this

    if (peek == '\'')
    { // check for the single quote to start the string
        file.get(x);
        NT += x; // Add quotes to the token to separate the string from non-string literals with same value
        peek = file.peek();
    }
    else
    {
        throw exception();
    }
    while (isStringAllowedChar(peek) || (isEscapeCharInString(file, peek) && isStringAllowedChar(peek)))
    {
        file.get(x);
        NT += x;
        peek = file.peek();
    }
    if (peek == '\'')
    { // check for the single quote to close the string
        file.get(x);
        NT += x; // Add quotes to the token to separate the string from non-string literals with same value
    }
    else
    {
        throw exception();
    }
}

void scan(ifstream &file);

void resolveIfCommentOrOperator(ifstream &file)
{
    char x;
    file.get(x); // Move past the first '/'
    char peek = file.peek();
    if (peek == '/')
    {
        // This means it's a comment line, so keep reading/updating file stream pointer without "tokenizing" (adding to NT) until an eol.
        while (!strchr(eolCharArray, peek))
        {
            file.get(x); // move past the whitespaces until an eol
            peek = file.peek();
        }
        file.get(x); // Move past the EOL
        scan(file); // call scan to get the next token
    }
    else
    {
        // this means it's an operator sequence
        NT += '/'; // Add the first '/' that we moved past to the operator token
        readOperatorToken(file);
    }
}

void scan(ifstream &file)
{
    // If the end of the file has been reached, return
    if (checkIfEOF(file))
    {
        return;
    }
    nextTokenType = UNDEFINED;

    // Peek and store the next character in the file stream
    char peek = file.peek(); 

    // Clear the current token string
    NT.clear(); 

    // If the next character is an alphabetic character
    if (isalpha(peek))
    {
        // Read an identifier token
        readIdentifierToken(file);
    }
    // If the next character is a digit
    else if (isdigit(peek))
    {
        readIntegerToken(file);
    }
    else if (peek == '/')
    {
        // Determine if the slash is the start of a comment or a division operator
        resolveIfCommentOrOperator(file);
    }
    else if (isOperator(peek))
    {
        readOperatorToken(file);
    }
    else if (peek == '\'')
    { // Start of a string
        readStringToken(file);
    }
    else if (iswspace(peek))
    { 
        // Ignore the whitespace character and call scan recursively to get the next token
        char x;
        file.get(x); // further the file pointer and ignore the whitespace character (no need to tokenize it)
        NT += x;
        scan(file); // call scan to get the next token
    }
    else if (isPunctuation(peek))
    {
        readPunctuationChar(file);
    }
}
