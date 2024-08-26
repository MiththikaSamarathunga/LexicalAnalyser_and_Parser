#ifndef PARSER_HPP
#define PARSER_HPP
#include <cstring>
#include <fstream>
#include <iostream>
#include <stack>

using namespace std;

// Structure for representing nodes in the tree
struct Node { 
    string label;
    struct Node *firstKid;     // Pointer to the first child node
    struct Node *nextSibling;  // Pointer to the next sibling node
};

// External constants for node labels
extern const string FCN_FORM_LABEL;

extern const string GAMMA_STD_LABEL;
extern const string LAMBDA_STD_LABEL;

extern stack<Node *> trees;     // Stack to store trees

/**
   Builds a tree with the given node label and pops the specified number of trees to make children
   noOfTreesToPopAndMakeChildren The number of trees to pop and make children
 **/
void buildTree(string nodeLabel, int noOfTreesToPopAndMakeChildren);

/*
 * Parses the input file
 * file The input file stream
 */
void D(ifstream &file);

/*
 * Reads a token from the input file
 * file The input file stream
 * token The token to be read
 */
void readToken(ifstream &file, string token);

/**
 * Parses the input file (Vl function)
 * param file The input file stream
 * param identifiersReadBefore The number of identifiers read before
 * param isRecursiveCall Whether it's a recursive call
 * return The result of the parsing
 */
int Vl(ifstream &file, int identifiersReadBefore, bool isRecursiveCall);

void Vb(ifstream &file);
void Db(ifstream &file);
void Dr(ifstream &file);
void Da(ifstream &file);
void D(ifstream &file);
void Rn(ifstream &file);
void R(ifstream &file);
void Ap(ifstream &file);
void Af(ifstream &file);
void At(ifstream &file);
void A(ifstream &file);
void Bp(ifstream &file);
void Bs(ifstream &file);
void Bt(ifstream &file);
void B(ifstream &file);
void Tc(ifstream &file);
void Ta(ifstream &file);
void T(ifstream &file);
void Ew(ifstream &file);
void E(ifstream &file);

// Main expression parsing function
void E(ifstream &file);

#endif //INTERPRETER_PARSER_H
