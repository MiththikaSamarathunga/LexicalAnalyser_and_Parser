#ifndef FLATTENST_HPP
#define FLATTENST_HPP
#include <cstring>
#include <fstream>
#include <iostream>
#include <stack>
#include <list>
#include <vector>

using namespace std;

// Node abstraction for the CSE machine for both the control and stack
struct CSEMachineNode { 
    bool isName;    //whether it's an identifier
    bool isString;
    string stringValue;
    bool isGamma;
    bool isLambda;
    std::vector<string> boundVariables;
    int indexOfBodyOfLambda; //index of the controlStructure of this lambda expression
    bool isTau;  //refers to the control stack variable which will convert stack elements to tuple
    int numberOfElementsInTauTuple;
    bool isTuple;  //refers to the CSE stack structure variable containing variables
    std::vector<CSEMachineNode> tupleElements; //can be either int/bool/string
    bool isComma;
    bool isEnvironmentMarker;
    int environmentMarkerIndex;  //for a lambda, it means the environment in which it was placed on the stack.
    string nameValue;
    bool isInt;
    int intValue;
    bool isConditional;
    bool isUnaryOperator;
    bool isBinaryOperator;
    string operatorStringValue;
    string defaultLabel;
    bool isBoolean;
    bool isBuiltInFunction;
    bool isY;
    bool isYF;
    bool isDummy;

    CSEMachineNode() {
        isName = false;
        isString = false;
        isGamma = false;
        isLambda = false;
        isTau = false;
        isEnvironmentMarker = false;
        isInt = false;
        isConditional = false;
        isUnaryOperator = false;
        isBinaryOperator = false;
        isComma = false;
        isBoolean = false;
        isBuiltInFunction = false;
        isTuple = false;
        isY = false;
        isYF = false;
        isDummy = false;
    }
};

// Node abstraction for an environment marker in the CSE machine
struct EnvironmentNode { 
    EnvironmentNode *parentEnvironment;     // Pointer to the parent environment
    EnvironmentNode *previousEnvironment;   // Pointer to the previous environment
    CSEMachineNode boundedValuesNode;
    // The boundedValuesNode stores the mappings from the boundedVariables string vector to the tupleElements CSEMachineNode vector.
    // The tupleElement could be an int, string, or Lambda.
    int environmentIndex;
};

// Declaration of the 'controlStructures' vector
extern vector<list<CSEMachineNode>> controlStructures; 

// Declaration of the numberOfControlStructures variable
extern int numberOfControlStructures;

// Recursively flattens a tree into a control structure
void recursivelyFlattenTree(Node *treeNode, list<CSEMachineNode> *controlStructure, int controlStructureIndex,bool processKid, bool processSiblings);

// Flattens a standardized tree into the control structures
void flattenStandardizedTree();

#endif