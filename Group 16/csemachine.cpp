#include "parser.hpp"
#include "lexicon.hpp"
#include "asttost.hpp"
#include "flattenst.hpp"
#include "csemachine.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <stack>
#include <list>
#include <cmath>

// Array to store EnvironmentNode pointers, representing environments in the CSE machine
EnvironmentNode *environments[1200];

// Stack to hold CSEMachineNode objects, representing the Control stack of the CSE machine
stack<CSEMachineNode> cseMachineControl;

// Stack to hold CSEMachineNode objects, representing the "Stack" stack of values of the CSE machine
stack<CSEMachineNode> cseMachineStack;

// Pointer to the current environment
EnvironmentNode *currentEnvironment = new EnvironmentNode;

// Counter to keep track of the number of environments created
int environmentCounter = 0;

// Function to initialize the CSE machine
void initializeCSEMachine()
{
    // Initialize environment with the primitive environment (PE / e0)
    currentEnvironment->environmentIndex = 0;
    currentEnvironment->parentEnvironment = NULL;
    currentEnvironment->previousEnvironment = NULL;
    environments[environmentCounter++] = currentEnvironment;

    // Initialize control.
    // Push the first token as the e0 environment variable
    CSEMachineNode e0 = CSEMachineNode();
    e0.isEnvironmentMarker = true;
    e0.environmentMarkerIndex = 0;
    e0.defaultLabel = "e0";
    cseMachineControl.push(e0);

    // Push the elements of the 0th control structure onto the control stack
    std::list<CSEMachineNode>::const_iterator iterator;
    for (iterator = controlStructures[0].begin(); iterator != controlStructures[0].end(); ++iterator)
    {
        CSEMachineNode controlStructureToken = *iterator;
        cseMachineControl.push(controlStructureToken);
    }

    // Initialize stack with e0 as well
    cseMachineStack.push(e0);
}


// Function to print the tree recursively
void printTree()
{
    // Check if the tree stack is not empty
    if (!trees.empty())
    {
        // Get the root of the tree from the top of the stack
        Node *treeRoot = trees.top();
        // Call the recursive function to print the tree starting from the root
        recursivelyPrintTree(treeRoot, "");
    }
}

// Recursive function to print the tree nodes with proper indentation
void recursivelyPrintTree(Node *node, string indentDots)
{
    // Print the current node label with indentation
    cout << indentDots + node->label << "\n";
    // Recursively print the tree for the first child node, if it exists
    if (node->firstKid != NULL)
    {
        recursivelyPrintTree(node->firstKid, indentDots + ".");
    }
    // Recursively print the tree for the next sibling node, if it exists
    if (node->nextSibling != NULL)
    {
        recursivelyPrintTree(node->nextSibling, indentDots);
    }
}

void printString(std::string stringToPrint)
{
    // Loop through each character in the string
    for (size_t i = 0; i < stringToPrint.length(); i++)
    {
        // Check if the current character is '\' and the next character is 'n'
        if (stringToPrint.at(i) == '\\' && stringToPrint.at(i + 1) == 'n')
        {
            // Print a newline character and skip the next character
            cout << "\n";
            i++;
        }
        // Check if the current character is '\' and the next character is 't'
        else if (stringToPrint.at(i) == '\\' && stringToPrint.at(i + 1) == 't')
        {
            // Print a tab character and skip the next character
            cout << "\t";
            i++;
        }
        else
        {
            // Print the current character
            cout << stringToPrint.at(i);
        }
    }
}

void processCSEMachine()
{
    // Get the top element from the control stack and remove it from the stack
    CSEMachineNode controlTop = cseMachineControl.top();
    cseMachineControl.pop();

    // Check if controlTop is an integer, string, boolean, or dummy
    if (controlTop.isInt || controlTop.isString || controlTop.isBoolean ||
        controlTop.isDummy)
    { 
        // Push controlTop onto the CSE machine stack
        cseMachineStack.push(controlTop);
    }
    // Check if controlTop is a 'Y' combinator
    else if (controlTop.isY)
    {
        // Push controlTop onto the CSE machine stack
        cseMachineStack.push(controlTop);
    }
    // Check if controlTop is a tuple (only 'nil' tuple is allowed in control structure)
    else if (controlTop.isTuple)
    {
        // Push controlTop onto the CSE machine stack
        cseMachineStack.push(controlTop);
    }
    // Check if controlTop is a variable name
    else if (controlTop.isName)
    { 
        // Set isName to false as we are resolving the variable
        controlTop.isName = false;
        EnvironmentNode *environmentWithVariableValue = currentEnvironment;
        CSEMachineNode boundedValuesNode;
        bool variableValueFound = false;
        int indexOfBoundVariable = 0;

        // Traverse the environment tree to find the variable's value
        while (environmentWithVariableValue != NULL)
        {
            // Get the bounded values node from the current environment
            boundedValuesNode = environmentWithVariableValue->boundedValuesNode;
            for (int i = 0; i < boundedValuesNode.boundVariables.size(); i++)
            {
                // Check if the variable is found in the current environment
                if (boundedValuesNode.boundVariables[i] == controlTop.nameValue)
                {
                    indexOfBoundVariable = i;
                    variableValueFound = true;
                    break;
                }
            }
            if (variableValueFound)
            {
                break;
            }
            else
            {
                // Move to the parent environment
                environmentWithVariableValue = environmentWithVariableValue->parentEnvironment;
            }
        }

        // If variable is not found in any environment
        if (!variableValueFound)
        {
            // Check if it's a built-in function
            if (controlTop.nameValue == "Print" || controlTop.nameValue == "Conc" ||
                controlTop.nameValue == "Istuple" || controlTop.nameValue == "Isinteger" ||
                controlTop.nameValue == "Istruthvalue" || controlTop.nameValue == "Isstring" ||
                controlTop.nameValue == "Isfunction" || controlTop.nameValue == "Isdummy" ||
                controlTop.nameValue == "Stem" || controlTop.nameValue == "Stern" || controlTop.nameValue == "Order" ||
                controlTop.nameValue == "ItoS")
            {
                // Mark it as a built-in function and push it onto the stack
                controlTop.isBuiltInFunction = true;
                controlTop.defaultLabel = controlTop.nameValue;
                cseMachineStack.push(controlTop);
            }
            else
            {
                // Exit the program if the variable is not found and is not a built-in function
                exit(0);
            }
        }
        else
        {
            // Retrieve the value of the variable and push it onto the stack
            controlTop = environmentWithVariableValue->boundedValuesNode.tupleElements[indexOfBoundVariable];
            cseMachineStack.push(controlTop);
        }
    }


    else if (controlTop.isEnvironmentMarker)
{ 
    // CSE rule 5: Process environment markers
    CSEMachineNode stackTop = cseMachineStack.top();
    cseMachineStack.pop();
    if (!stackTop.isEnvironmentMarker)
    {
        // Handle case where the stack top is not an environment marker
        CSEMachineNode stackTopEnvironmentVariable = cseMachineStack.top();
        cseMachineStack.pop();
        if (!stackTopEnvironmentVariable.isEnvironmentMarker ||
            (controlTop.environmentMarkerIndex != stackTopEnvironmentVariable.environmentMarkerIndex))
        {
            // Error if environment markers do not match
            cout << "\n ERROR in resolving environment variables on control and stack! Die now! \n";
            exit(0);
        }
        cseMachineStack.push(stackTop);
    }
    else
    {
        // Check if environment marker indices match
        if (controlTop.environmentMarkerIndex != stackTop.environmentMarkerIndex)
        {
            // Error if environment markers do not match
            exit(0);
        }
    }
    // Set current environment to the previous environment
    currentEnvironment = environments[controlTop.environmentMarkerIndex]->previousEnvironment;
}
else if (controlTop.isLambda)
{   
    // CSE rule 2: Process lambda expressions
    controlTop.environmentMarkerIndex = currentEnvironment->environmentIndex; // index of environment in which this lambda holds
    cseMachineStack.push(controlTop);
}
else if (controlTop.isGamma)
{ 
    // CSE rule 3 & 4: Process gamma (function application)
    CSEMachineNode result = CSEMachineNode();
    CSEMachineNode operatorNode = cseMachineStack.top();
    cseMachineStack.pop();
    CSEMachineNode firstOperand = cseMachineStack.top();
    cseMachineStack.pop();
    if (operatorNode.isUnaryOperator || operatorNode.isBinaryOperator)
    { 
        // CSE rule 3: Process unary operators
        if (operatorNode.isUnaryOperator)
        {
            // Handle 'neg' operator for negation
            if (operatorNode.operatorStringValue == "neg")
            {
                if (!firstOperand.isInt)
                {
                    cout << "\n Operand is not int to apply 'neg', EXIT! \n";
                    exit(0);
                }
                else
                {
                    result.isInt = true;
                    result.intValue = -firstOperand.intValue;
                    result.defaultLabel = std::to_string(result.intValue);
                }
            }
            // Handle 'not' operator for boolean negation
            else if (operatorNode.operatorStringValue == "not")
            {
                if (!firstOperand.isBoolean)
                {
                    cout << "\n Operand is not boolean to apply 'not', EXIT! \n";
                    exit(0);
                }
                else
                {
                    result.isBoolean = true;
                    if (firstOperand.defaultLabel == "true")
                    {
                        result.defaultLabel = "false";
                    }
                    else if (firstOperand.defaultLabel == "false")
                    {
                        result.defaultLabel = "true";
                    }
                }
            }
            cseMachineStack.push(result);
        }
        // CSE rule 4: Process binary operators
        else if (operatorNode.isBinaryOperator)
        {
            CSEMachineNode secondOperand = cseMachineStack.top();
            cseMachineStack.pop();
            // Handle '**' operator for exponentiation
            if (operatorNode.operatorStringValue == "**")
            {
                if (!firstOperand.isInt || !secondOperand.isInt)
                {
                    cout << "\n operands not int for ** operation! exiting! \n";
                    exit(0);
                }
                else
                {
                    result.isInt = true;
                    result.intValue = pow(firstOperand.intValue, secondOperand.intValue);
                    result.defaultLabel = std::to_string(result.intValue);
                }
            }

                else if (operatorNode.operatorStringValue == "*")
{
    // Handle '*' operator for multiplication
    if (!firstOperand.isInt || !secondOperand.isInt)
    {
        cout << "\n operands not int for * operation! exiting! \n";
        exit(0);
    }
    else
    {
        result.isInt = true;
        result.intValue = firstOperand.intValue * secondOperand.intValue;
        result.defaultLabel = std::to_string(result.intValue);
    }
}
else if (operatorNode.operatorStringValue == "aug")
{
    // Handle 'aug' operator for augmenting tuples
    if (!firstOperand.isTuple)
    {
        cout << "\n first Operand is not a tuple for 'aug' operation! exiting! \n";
        exit(0);
    }
    else
    {
        result.isTuple = true;
        result.numberOfElementsInTauTuple = firstOperand.numberOfElementsInTauTuple + 1;

        if (firstOperand.numberOfElementsInTauTuple == 0)
        { 
            // If the first operand is nil
            result.tupleElements.push_back(secondOperand);
        }
        else
        {
            result.tupleElements = firstOperand.tupleElements;
            result.tupleElements.push_back(secondOperand);
        }
        result.defaultLabel = "TupleOfSize=" + std::to_string(result.numberOfElementsInTauTuple);
    }
}
else if (operatorNode.operatorStringValue == "-")
{
    // Handle '-' operator for subtraction
    if (!firstOperand.isInt || !secondOperand.isInt)
    {
        cout << "\n operands not int for - operation! exiting! \n";
        exit(0);
    }
    else
    {
        result.isInt = true;
        result.intValue = firstOperand.intValue - secondOperand.intValue;
        result.defaultLabel = std::to_string(result.intValue);
    }
}
else if (operatorNode.operatorStringValue == "+")
{
    // Handle '+' operator for addition
    if (!firstOperand.isInt || !secondOperand.isInt)
    {
        cout << "\n operands not int for + operation! exiting! \n";
        exit(0);
    }
    else
    {
        result.isInt = true;
        result.intValue = firstOperand.intValue + secondOperand.intValue;
        result.defaultLabel = std::to_string(result.intValue);
    }
}
else if (operatorNode.operatorStringValue == "/")
{
    // Handle '/' operator for division
    if (!firstOperand.isInt || !secondOperand.isInt)
    {
        cout << "\n operands not int for '/' operation! exiting! \n";
        exit(0);
    }
    else
    {
        result.isInt = true;
        result.intValue = firstOperand.intValue / secondOperand.intValue;
        result.defaultLabel = std::to_string(result.intValue);
    }
}

                else if (operatorNode.operatorStringValue == "gr")
{
    // Handle 'gr' (greater) operator for greater than comparison
    if (!firstOperand.isInt || !secondOperand.isInt)
    {
        // Exit if operands are not integers
        cout << "\n operands not int for 'gr' operation! exiting! \n";
        exit(0);
    }
    else
    {
        // Set the result to boolean indicating whether the first operand is greater than the second operand
        result.isInt = false; // Result is not an integer
        result.isBoolean = true; // Result is a boolean
        result.defaultLabel = firstOperand.intValue > secondOperand.intValue ? "true" : "false";
    }
}
else if (operatorNode.operatorStringValue == "ge")
{
    // Handle 'ge' (greater or equal) operator for greater than or equal comparison
    if (!firstOperand.isInt || !secondOperand.isInt)
    {
        // Exit if operands are not integers
        cout << "\n operands not int for 'ge' operation! exiting! \n";
        exit(0);
    }
    else
    {
        // Set the result to boolean indicating whether the first operand is greater than or equal to the second operand
        result.isInt = false; // Result is not an integer
        result.isBoolean = true; // Result is a boolean
        result.defaultLabel = firstOperand.intValue >= secondOperand.intValue ? "true" : "false";
    }
}
else if (operatorNode.operatorStringValue == "ls")
{
    // Handle 'ls' (less) operator for less than comparison
    if (!firstOperand.isInt || !secondOperand.isInt)
    {
        // Exit if operands are not integers
        cout << "\n operands not int for 'ls' operation! exiting! \n";
        exit(0);
    }
    else
    {
        // Set the result to boolean indicating whether the first operand is less than the second operand
        result.isInt = false; // Result is not an integer
        result.isBoolean = true; // Result is a boolean
        result.defaultLabel = firstOperand.intValue < secondOperand.intValue ? "true" : "false";
    }
}
else if (operatorNode.operatorStringValue == "le")
{
    // Handle 'le' (less or equal) operator for less than or equal comparison
    if (!firstOperand.isInt || !secondOperand.isInt)
    {
        // Exit if operands are not integers
        cout << "\n operands not int for 'le' operation! exiting! \n";
        exit(0);
    }
    else
    {
        // Set the result to boolean indicating whether the first operand is less than or equal to the second operand
        result.isInt = false; // Result is not an integer
        result.isBoolean = true; // Result is a boolean
        result.defaultLabel = firstOperand.intValue <= secondOperand.intValue ? "true" : "false";
    }
}
else if (operatorNode.operatorStringValue == "eq")
{
    // Handle 'eq' (equal) operator for equality comparison
    if (!((!firstOperand.isInt || !secondOperand.isInt) &&
          (!firstOperand.isBoolean || !secondOperand.isBoolean) &&
          (!firstOperand.isString || !secondOperand.isString)))
    {
        // Exit if operands are not of the same type or if they are not int, boolean, or string
        cout << "\n operands not of same type for 'eq' operation! exiting! \n";
        exit(0);
    }
    else
    {
        // Set the result to boolean indicating whether the first operand is equal to the second operand
        result.isInt = false; // Result is not an integer
        result.isBoolean = true; // Result is a boolean
        // Check equality based on operand type
        if (firstOperand.isInt)
        {
            // Compare integer values
            result.defaultLabel = firstOperand.intValue == secondOperand.intValue ? "true" : "false";
        }
        else if (firstOperand.isBoolean)
        {
            // Compare boolean values
            result.defaultLabel = firstOperand.defaultLabel == secondOperand.defaultLabel ? "true" : "false";
        }
        else if (firstOperand.isString)
        {
            // Compare string values
            result.defaultLabel = firstOperand.stringValue == secondOperand.stringValue ? "true" : "false";
        }
    }
}

               else if (operatorNode.operatorStringValue == "ne")
{
    // Handle 'ne' (not equal) operator for inequality comparison
    if (!((!firstOperand.isInt || !secondOperand.isInt) ||
          (!firstOperand.isBoolean || !secondOperand.isBoolean) ||
          (!firstOperand.isString || !secondOperand.isString)))
    {
        // Exit if operands are not of the same type or if they are not int, boolean, or string
        cout << "\n operands not of same type for 'ne' operation! exiting! \n";
        exit(0);
    }
    else
    {
        // Set the result to boolean indicating whether the first operand is not equal to the second operand
        result.isInt = false; // Result is not an integer
        result.isBoolean = true; // Result is a boolean
        // Check inequality based on operand type
        if (firstOperand.isInt)
        {
            // Compare integer values
            result.defaultLabel = firstOperand.intValue != secondOperand.intValue ? "true" : "false";
        }
        else if (firstOperand.isBoolean)
        {
            // Compare boolean values
            result.defaultLabel = firstOperand.defaultLabel != secondOperand.defaultLabel ? "true" : "false";
        }
        else if (firstOperand.isString)
        {
            // Compare string values
            result.defaultLabel = firstOperand.stringValue != secondOperand.stringValue ? "true" : "false";
        }
    }
}
else if (operatorNode.operatorStringValue == "or")
{
    // Handle 'or' operator for logical OR operation
    if (!firstOperand.isBoolean || !secondOperand.isBoolean)
    {
        // Exit if operands are not boolean
        cout << "\n operands are not boolean for 'or' operation! exiting! \n";
        exit(0);
    }
    else
    {
        // Set the result to boolean indicating the logical OR of the operands
        result.isInt = false; // Result is not an integer
        result.isBoolean = true; // Result is a boolean
        result.defaultLabel = (firstOperand.defaultLabel == "true" || secondOperand.defaultLabel == "true") ? "true" : "false";
    }
}
else if (operatorNode.operatorStringValue == "&")
{
    // Handle '&' operator for logical AND operation
    if (!firstOperand.isBoolean || !secondOperand.isBoolean)
    {
        // Exit if operands are not boolean
        cout << "\n operands are not boolean for '&' operation! exiting! \n";
        exit(0);
    }
    else
    {
        // Set the result to boolean indicating the logical AND of the operands
        result.isInt = false; // Result is not an integer
        result.isBoolean = true; // Result is a boolean
        result.defaultLabel = (firstOperand.defaultLabel == "true" && secondOperand.defaultLabel == "true") ? "true" : "false";
    }
}
cseMachineStack.push(result);
}
}
else if (operatorNode.isLambda)
{ // Handle lambda expressions, CSE rule 4

    
    // Create a new environment marker for the lambda and add it to the control
    CSEMachineNode newEnvironmentVariableForCurrentLambda = CSEMachineNode();
    newEnvironmentVariableForCurrentLambda.isEnvironmentMarker = true;
    newEnvironmentVariableForCurrentLambda.environmentMarkerIndex = environmentCounter++;
    newEnvironmentVariableForCurrentLambda.defaultLabel =
        "e" + std::to_string(newEnvironmentVariableForCurrentLambda.environmentMarkerIndex);
    cseMachineControl.push(newEnvironmentVariableForCurrentLambda);

    // Update the current environment to include the new environment for the lambda
    EnvironmentNode *newEnvironmentForCurrentLambda = new EnvironmentNode();
    newEnvironmentForCurrentLambda->parentEnvironment = environments[operatorNode.environmentMarkerIndex];
    newEnvironmentForCurrentLambda->previousEnvironment = currentEnvironment;
    currentEnvironment = newEnvironmentForCurrentLambda;
    newEnvironmentForCurrentLambda->environmentIndex = newEnvironmentVariableForCurrentLambda.environmentMarkerIndex;
    newEnvironmentForCurrentLambda->boundedValuesNode = CSEMachineNode();
    newEnvironmentForCurrentLambda->boundedValuesNode.boundVariables = operatorNode.boundVariables;
    environments[newEnvironmentForCurrentLambda->environmentIndex] = newEnvironmentForCurrentLambda;

    // Handle variable bindings for the new environment
    if (operatorNode.boundVariables.size() == 1)
    { // Only one bound variable, store the first operand directly
        // First operand could be int/string/tuple
        newEnvironmentForCurrentLambda->boundedValuesNode.tupleElements.push_back(firstOperand);
    }
    else
    { // Multiple variable bindings, the first operand must be a tuple
        // CSE Rule 11 (n-ary function)
        newEnvironmentForCurrentLambda->boundedValuesNode.tupleElements = firstOperand.tupleElements;
    }

    // Add the new environment marker to the stack
    cseMachineStack.push(newEnvironmentVariableForCurrentLambda);

   
    // Add the lambda's control structure to the control stack
    std::list<CSEMachineNode>::const_iterator iterator;
    for (iterator = controlStructures[operatorNode.indexOfBodyOfLambda].begin();
         iterator != controlStructures[operatorNode.indexOfBodyOfLambda].end(); ++iterator)
    {
        CSEMachineNode controlStructureToken = *iterator;
        cseMachineControl.push(controlStructureToken);
    }
    
}

        else if (operatorNode.isY)
{ 
    // CSE rule 12: Handling the Y combinator
    firstOperand.isYF = true; // Mark the first operand as a YF (fixed-point) function
    firstOperand.isLambda = false; // It is not a lambda function
    cseMachineStack.push(firstOperand); // Push the modified operand onto the stack
}
else if (operatorNode.isYF)
{ 
    // CSE rule 13: Handling the fixed-point application
    cseMachineStack.push(firstOperand); // Push the first operand onto the stack
    cseMachineStack.push(operatorNode); // Push the operator node onto the stack
    CSEMachineNode lambdaNode = operatorNode;
    lambdaNode.isYF = false; // Mark it as a lambda function
    lambdaNode.isLambda = true; // Set it as a lambda
    cseMachineStack.push(lambdaNode); // Push the lambda node onto the stack
    CSEMachineNode gammaNode = CSEMachineNode();
    gammaNode.isGamma = true; // Create a gamma node
    gammaNode.defaultLabel = "gamma"; // Label it as "gamma"
    cseMachineControl.push(gammaNode); // Push the gamma node onto the control
    cseMachineControl.push(gammaNode); // Push another gamma node onto the control
}
else if (operatorNode.isBuiltInFunction)
{
    // Handling built-in functions
    if (operatorNode.defaultLabel == "Print")
    {
        // Handle the 'Print' built-in function
        if (firstOperand.isBoolean)
        {
            // Print boolean value
            cout << firstOperand.defaultLabel;
        }
        else if (firstOperand.isInt)
        {
            // Print integer value
            cout << firstOperand.intValue;
        }
        else if (firstOperand.isString)
        {
            // Print string value
            printString(firstOperand.stringValue);
        }
        else if (firstOperand.isDummy)
        {
            // Do nothing for dummy values
        }
        else if (firstOperand.isTuple)
        {
            // Handle printing of tuples
            if (firstOperand.tupleElements.size() == 0)
            {
                cout << "nil"; // Print "nil" for empty tuples
            }
            else
            {
                cout << "(";
                for (int i = 0; i < firstOperand.tupleElements.size(); i++)
                {
                    // Print each element in the tuple
                    if (firstOperand.tupleElements[i].isBoolean)
                    {
                        cout << firstOperand.tupleElements[i].defaultLabel;
                    }
                    else if (firstOperand.tupleElements[i].isInt)
                    {
                        cout << firstOperand.tupleElements[i].intValue;
                    }
                    else if (firstOperand.tupleElements[i].isString)
                    {
                        printString(firstOperand.tupleElements[i].stringValue);
                    }
                    if (i + 1 != firstOperand.tupleElements.size())
                    {
                        cout << ", ";
                    }
                }
                cout << ")";
            }
        }
        else if (firstOperand.isLambda)
        {
            // Print lambda closure information
            cout << "[lambda closure: " + firstOperand.boundVariables[0] + ": " +
                        std::to_string(firstOperand.indexOfBodyOfLambda) + "]";
        }
        else
        {
            // Print error message for unsupported types
            cout << "\n\n ERROR! I don't know how to PRINT the value on stack= " + firstOperand.defaultLabel + "\n\n";
            exit(0);
        }
    }
    else if (operatorNode.defaultLabel == "Conc")
    {
        // Handle the 'Conc' built-in function for string concatenation
        cseMachineControl.pop(); // Pop out the second gamma node
        CSEMachineNode secondOperand = cseMachineStack.top();
        cseMachineStack.pop();
        result.isString = true; // Result is a string
        result.stringValue = firstOperand.stringValue + secondOperand.stringValue; // Concatenate strings
        cseMachineStack.push(result); // Push the result onto the stack
    }
    else if (operatorNode.defaultLabel == "Order")
    {
        // Handle the 'Order' built-in function to get the order of a tuple
        if (!firstOperand.isTuple)
        {
            cout << "\n\n Error! can't apply 'Order' to a datatype other than tuple! DIE NO! \n\n ";
            exit(0);
        }
        else
        {
            result.isInt = true; // Result is an integer
            result.intValue = firstOperand.numberOfElementsInTauTuple; // Get the number of elements in the tuple
            result.defaultLabel = std::to_string(result.intValue); // Set the default label to the integer value
            cseMachineStack.push(result); // Push the result onto the stack
        }
    }
    else if (operatorNode.defaultLabel == "Stem")
    {
        // Handle the 'Stem' built-in function to get the first character of a string
        result.isString = true; // Result is a string
        result.stringValue = firstOperand.stringValue[0]; // Get the first character of the string
        cseMachineStack.push(result); // Push the result onto the stack
    }
    else if (operatorNode.defaultLabel == "Stern")
    {
        // Handle the 'Stern' built-in function to get the substring excluding the first character
        result.isString = true; // Result is a string
        result.stringValue = firstOperand.stringValue.substr(1); // Get the substring excluding the first character
        cseMachineStack.push(result); // Push the result onto the stack
    }
    else if (operatorNode.defaultLabel == "Isstring")
    {
        // Handle the 'Isstring' built-in function to check if the operand is a string
        result.isBoolean = true; // Result is a boolean
        result.defaultLabel = firstOperand.isString ? "true" : "false"; // Check if the operand is a string
        cseMachineStack.push(result); // Push the result onto the stack
    }
    else if (operatorNode.defaultLabel == "Istuple")
    {
        // Handle the 'Istuple' built-in function to check if the operand is a tuple
        result.isBoolean = true; // Result is a boolean
        result.defaultLabel = firstOperand.isTuple ? "true" : "false"; // Check if the operand is a tuple
        cseMachineStack.push(result); // Push the result onto the stack
    }
    else if (operatorNode.defaultLabel == "Isinteger")
    {
        // Handle the 'Isinteger' built-in function to check if the operand is an integer
        result.isBoolean = true; // Result is a boolean
        result.defaultLabel = firstOperand.isInt ? "true" : "false"; // Check if the operand is an integer
        cseMachineStack.push(result); // Push the result onto the stack
    }
    else if (operatorNode.defaultLabel == "ItoS")
    {
        // Handle the 'ItoS' built-in function to convert an integer to a string
        if (!firstOperand.isInt)
        {
            cout << "ERROR! operand to ItoS is not Int! DIE NOW!";
            exit(0);
        }
        result.isString = true; // Result is a string
        result.defaultLabel = std::to_string(firstOperand.intValue); // Convert the integer to a string
        result.stringValue = std::to_string(firstOperand.intValue); // Set the string value
        cseMachineStack.push(result); // Push the result onto the stack
    }
    else if (operatorNode.defaultLabel == "Istruthvalue")
    {
        // Handle the 'Istruthvalue' built-in function to check if the operand is a truth value (boolean)
        result.isBoolean = true; // Result is a boolean
        result.defaultLabel = firstOperand.isBoolean ? "true" : "false"; // Check if the operand is a boolean
        cseMachineStack.push(result); // Push the result onto the stack
    }
    else
    {
        // Print error message for undefined built-in functions
        cout << "\n\n AYO!! I haven't defined the behavior of the function= " + operatorNode.defaultLabel + "\n\n";
        exit(0);
    }
}

        else if (operatorNode.isTuple)
{ // CSE rule 10 for Tuple selection
    result = operatorNode.tupleElements[firstOperand.intValue - 1]; // Select the appropriate tuple element
    cseMachineStack.push(result); // Push the selected element to the stack
}
}
else if (controlTop.isBinaryOperator)
{ // CSE rule 6 for binary operations
    CSEMachineNode result = CSEMachineNode();
    CSEMachineNode operatorNode = controlTop;
    CSEMachineNode firstOperand = cseMachineStack.top();
    cseMachineStack.pop();
    CSEMachineNode secondOperand = cseMachineStack.top();
    cseMachineStack.pop();
    if (operatorNode.operatorStringValue == "**")
    {
        if (!firstOperand.isInt || !secondOperand.isInt)
        { // Check if operands are integers
            cout << "\n operands not int for '**' operation! exiting! \n";
            exit(0);
        }
        else
        {
            result.isInt = true;
            result.intValue = pow(firstOperand.intValue, secondOperand.intValue); // Perform exponentiation
            result.defaultLabel = std::to_string(result.intValue);
        }
    }
    else if (operatorNode.operatorStringValue == "*")
    {
        if (!firstOperand.isInt || !secondOperand.isInt)
        { // Check if operands are integers
            cout << "\n operands not int for '*' operation! exiting! \n";
            exit(0);
        }
        else
        {
            result.isInt = true;
            result.intValue = firstOperand.intValue * secondOperand.intValue; // Perform multiplication
            result.defaultLabel = std::to_string(result.intValue);
        }
    }
    else if (operatorNode.operatorStringValue == "aug")
    {
        if (!firstOperand.isTuple)
        { // Check if the first operand is a tuple
            cout << "\n first Operand is not a tuple for 'aug' operation! exiting! \n";
            exit(0);
        }
        else
        {
            result.isTuple = true;
            result.numberOfElementsInTauTuple = firstOperand.numberOfElementsInTauTuple + 1;
            if (firstOperand.numberOfElementsInTauTuple == 0)
            { // If the first operand is an empty tuple
                result.tupleElements.push_back(secondOperand); // Augment the second operand
            }
            else
            {
                result.tupleElements = firstOperand.tupleElements;
                result.tupleElements.push_back(secondOperand); // Augment the second operand
            }
            result.defaultLabel = "TupleOfSize=" + std::to_string(result.numberOfElementsInTauTuple);
        }
    }
    else if (operatorNode.operatorStringValue == "-")
    {
        if (!firstOperand.isInt || !secondOperand.isInt)
        { // Check if operands are integers
            cout << "\n operands not int for '-' operation! exiting! \n";
            exit(0);
        }
        else
        {
            result.isInt = true;
            result.intValue = firstOperand.intValue - secondOperand.intValue; // Perform subtraction
            result.defaultLabel = std::to_string(result.intValue);
        }
    }
    else if (operatorNode.operatorStringValue == "+")
    {
        if (!firstOperand.isInt || !secondOperand.isInt)
        { // Check if operands are integers
            cout << "\n operands not int for '+' operation! exiting! \n";
            exit(0);
        }
        else
        {
            result.isInt = true;
            result.intValue = firstOperand.intValue + secondOperand.intValue; // Perform addition
            result.defaultLabel = std::to_string(result.intValue);
        }
    }
    else if (operatorNode.operatorStringValue == "/")
    {
        if (!firstOperand.isInt || !secondOperand.isInt)
        { // Check if operands are integers
            cout << "\n operands not int for '/' operation! exiting! \n";
            exit(0);
        }
        else
        {
            result.isInt = true;
            result.intValue = firstOperand.intValue / secondOperand.intValue; // Perform division
            result.defaultLabel = std::to_string(result.intValue);
        }
    }
    else if (operatorNode.operatorStringValue == "gr")
    {
        if (!firstOperand.isInt || !secondOperand.isInt)
        { // Check if operands are integers
            cout << "\n operands not int for 'gr' operation! exiting! \n";
            exit(0);
        }
        else
        {
            result.isInt = false;
            result.isBoolean = true;
            result.defaultLabel = firstOperand.intValue > secondOperand.intValue ? "true" : "false"; // Greater than comparison
        }
    }
    else if (operatorNode.operatorStringValue == "ge")
    {
        if (!firstOperand.isInt || !secondOperand.isInt)
        { // Check if operands are integers
            cout << "\n operands not int for 'ge' operation! exiting! \n";
            exit(0);
        }
        else
        {
            result.isInt = false;
            result.isBoolean = true;
            result.defaultLabel = firstOperand.intValue >= secondOperand.intValue ? "true" : "false"; // Greater or equal comparison
        }
    }
    else if (operatorNode.operatorStringValue == "ls")
    {
        if (!firstOperand.isInt || !secondOperand.isInt)
        { // Check if operands are integers
            cout << "\n operands not int for 'ls' operation! exiting! \n";
            exit(0);
        }
        else
        {
            result.isInt = false;
            result.isBoolean = true;
            result.defaultLabel = firstOperand.intValue < secondOperand.intValue ? "true" : "false"; // Less than comparison
        }
    }
    else if (operatorNode.operatorStringValue == "le")
    {
        if (!firstOperand.isInt || !secondOperand.isInt)
        { // Check if operands are integers
            cout << "\n operands not int for 'le' operation! exiting! \n";
            exit(0);
        }
        else
        {
            result.isInt = false;
            result.isBoolean = true;
            result.defaultLabel = firstOperand.intValue <= secondOperand.intValue ? "true" : "false"; // Less or equal comparison
        }
    }
    else if (operatorNode.operatorStringValue == "eq")
    {
        if (!((!firstOperand.isInt || !secondOperand.isInt) ||
            (!firstOperand.isBoolean || !secondOperand.isBoolean) ||
            (!firstOperand.isString || !secondOperand.isString)))
        { // Check if operands are of the same type
            cout << "\n operands not of same type for 'eq' operation! exiting! \n";
            exit(0);
        }
        else
        {
            result.isInt = false;
            result.isBoolean = true;
            if (firstOperand.isInt)
            {
                result.defaultLabel = firstOperand.intValue == secondOperand.intValue ? "true" : "false"; // Equality comparison for integers
            }
            else if (firstOperand.isBoolean)
            {
                result.defaultLabel = firstOperand.defaultLabel == secondOperand.defaultLabel ? "true" : "false"; // Equality comparison for booleans
            }
            else if (firstOperand.isString)
            {
                result.defaultLabel = firstOperand.stringValue == secondOperand.stringValue ? "true" : "false"; // Equality comparison for strings
            }
        }
    }
    else if (operatorNode.operatorStringValue == "ne")
    {
        if (!((!firstOperand.isInt || !secondOperand.isInt) ||
            (!firstOperand.isBoolean || !secondOperand.isBoolean) ||
            (!firstOperand.isString || !secondOperand.isString)))
        { // Check if operands are of the same type
            cout << "\n operands not of same type for 'ne' operation! exiting! \n";
            exit(0);
        }
        else
        {
            result.isInt = false;
            result.isBoolean = true;
            if (firstOperand.isInt)
            {
                result.defaultLabel = firstOperand.intValue != secondOperand.intValue ? "true" : "false"; // Inequality comparison for integers
            }
            else if (firstOperand.isBoolean)
            {
                result.defaultLabel = firstOperand.defaultLabel != secondOperand.defaultLabel ? "true" : "false"; // Inequality comparison for booleans
            }
            else if (firstOperand.isString)
            {
                result.defaultLabel = firstOperand.stringValue != secondOperand.stringValue ? "true" : "false"; // Inequality comparison for strings
            }
        }
    }
    else if (operatorNode.operatorStringValue == "or")
    {
        if (!firstOperand.isBoolean || !secondOperand.isBoolean)
        { // Check if operands are booleans
            cout << "\n operands are not boolean for 'or' operation! exiting! \n";
            exit(0);
        }
        else
        {
            result.isInt = false;
            result.isBoolean = true;
            result.defaultLabel = (firstOperand.defaultLabel == "true" || secondOperand.defaultLabel == "true")
                ? "true"
                : "false"; // Logical OR operation
        }
    }
    else if (operatorNode.operatorStringValue == "&")
    {
        if (!firstOperand.isBoolean || !secondOperand.isBoolean)
        { // Check if operands are booleans
            cout << "\n operands are not boolean for '&' operation! exiting! \n";
            exit(0);
        }
        else
        {
            result.isInt = false;
            result.isBoolean = true;
            result.defaultLabel = (firstOperand.defaultLabel == "true" && secondOperand.defaultLabel == "true")
                ? "true"
                : "false"; // Logical AND operation
        }
    }
    cseMachineStack.push(result); // Push the result to the stack
}
else if (controlTop.isUnaryOperator)
{ // CSE rule 7 for unary operations
    CSEMachineNode result = CSEMachineNode();
    CSEMachineNode operatorNode = controlTop;
    CSEMachineNode firstOperand = cseMachineStack.top();
    cseMachineStack.pop();
    if (operatorNode.operatorStringValue == "neg")
    {
        if (!firstOperand.isInt)
        { // Check if the operand is an integer
            cout << "\n Operand is not int to apply 'neg', EXIT! \n";
            exit(0);
        }
        else
        {
            result.isInt = true;
            result.intValue = -firstOperand.intValue; // Perform negation
            result.defaultLabel = std::to_string(result.intValue);
        }
    }
    else if (operatorNode.operatorStringValue == "not")
    {
        if (!firstOperand.isBoolean)
        { // Check if the operand is boolean
            cout << "\n Operand is not boolean to apply not, EXIT! \n";
            exit(0);
        }
        else
        {
            result.isBoolean = true;
            if (firstOperand.defaultLabel == "true")
            {
                result.defaultLabel = "false"; // Logical NOT operation
            }
            else if (firstOperand.defaultLabel == "false")
            {
                result.defaultLabel = "true"; // Logical NOT operation
            }
        }
    }
    cseMachineStack.push(result); // Push the result to the stack
}
else if (controlTop.isConditional)
{ // CSE rule 8 for conditional operations
    CSEMachineNode booleanNode = cseMachineStack.top();
    cseMachineStack.pop();
    CSEMachineNode falseNode = cseMachineControl.top();
    cseMachineControl.pop();
    CSEMachineNode trueNode = cseMachineControl.top();
    cseMachineControl.pop();
    int controlStructureIndexOfChosenConditional;
    if (booleanNode.defaultLabel == "true")
    { // Choose the true control structure
        controlStructureIndexOfChosenConditional = trueNode.indexOfBodyOfLambda;
    }
    else if (booleanNode.defaultLabel == "false")
    { // Choose the false control structure
        controlStructureIndexOfChosenConditional = falseNode.indexOfBodyOfLambda;
    }
    // Push the chosen control structure's elements
    std::list<CSEMachineNode>::const_iterator iterator;
    for (iterator = controlStructures[controlStructureIndexOfChosenConditional].begin();
        iterator != controlStructures[controlStructureIndexOfChosenConditional].end(); ++iterator)
    {
        CSEMachineNode controlStructureToken = *iterator;
        cseMachineControl.push(controlStructureToken);
    }
}
else if (controlTop.isTau)
{ // CSE rule 9 for Tau tuple formation on CSE's stack structure
    int numberOfTupleElements = controlTop.numberOfElementsInTauTuple;
    // TODO: This checking for if the popped elements are environmentMarkers and working around it was added to handle the 'recurs.1'
    // program. In that program, the tau selection didn't have enough elements for it on the stack. Is this the actual way to do it?
    stack<CSEMachineNode> environmentVariablesToBePushedBackToStack;
    while (numberOfTupleElements > 0 && !cseMachineStack.empty())
    {
        CSEMachineNode poppedStackElement = cseMachineStack.top();
        cseMachineStack.pop();
        if (!poppedStackElement.isEnvironmentMarker)
        {
            numberOfTupleElements--;
            controlTop.tupleElements.push_back(poppedStackElement); // Add elements to the tuple
        }
        else
        {
            environmentVariablesToBePushedBackToStack.push(poppedStackElement); // Handle environment markers
        }
    }
    controlTop.isTau = false;
    controlTop.isTuple = true;
    controlTop.defaultLabel = "TupleOfSize=" + std::to_string(controlTop.tupleElements.size());
    controlTop.numberOfElementsInTauTuple = controlTop.tupleElements.size();
    while (!environmentVariablesToBePushedBackToStack.empty())
    {
        cseMachineStack.push(environmentVariablesToBePushedBackToStack.top());
        environmentVariablesToBePushedBackToStack.pop();
    }
    cseMachineStack.push(controlTop); // Push the tuple to the stack
}
}

void runCSEMachine()
{
    initializeCSEMachine();
    while (!cseMachineControl.empty())
    {
        processCSEMachine(); // Process the value on top of the control stack one by one
        // according to the rules of the CSE machine
    }
}
