#include "parser.hpp"
#include "lexicon.hpp"
#include "asttost.hpp"
#include "flattenst.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <stack>
#include <list>

// Initialize a controlStructures vector with a size of 150 lists, each containing CSEMachineNodes
vector<list<CSEMachineNode>> controlStructures(150); // each controlStructure would be a list of CSEMachineNodes

// Initialize the number of control structures to 1
int numberOfControlStructures = 1;

// Function to recursively flatten the tree representation into control structures
void recursivelyFlattenTree(Node *treeNode, list<CSEMachineNode> *controlStructure, int controlStructureIndex,
                            bool processKid, bool processSiblings)
{
    // Create a new CSEMachineNode for the current control structure node
    CSEMachineNode controlStructureNode = CSEMachineNode();

    // Set the default label of the control structure node to the label of the current tree node
    controlStructureNode.defaultLabel = treeNode->label;

    // Check the label of the current tree node and handle different cases accordingly
    if (treeNode->label == "gamma" || treeNode->label == GAMMA_STD_LABEL)
    {
        // Mark the node as representing a gamma expression
        controlStructureNode.isGamma = true;
        // Set the default label to "gamma"
        controlStructureNode.defaultLabel = "gamma";
        // Add the control structure node to the list of control structures
        controlStructure->push_back(controlStructureNode);
    }
    else if (treeNode->label == "Y")
    {
        // Mark the node as representing a Y combinator
        controlStructureNode.isY = true;
        // Set the default label to "Y"
        controlStructureNode.defaultLabel = "Y";
        // Add the control structure node to the list of control structures
        controlStructure->push_back(controlStructureNode);
    }
    else if (treeNode->label.compare(0, 6, "<STR:'") == 0)
    {
        // Handle the case when the label starts with "<STR:'"
        controlStructureNode.isString = true;
        // Extract the string value from the label
        controlStructureNode.stringValue = treeNode->label.substr(6);
        // Remove the extra characters surrounding the string value
        controlStructureNode.stringValue = controlStructureNode.stringValue.substr(0,
                                                                                   controlStructureNode.stringValue.length() -
                                                                                       2);
        // Set the default label to the extracted string value
        controlStructureNode.defaultLabel = controlStructureNode.stringValue;
        // Add the control structure node to the list of control structures
        controlStructure->push_back(controlStructureNode);
    }
    else if (treeNode->label.compare(0, 4, "<ID:") == 0)
    {
        // Handle the case when the label starts with "<ID:"
        controlStructureNode.isName = true;
        // Extract the name value from the label
        controlStructureNode.nameValue = treeNode->label.substr(4);
        // Remove the extra characters surrounding the name value
        controlStructureNode.nameValue = controlStructureNode.nameValue.substr(0,
                                                                               controlStructureNode.nameValue.length() -
                                                                                   1);
        // Set the default label to the extracted name value
        controlStructureNode.defaultLabel = controlStructureNode.nameValue;
        // Add the control structure node to the list of control structures
        controlStructure->push_back(controlStructureNode);
    }
    else if (treeNode->label.compare(0, 5, "<INT:") == 0)
    {
        // Handle the case when the label starts with "<INT:"
        controlStructureNode.isInt = true;
        // Extract the integer string value from the label
        string intString = treeNode->label.substr(5);
        // Remove the extra characters surrounding the integer string value
        intString = intString.substr(0,
                                     intString.length() -
                                         1);
        // Convert the integer string value to an integer and assign it to the node
        controlStructureNode.intValue = atoi(intString.c_str());
        // Set the default label to the integer string value
        controlStructureNode.defaultLabel = intString;
        // Add the control structure node to the list of control structures
        controlStructure->push_back(controlStructureNode);
    }


    // Handle the case when the tree node represents a boolean true or false value
else if (treeNode->label == "<true>" || treeNode->label == "<false>")
{
    // Mark the control structure node as a boolean
    controlStructureNode.isBoolean = true;
    // Set the default label to "true" if the label is "<true>", otherwise set it to "false"
    controlStructureNode.defaultLabel = treeNode->label == "<true>" ? "true" : "false";
    // Add the control structure node to the list of control structures
    controlStructure->push_back(controlStructureNode);
}

// Handle the case when the tree node represents a nil value
else if (treeNode->label == "<nil>")
{
    // Mark the control structure node as a tuple
    controlStructureNode.isTuple = true;
    // Set the default label to "nil"
    controlStructureNode.defaultLabel = "nil";
    // Set the number of elements in the tau tuple to 0
    controlStructureNode.numberOfElementsInTauTuple = 0;
    // Add the control structure node to the list of control structures
    controlStructure->push_back(controlStructureNode);
}

// Handle the case when the tree node represents a dummy value
else if (treeNode->label == "<dummy>")
{
    // Mark the control structure node as a dummy
    controlStructureNode.isDummy = true;
    // Set the default label to "dummy"
    controlStructureNode.defaultLabel = "dummy";
    // Add the control structure node to the list of control structures
    controlStructure->push_back(controlStructureNode);
}

// Handle the case when the tree node represents a lambda expression
else if (treeNode->label == LAMBDA_STD_LABEL || treeNode->label == "lambda")
{
    // Disable processing of the child nodes
    processKid = false;
    // Mark the control structure node as a lambda
    controlStructureNode.isLambda = true;
    // Initialize the number of bound variables to 0
    int numberOfBoundVariables = 0;
    // Check if the first child node represents multiple bound variables
    if (treeNode->firstKid->label == ",")
    {
        // Iterate through the list of bound variables
        Node *boundVariableNode = treeNode->firstKid->firstKid;
        while (boundVariableNode != NULL)
        {
            // Increment the count of bound variables
            numberOfBoundVariables++;
            // Extract the variable name from the label
            string variable = boundVariableNode->label.substr(4);
            variable = variable.substr(0, variable.length() - 1);
            // Add the variable to the list of bound variables
            controlStructureNode.boundVariables.push_back(variable);
            // Move to the next sibling node
            boundVariableNode = boundVariableNode->nextSibling;
        }
    }
    else
    {
        // If there is only one bound variable
        numberOfBoundVariables++;
        // Extract the variable name from the label
        string variable = treeNode->firstKid->label.substr(4);
        variable = variable.substr(0, variable.length() - 1);
        // Add the variable to the list of bound variables
        controlStructureNode.boundVariables.push_back(variable);
    }
    // Increment the index of the body of the lambda expression and assign it to the node
    controlStructureNode.indexOfBodyOfLambda = numberOfControlStructures++;
    // Set the number of elements in the tau tuple to the number of bound variables
    controlStructureNode.numberOfElementsInTauTuple = numberOfBoundVariables;
    // Construct a string representing the bound variables
    string boundVariables;
    for (int i = 0; i < numberOfBoundVariables; i++)
    {
        boundVariables += controlStructureNode.boundVariables[i] + ", ";
    }
    // Set the default label to describe the lambda expression
    controlStructureNode.defaultLabel =
        "Lambda with bound variables(" + boundVariables + ") and body(" +
        std::to_string(controlStructureNode.indexOfBodyOfLambda) + ")";
    // Add the control structure node to the list of control structures
    controlStructure->push_back(controlStructureNode);

    // Create a new control structure for the body of the lambda expression and recursively flatten it
    list<CSEMachineNode> *controlStructureOfLambda = new list<CSEMachineNode>;
    recursivelyFlattenTree(treeNode->firstKid->nextSibling, controlStructureOfLambda,
                           controlStructureNode.indexOfBodyOfLambda, true, true);
}

   // Handle the case when the tree node represents a conditional operator "->"
else if (treeNode->label == "->")
{
    // Disable processing of the child nodes
    processKid = false;

    // Create nodes for the true, false, and beta branches
    CSEMachineNode trueNode = CSEMachineNode();
    CSEMachineNode falseNode = CSEMachineNode();
    CSEMachineNode betaNode = CSEMachineNode();

    // Mark the beta, true, and false nodes as conditional nodes
    betaNode.isConditional = true;
    trueNode.isConditional = true;
    falseNode.isConditional = true;

    // Set default labels for beta, true, and false nodes
    betaNode.defaultLabel = "BetaNode";
    trueNode.defaultLabel = "trueNode";
    falseNode.defaultLabel = "falseNode";

    // Assign indices for the bodies of the true and false branches
    trueNode.indexOfBodyOfLambda = numberOfControlStructures++;
    falseNode.indexOfBodyOfLambda = numberOfControlStructures++;
    betaNode.indexOfBodyOfLambda = controlStructureIndex;

    // Create control structures for the true and false branches and recursively flatten them
    list<CSEMachineNode> *controlStructureOfTrueNode = new list<CSEMachineNode>;
    recursivelyFlattenTree(treeNode->firstKid->nextSibling, controlStructureOfTrueNode,
                           trueNode.indexOfBodyOfLambda, true, false);
    list<CSEMachineNode> *controlStructureOfFalseNode = new list<CSEMachineNode>;
    recursivelyFlattenTree(treeNode->firstKid->nextSibling->nextSibling, controlStructureOfFalseNode,
                           falseNode.indexOfBodyOfLambda, true, false);

    // Add the true, false, and beta nodes to the control structure list
    controlStructure->push_back(trueNode);
    controlStructure->push_back(falseNode);
    controlStructure->push_back(betaNode);

    // Recursively flatten the child node
    recursivelyFlattenTree(treeNode->firstKid, controlStructure, controlStructureIndex, true, false);
}

// Handle the case when the tree node represents a unary operator ("not" or "neg")
else if (treeNode->label == "not" || treeNode->label == "neg")
{
    // Mark the control structure node as a unary operator
    controlStructureNode.isUnaryOperator = true;
    // Set the operator string value
    controlStructureNode.operatorStringValue = treeNode->label;
    // Add the control structure node to the list of control structures
    controlStructure->push_back(controlStructureNode);
}

// Handle the case when the tree node represents a binary operator
else if (treeNode->label == "aug" || treeNode->label == "or" || treeNode->label == "&" ||
         treeNode->label == "gr" || treeNode->label == "ge" || treeNode->label == "ls" ||
         treeNode->label == "le" || treeNode->label == "eq" || treeNode->label == "ne" ||
         treeNode->label == "+" || treeNode->label == "-" || treeNode->label == "*" ||
         treeNode->label == "/" || treeNode->label == "**")
{
    // Mark the control structure node as a binary operator
    controlStructureNode.isBinaryOperator = true;
    // Set the operator string value
    controlStructureNode.operatorStringValue = treeNode->label;
    // Add the control structure node to the list of control structures
    controlStructure->push_back(controlStructureNode);
}

// Handle the case when the tree node represents a tau tuple
else if (treeNode->label == "tau")
{
    // Disable processing of the child nodes
    processKid = false;

    // Mark the control structure node as a tau tuple
    controlStructureNode.isTau = true;

    // Count the number of elements in the tau tuple
    int numberOfElementsInTuple = 0;
    Node *tauElementNode = treeNode->firstKid;
    do
    {
        numberOfElementsInTuple++;
        tauElementNode = tauElementNode->nextSibling;
    } while (tauElementNode != NULL);

    // Set the number of elements in the tau tuple
    controlStructureNode.numberOfElementsInTauTuple = numberOfElementsInTuple;

    // Set the default label for the tau tuple
    controlStructureNode.defaultLabel =
        "TAU[" + std::to_string(controlStructureNode.numberOfElementsInTauTuple) + "]";

    // Add the control structure node to the list of control structures
    controlStructure->push_back(controlStructureNode);

    // Process each element of the tau tuple
    tauElementNode = treeNode->firstKid;
    do
    {
        CSEMachineNode tupleElementNode = CSEMachineNode();
        // Handle different types of elements within the tau tuple
        if (tauElementNode->label.compare(0, 6, "<STR:'") == 0)
        {
            // Handle string elements
            tupleElementNode.isString = true;
            tupleElementNode.stringValue = tauElementNode->label.substr(6);
            tupleElementNode.stringValue = tupleElementNode.stringValue.substr(0,
                                                                               tupleElementNode.stringValue.length() -
                                                                                   2);
            tupleElementNode.defaultLabel = tupleElementNode.stringValue;
            controlStructure->push_back(tupleElementNode);
        }
        else if (tauElementNode->label.compare(0, 4, "<ID:") == 0)
        {
            // Handle identifier elements
            tupleElementNode.isName = true;
            tupleElementNode.nameValue = tauElementNode->label.substr(4);
            tupleElementNode.nameValue = tupleElementNode.nameValue.substr(0,
                                                                           tupleElementNode.nameValue.length() -
                                                                               1);
            tupleElementNode.defaultLabel = tupleElementNode.nameValue;
            controlStructure->push_back(tupleElementNode);
        }
        else if (tauElementNode->label.compare(0, 5, "<INT:") == 0)
        {
            // Handle integer elements
            tupleElementNode.isInt = true;
            string intString = tauElementNode->label.substr(5);
            intString = intString.substr(0,
                                         intString.length() -
                                             1);
            tupleElementNode.intValue = atoi(intString.c_str());
            tupleElementNode.defaultLabel = intString;
            controlStructure->push_back(tupleElementNode);
        }
        else if (tauElementNode->label == "<true>" || tauElementNode->label == "<false>")
        {
            // Handle boolean elements
            tupleElementNode.isBoolean = true;
            tupleElementNode.defaultLabel = tauElementNode->label == "<true>" ? "true" : "false";
            controlStructure->push_back(tupleElementNode);
        }
        else if (tauElementNode->label == "gamma" || tauElementNode->label == GAMMA_STD_LABEL)
        {
            // Handle gamma elements
            tupleElementNode.isGamma = true;
            tupleElementNode.defaultLabel = "gamma";
            controlStructure->push_back(tupleElementNode);
            // Recursively flatten the gamma node
            recursivelyFlattenTree(tauElementNode->firstKid, controlStructure, controlStructureIndex, true, true);
        }
        else if (tauElementNode->label == "aug" || tauElementNode->label == "or" ||
                 tauElementNode->label == "&" || tauElementNode->label == "gr" ||
                 tauElementNode->label == "ge" || tauElementNode->label == "ls" ||
                 tauElementNode->label == "le" || tauElementNode->label == "eq" ||
                 tauElementNode->label == "ne" || tauElementNode->label == "+" ||
                 tauElementNode->label == "-" || tauElementNode->label == "*" ||
                 tauElementNode->label == "/" || tauElementNode->label == "**")
        {
            // Handle binary operator elements
            tupleElementNode.isBinaryOperator = true;
            tupleElementNode.operatorStringValue = tauElementNode->label;
            controlStructure->push_back(tupleElementNode);
            // Recursively flatten the binary operator node
            recursivelyFlattenTree(tauElementNode->firstKid, controlStructure, controlStructureIndex, true, true);
        }
        else
        {
            // Handle other types of elements
            recursivelyFlattenTree(tauElementNode, controlStructure, controlStructureIndex, true, false);
        }
        // Move to the next sibling node
        tauElementNode = tauElementNode->nextSibling;
    } while (tauElementNode != NULL);
}

    // Process the current tree node based on its label
else if (treeNode->label == ",")
{
    // Set the flag indicating it's a comma node
    controlStructureNode.isComma = true;
    // Add the node to the control structure list
    controlStructure->push_back(controlStructureNode);
}
else if (treeNode->label == "true" || treeNode->label == "false")
{
    // Set the flag indicating it's a boolean node
    controlStructureNode.isBoolean = true;
    // Add the node to the control structure list
    controlStructure->push_back(controlStructureNode);
}

// Update the control structures array with the current control structure
controlStructures[controlStructureIndex] = *controlStructure;

// Recursively process the children of the current node if required
if (processKid && treeNode->firstKid != NULL)
{
    recursivelyFlattenTree(treeNode->firstKid, controlStructure, controlStructureIndex, true, true);
}

// Recursively process the siblings of the current node if required
if (processSiblings && treeNode->nextSibling != NULL)
{
    recursivelyFlattenTree(treeNode->nextSibling, controlStructure, controlStructureIndex, true, true);
}
}
// Function to flatten the standardized tree into control structures
void flattenStandardizedTree()
{
    // Check if the tree stack is not empty
    if (!trees.empty())
    {
        // Get the root of the tree from the top of the stack
        Node *treeRoot = trees.top();
        // Declare a pointer for the list of control structures
        list<CSEMachineNode> *controlStructure = new list<CSEMachineNode>;
        // Recursively flatten the tree starting from the root
        recursivelyFlattenTree(treeRoot, controlStructure, 0, true, true);
    }
}
