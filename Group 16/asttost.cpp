#include "parser.hpp"
#include "lexicon.hpp"
#include "asttost.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <stack>
#include <list>

void convertFunctionForm(Node *functionFormNode)
{
    // Create a new node for the "lambda" right child node header of the final standardized sub-tree
    Node *fcnLambdaRightChildNodeHeader = new Node;
    fcnLambdaRightChildNodeHeader->label = LAMBDA_STD_LABEL;
    fcnLambdaRightChildNodeHeader->nextSibling = NULL;

    // List to store nodes representing function variables
    list<Node *> fcnVariableList;

    // Set the label of the root node to "=" (the header node of the final standardized sub-tree)
    functionFormNode->label = "=";

    // Traverse the tree to collect nodes representing function variables
    Node *temp = functionFormNode->firstKid;
    while (temp->nextSibling->nextSibling != NULL)
    {
        temp = temp->nextSibling;
        fcnVariableList.push_back(temp);
    }

    // Modify the structure of the tree to standardize the function form
    functionFormNode->firstKid->nextSibling = fcnLambdaRightChildNodeHeader;
    fcnLambdaRightChildNodeHeader->firstKid = fcnVariableList.front();
    fcnVariableList.pop_front();

    // Create new "lambda" nodes for each function variable
    Node *lambdaTemp = fcnLambdaRightChildNodeHeader;
    while (fcnVariableList.size() > 0)
    {
        Node *newLambdaRightNode = new Node;
        lambdaTemp->firstKid->nextSibling = newLambdaRightNode;
        newLambdaRightNode->label = LAMBDA_STD_LABEL;
        newLambdaRightNode->nextSibling = NULL;
        lambdaTemp = newLambdaRightNode;
        lambdaTemp->firstKid = fcnVariableList.front();
        fcnVariableList.pop_front();
    }
}

void convertInfixOperator(Node *infixOperatorNode)
{
    // Create nodes to standardize infix operator expression
    Node *leftGammaChild = new Node;
    Node *leftLeftOperatorChild = new Node;
    
    // Set the label of leftLeftOperatorChild to the operator label
    leftLeftOperatorChild->label = infixOperatorNode->firstKid->nextSibling->label;
    // Connect the leftLeftOperatorChild to the left expression node (E1)
    leftLeftOperatorChild->nextSibling = infixOperatorNode->firstKid;
    leftLeftOperatorChild->firstKid = NULL;

    // Create a node for the left gamma child
    leftGammaChild->label = GAMMA_STD_LABEL;
    leftGammaChild->firstKid = leftLeftOperatorChild;
    // Connect the left gamma child to the right expression node (E2)
    leftGammaChild->nextSibling = infixOperatorNode->firstKid->nextSibling->nextSibling;

    // Modify the structure of the infix operator node to standardize it
    infixOperatorNode->firstKid->nextSibling = NULL;
    infixOperatorNode->firstKid = leftGammaChild;
    infixOperatorNode->label = GAMMA_STD_LABEL;
}


void convertLambdaExpression(Node *lambdaNode)
{
    // Standardize the label of the lambda node
    lambdaNode->label = LAMBDA_STD_LABEL;

    // List to store nodes representing function variables
    list<Node *> fcnVariableList;

    // Traverse the tree to collect nodes representing function variables
    Node *temp = lambdaNode->firstKid;
    while (temp->nextSibling->nextSibling != NULL)
    {
        temp = temp->nextSibling;
        fcnVariableList.push_back(temp);
    }
    
    // Retrieve the expression node which is the rightmost child of the right sub-tree
    temp = temp->nextSibling;

    // Update the structure of the lambda node to standardize it
    Node *lambdaTemp = lambdaNode;
    while (fcnVariableList.size() > 0)
    {
        Node *newLambdaRightNode = new Node;
        lambdaTemp->firstKid->nextSibling = newLambdaRightNode;
        newLambdaRightNode->nextSibling = NULL;
        newLambdaRightNode->label = LAMBDA_STD_LABEL;
        lambdaTemp = newLambdaRightNode;
        lambdaTemp->firstKid = fcnVariableList.front();
        fcnVariableList.pop_front();
    }
    // Connect the last standardized lambda node to the expression node (E)
    lambdaTemp->firstKid->nextSibling = temp;
}

void convertRecExpression(Node *recNode)
{
    // Retrieve the original child node of the "=" (equals) node
    Node *recNodeOriginalEqualsChild = recNode->firstKid;

    // Update the label of the recNode to match the original "=" node
    recNode->label = recNodeOriginalEqualsChild->label;
    // Set the first kid of recNode to match the original first kid of "=" node
    recNode->firstKid = recNodeOriginalEqualsChild->firstKid;

    // Create nodes for standardizing recursive function expression
    Node *rightGammaChild = new Node;
    rightGammaChild->label = GAMMA_STD_LABEL;
    rightGammaChild->nextSibling = NULL;

    Node *rightRightLambdaChild = new Node;
    rightRightLambdaChild->label = LAMBDA_STD_LABEL;
    rightRightLambdaChild->nextSibling = NULL;

    Node *leftChildYNode = new Node;
    leftChildYNode->label = "Y";
    leftChildYNode->firstKid = NULL;

    // Set up the right gamma child node
    rightGammaChild->firstKid = leftChildYNode;
    leftChildYNode->nextSibling = rightRightLambdaChild;

    Node *functionNameNode = new Node;
    functionNameNode->label = recNode->firstKid->label;
    functionNameNode->firstKid = NULL;

    // Set up the right lambda child node
    rightRightLambdaChild->firstKid = functionNameNode;
    functionNameNode->nextSibling = recNode->firstKid->nextSibling; // E

    // Modify the structure of the recNode to standardize it
    recNode->firstKid->nextSibling = rightGammaChild;
}


void convertWhereExpression(Node *whereNode)
{
    // Set the label of the node to indicate it's a standard gamma expression
    whereNode->label = GAMMA_STD_LABEL;

    // Extract necessary nodes from the where expression
    Node *pNode = whereNode->firstKid;
    Node *leftChildLambdaNode = pNode->nextSibling;
    leftChildLambdaNode->label = LAMBDA_STD_LABEL; // Standardize the left child lambda node
    Node *eNode = leftChildLambdaNode->firstKid->nextSibling;

    // Rearrange the structure of the where expression to standardize it
    whereNode->firstKid = leftChildLambdaNode;

    // Swap the positions of p and e nodes
    leftChildLambdaNode->nextSibling = eNode;
    leftChildLambdaNode->firstKid->nextSibling = pNode;
    pNode->nextSibling = NULL;
}

void convertWithinExpression(Node *withinNode)
{
    // Set the label of the node to indicate it's a standard equals expression
    withinNode->label = "=";

    // Extract nodes from the within expression
    Node *withinOne = withinNode->firstKid;
    Node *withinTwo = withinOne->nextSibling;

    // Create necessary nodes for standardization
    Node *rightGammaChild = new Node;
    Node *rightLeftLambdaChild = new Node;

    // Set labels for new nodes
    rightGammaChild->label = GAMMA_STD_LABEL;
    rightLeftLambdaChild->label = LAMBDA_STD_LABEL;

    // Rearrange the structure of the within expression to standardize it
    rightGammaChild->firstKid = rightLeftLambdaChild;
    rightLeftLambdaChild->nextSibling = withinOne->firstKid->nextSibling;           
    rightLeftLambdaChild->firstKid = withinOne->firstKid;                           
    rightLeftLambdaChild->firstKid->nextSibling = withinTwo->firstKid->nextSibling; 

    withinNode->firstKid = withinTwo->firstKid; 
    withinNode->firstKid->nextSibling = rightGammaChild;
}

void convertAndExpression(Node *andHeaderNode)
{
    // Set the label of the node to indicate it's a standard equals expression
    andHeaderNode->label = "=";

    // Extract nodes from the and expression
    Node *tempEqualsChildHeaderNode = andHeaderNode->firstKid;

    // Create lists to hold variable and expression nodes
    list<Node *> variableNodesList;
    list<Node *> expressionNodesList;

    // Populate the lists with nodes from the and expression
    while (tempEqualsChildHeaderNode != NULL)
    {
        variableNodesList.push_back(tempEqualsChildHeaderNode->firstKid);
        expressionNodesList.push_back(tempEqualsChildHeaderNode->firstKid->nextSibling);
        tempEqualsChildHeaderNode = tempEqualsChildHeaderNode->nextSibling;
    }

    // Create nodes for standardization
    Node *commaHeaderNode = new Node;
    Node *tauHeaderNode = new Node;

    // Set labels for new nodes
    commaHeaderNode->label = ",";
    tauHeaderNode->label = "tau";
    tauHeaderNode->nextSibling = NULL;
    commaHeaderNode->nextSibling = tauHeaderNode;

    // Rearrange the structure of the and expression to standardize it
    andHeaderNode->firstKid = commaHeaderNode;

    Node *commaVariableTempNode, *tauExpressionTempNode;

    // Initialize temporary nodes to the first nodes in the lists
    commaVariableTempNode = variableNodesList.front();
    variableNodesList.pop_front();
    tauExpressionTempNode = expressionNodesList.front();
    expressionNodesList.pop_front();

    commaHeaderNode->firstKid = commaVariableTempNode;

    tauHeaderNode->firstKid = tauExpressionTempNode;

    // Iterate through the lists to complete the standardization
    while (!variableNodesList.empty())
    {
        commaVariableTempNode->nextSibling = variableNodesList.front();
        variableNodesList.pop_front();
        tauExpressionTempNode->nextSibling = expressionNodesList.front();
        expressionNodesList.pop_front();
        commaVariableTempNode = commaVariableTempNode->nextSibling;
        tauExpressionTempNode = tauExpressionTempNode->nextSibling;
    }

    commaVariableTempNode->nextSibling = NULL;
    tauExpressionTempNode->nextSibling = NULL;
}


void convertLetExpression(Node *letNode)
{
    // Set the label of the node to indicate it's a standard gamma expression
    letNode->label = GAMMA_STD_LABEL;

    // Set the label of the first child (lambda) node to indicate it's in standard lambda form
    letNode->firstKid->label = LAMBDA_STD_LABEL;

    // Extract nodes from the let expression
    Node *pNode = letNode->firstKid->nextSibling;
    Node *eNode = letNode->firstKid->firstKid->nextSibling;

    // Swap the positions of p and e nodes to standardize the expression
    letNode->firstKid->nextSibling = eNode;
    letNode->firstKid->firstKid->nextSibling = pNode;
}

void recursivelyStandardizeTree(Node *node)
{
    // Recursive function to traverse and standardize the abstract syntax tree (AST)

    // Base case: If the node has no children, return
    if (node->firstKid != NULL)
    {
        recursivelyStandardizeTree(node->firstKid);
    }
    // Traverse the next sibling of the current node, if it exists
    if (node->nextSibling != NULL)
    {
        recursivelyStandardizeTree(node->nextSibling);
    }

    // Check the label of the current node and perform corresponding standardization
    if (node->label == "->")
    {
        // Do not standardize conditionals
    }
    else if (node->label == "not" || node->label == "neg")
    {
        // Do not standardize unary operators
    }
    else if (node->label == "aug" || node->label == "or" || node->label == "&" || node->label == "gr" ||
             node->label == "ge" || node->label == "ls" || node->label == "le" || node->label == "eq" ||
             node->label == "ne" || node->label == "+" || node->label == "-" || node->label == "*" ||
             node->label == "/" || node->label == "**")
    {
        // Do not standardize binary operators
    }
    else if (node->label == "tau")
    {
        // Do not standardize tau
    }
    else if (node->label == "lambda")
    {
        // Convert lambda expressions to standardized form
        if (node->firstKid->label == ",")
        {
            // Do not standardize lambda with a tuple of variables
        }
        else
        {
            convertLambdaExpression(node);
        }
    }
    else if (node->label == FCN_FORM_LABEL)
    {
        // Convert function_form to standardized form
        convertFunctionForm(node);
    }
    else if (node->label == "@")
    {
        // Convert infix operator to standardized form
        convertInfixOperator(node);
    }
    else if (node->label == "and")
    {
        // Convert and expressions to standardized form
        convertAndExpression(node);
    }
    else if (node->label == "within")
    {
        // Convert within expressions to standardized form
        convertWithinExpression(node);
    }
    else if (node->label == "rec")
    {
        // Convert rec expressions to standardized form
        convertRecExpression(node);
    }
    else if (node->label == "let")
    {
        // Convert let expressions to standardized form
        convertLetExpression(node);
    }
    else if (node->label == "where")
    {
        // Convert where expressions to standardized form
        convertWhereExpression(node);
    }
}

void convertASTToStandardizedTree()
{
    // Convert the abstract syntax tree (AST) to a standardized tree

    // Check if the tree stack is empty
    if (trees.empty())
    {
        // If there are no trees, return
        return;
    }
    else
    {
        // If there are trees, retrieve the root of the AST from the stack
        Node *treeRootOfAST = trees.top();

        // Recursively standardize the tree starting from the root
        recursivelyStandardizeTree(treeRootOfAST);
    }
}
