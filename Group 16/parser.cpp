#include "parser.hpp"
#include "lexicon.hpp"
#include <fstream>
#include <iostream>
#include <stack>

using namespace std;

const string FCN_FORM_LABEL = "function_form";

const string GAMMA_STD_LABEL = "Gamma";
const string LAMBDA_STD_LABEL = "Lambda";

stack<Node *> trees;

using namespace std;

// Function to build a tree node
void buildTree(string nodeLabel, int noOfTreesToPopAndMakeChildren)
{
    // Create a new Node object
    Node *treeNode = new Node;
    treeNode->label = nodeLabel;
    treeNode->nextSibling = NULL;

    // Initialize a pointer to the first child of the new node
    Node *treeNodePtr = NULL; 

    // Check if the stack is empty or if there are not enough trees to pop
    if (noOfTreesToPopAndMakeChildren > 0 && trees.empty())
    {
        cout << "\n\nERROR! Something went wrong in AST generation! Program will die now!\n\n";
        exit(0);
    }

    // Pop trees from the stack and add them as children of the new node
    while (noOfTreesToPopAndMakeChildren > 0 && !trees.empty())
    {
        if (treeNodePtr != NULL)
        {
            // Add the popped tree as the next sibling of the current child
            trees.top()->nextSibling = treeNodePtr;
            treeNodePtr = trees.top();
        }
        else
        {
            // Add the popped tree as the first child of the new node
            treeNodePtr = trees.top();
        }
        trees.pop();
        noOfTreesToPopAndMakeChildren--;
    }

    // Set the first child of the new node
    treeNode->firstKid = treeNodePtr;
    
    // Add the new node to the stack
    trees.push(treeNode);
    return;
}

void E(ifstream &file);

void D(ifstream &file);

void readToken(ifstream &file, string token);

/*
 * The procedure for the Vl non-terminal.
 */
int Vl(ifstream &file, int identifiersReadBefore, bool isRecursiveCall)
{
    // Create a new Node object for the identifier
    buildTree("<ID:" + NT + ">", 0);

    // Read the next token from the file
    readToken(file, IDENTIFIER);

    // Check if the next token is a comma
    if (NT.compare(",") == 0)
    {
        // Read the comma token
        readToken(file, ",");

        // Increment the number of identifiers read before
        identifiersReadBefore += 1;

        // Recursively call Vl to parse the remaining identifiers
        identifiersReadBefore = Vl(file, identifiersReadBefore, true);
    }

    // Calculate the total number of identifiers in the VList
    int identifiersInVList = identifiersReadBefore + 1;

    // If this is not a recursive call and there are more than one identifiers in the VList
    if (!isRecursiveCall && identifiersInVList > 1)
    {
        // Create a new Node object for the comma separator
        buildTree(",", identifiersInVList);
    }
    return identifiersReadBefore;
}

/*
 * The procedure for the Vb non-terminal.
 */

// The procedure for the Vb non-terminal.
// This function parses a single variable or a list of variables.
// If the next token is an opening parenthesis, it parses the argument list.
// If the next token is an identifier, it parses it as a variable.
void Vb(ifstream &file)
{
    // Check if the next token is an opening parenthesis
    if (NT.compare("(") == 0)
    {
        // Read the opening parenthesis token
        readToken(file, "(");

        // Check if the next token is an identifier
        bool isVl = false;
        if (nextTokenType.compare(IDENTIFIER) == 0)
        {
            Vl(file, 0, false);
            isVl = true;
        }

        // Read the closing parenthesis token
        readToken(file, ")");

        // If the VList was not parsed, create a new Node object for the empty parentheses
        if (!isVl)
        {
            buildTree("()", 0);
        }
    }

    // If the next token is an identifier, parse it as a variable
    else if (nextTokenType.compare(IDENTIFIER) == 0)
    {
        buildTree("<ID:" + NT + ">", 0);
        readToken(file, IDENTIFIER);
    }
}

/*
 * The procedure for the Db non-terminal.
 */
void Db(ifstream &file)
{
    // Check if the next token is an opening parenthesis
    if (NT.compare("(") == 0)
    {
        readToken(file, "(");
        D(file);
        readToken(file, ")");
    }

    // If the next token is an identifier, parse it as a variable
    else if (nextTokenType.compare(IDENTIFIER) == 0)
    {
        buildTree("<ID:" + NT + ">", 0);
        readToken(file, IDENTIFIER);

        // If the next token is a comma, parse the rest of the VList
        if (NT.compare(",") == 0)
        {
            readToken(file, ",");
            Vl(file, 1, false);
            readToken(file, "=");
            E(file);
            
            // Add the variable-value pair to the tree
            buildTree("=", 2);
        }

        // If the next token is an equals sign, parse the value
        else if (NT.compare("=") == 0)
        {
            // read token value
            readToken(file, "=");
            E(file);
            
            // Add the variable-value pair to the tree
            buildTree("=", 2);
        }

        // If the next token is not an equals sign, parse the function body
        else
        {
            int n = 1;
            while (nextTokenType.compare(IDENTIFIER) == 0 || NT.compare("(") == 0)
            {
                Vb(file);
                n++;
            }

            // read token value
            readToken(file, "=");
            E(file);
            
            // Add the function body to the tree
            buildTree(FCN_FORM_LABEL, n + 1); // n + 'E'
        }
    }
}

/*
 * The procedure for the Dr non-terminal.
 */
void Dr(ifstream &file)
{
    // Check if the next token is "rec"
    int isRec = false;
    if (NT.compare("rec") == 0)
    {
       // Read the "rec" token
        readToken(file, "rec");
        isRec = true;
    }

    // Parse the function body
    Db(file);

    if (isRec)
    {
        // Add the "rec" keyword to the tree
        buildTree("rec", 1);
    }
}

/*
 *The procedure for the Da non-terminal.
 */
void Da(ifstream &file)
{
    // Parse the first Dr
    Dr(file);

    // Count the number of "and" keywords
    int n = 1;
    while (NT.compare("and") == 0)
    {
        // Read the "and" keyword
        readToken(file, "and");

        // Parse the next Dr
        Dr(file);

        // Increment the count
        n++;
    }

    // If there is more than one Dr, add the "and" nodes to the tree
    if (n > 1)
    {
        buildTree("and", n);
    }
}

/**
 ** The procedure for the D non-terminal.
 **/
void D(ifstream &file)
{
    // Parse the first Da
    Da(file);

    // If the next token is "within", parse the next D
    if (NT.compare("within") == 0)
    {
        // Read the "within" keyword
        readToken(file, "within");

        // Parse the next D
        D(file);

        // Add the "within" node to the tree
        buildTree("within", 2);
    }
}

/**
 ** The procedure for the Rn non-terminal.
 **/
void Rn(ifstream &file)
{
    // If the next token is an identifier, parse it
    if (nextTokenType.compare(IDENTIFIER) == 0)
    {
        buildTree("<ID:" + NT + ">", 0);
        readToken(file, IDENTIFIER);
    }

    // If the next token is a string, parse it
    else if (nextTokenType.compare(STRING) == 0)
    {
        buildTree("<STR:" + NT + ">", 0);
        readToken(file, STRING);   // read token
    }

    // If the next token is an integer, parse it
    else if (nextTokenType.compare(INTEGER) == 0)
    {
        buildTree("<INT:" + NT + ">", 0);
        readToken(file, INTEGER);
    }

    // If the next token is a keyword, parse it
    else if (NT.compare("true") == 0 || NT.compare("false") == 0 ||
             NT.compare("nil") == 0 || NT.compare("dummy") == 0)
    {
        buildTree("<" + NT + ">", 0);
        readToken(file, NT);
    }

    // If the next token is an opening parenthesis, parse the expression
    else if (NT.compare("(") == 0)
    {
        readToken(file, "(");
        E(file);
        readToken(file, ")");
    }
}

/**
 ** The procedure for the R non-terminal.
 **/
void R(ifstream &file)
{
    // Parse the first Rn
    Rn(file);

    // While the next token is a valid Rn, parse it and add the "gamma" node to the tree
    while (nextTokenType.compare(IDENTIFIER) == 0 || nextTokenType.compare(INTEGER) == 0 ||
           nextTokenType.compare(STRING) == 0 || NT.compare("true") == 0 || NT.compare("false") == 0 ||
           NT.compare("nil") == 0 || NT.compare("dummy") == 0 || NT.compare("(") == 0)
    {
        // parse next Rn
        Rn(file);

        // build tree with two children
        buildTree("gamma", 2);
    }
}

/**
 ** The procedure for the Ap non-terminal.
 **/
void Ap(ifstream &file)
{
    // parse the first R
    R(file);

    // While the next token is "@", parse the infix operator and the next R
    while (NT.compare("@") == 0)
    {
        // Read the "@" token
        readToken(file, "@");

        // Build a tree node for the infix operator
        buildTree("<ID:" + NT + ">", 0); 

        // Read the identifier for the infix operator
        readToken(file, IDENTIFIER);

        // Parse the next R
        R(file);

        // Add the "@" node to the tree with the infix operator node, the previous R, and the current R as children
        buildTree("@", 3);
    }
}

/*
 * The procedure for the Af non-terminal.
 */
void Af(ifstream &file)
{
    // Parse the sequence of Ap non-terminals
    Ap(file);

    // If the next token is "**", parse the next Af and build a tree node for the "**" operator
    if (NT.compare("**") == 0)
    {
        // Read the "**" token
        readToken(file, "**");

        // Parse the next Af
        Af(file);

        // Build a tree node for the "**" operator
        buildTree("**", 2);
    }
}

/*
 * The procedure for the At non-terminal.
 */
void At(ifstream &file)
{
    // Parse the sequence of Af non-terminals
    Af(file);

    // While the next token is "*" or "/", parse the next Af and build a tree node for the multiplicative operator
    while (NT.compare("*") == 0 || NT.compare("/") == 0)
    {
        // Save the current token as a string
        string token = NT; 

        // Read the next token
        readToken(file, NT);

        // Parse the next Af
        Af(file);

        // Build a tree node for the multiplicative operator
        buildTree(token, 2);
    }
}

/*
 * The procedure for the A non-terminal.
 */
void A(ifstream &file)
{
    // Parse the sequence of At non-terminals
    if (NT.compare("+") == 0)
    {
        // Read the "+" token
        readToken(file, "+");

        // Parse the next At
        At(file);
    }
    else if (NT.compare("-") == 0)
    {
        // read the "-" token
        readToken(file, "-");

        // parse the next At
        At(file);

        // Build a tree node for the negation operator
        buildTree("neg", 1);
    }
    else
    {
        // If the next token is not "+", "-", or ")", parse the next At
        At(file);
    }

    // While the next token is "+", "-", parse the next At and build a tree node for the additive operator
    while (NT.compare("+") == 0 || NT.compare("-") == 0)
    {
        // Save the current token as a string
        string token = NT;

        // Read the next token
        readToken(file, NT);

        // parse the next At
        At(file);

        // Build a tree node for the additive operator
        buildTree(token, 2);
    }
}

/*
 * The procedure for the Bp non-terminal.
 */
void Bp(ifstream &file)
{
    // Parse the sequence of A non-terminals
    A(file);

    // If the next token is "gr", ">", "ge", ">=", "ls", "<", "le", "<=", "eq", or "ne", parse the next A and build a tree node for the relational operator
    if (NT.compare("gr") == 0 || NT.compare(">") == 0)
    {
        // Read the "gr" or ">" token
        readToken(file, NT);

        // Parse the next A
        A(file);

        // Build a tree node for the ">" operator
        buildTree("gr", 2);
    }
    else if (NT.compare("ge") == 0 || NT.compare(">=") == 0)
    {
        // Read the "ge" or ">=" token
        readToken(file, NT);
        A(file);
        // Build a tree node for the ">=" operator
        buildTree("ge", 2);
    }
    else if (NT.compare("ls") == 0 || NT.compare("<") == 0)
    {
        // Read the "ls" or "<" token
        readToken(file, NT);
        A(file);
        // Build a tree node for the "<" operator
        buildTree("ls", 2);
    }
    else if (NT.compare("le") == 0 || NT.compare("<=") == 0)
    {
        // Read the "le" or "<=" token
        readToken(file, NT);
        A(file);
        // Build a tree node for the "<=" operator
        buildTree("le", 2);
    }
    else if (NT.compare("eq") == 0)
    {
        // Read the "eq" token
        readToken(file, "eq");
        A(file);
        // build a tree node for the "eq" operator
        buildTree("eq", 2);
    }
    else if (NT.compare("ne") == 0)
    {
        // Read the "ne" token
        readToken(file, "ne");

        // Parse the next A
        A(file);

        // Build a tree node for the "!=" operator
        buildTree("ne", 2);
    }
}

/*
 * The procedure for the Bs non-terminal.
 */
void Bs(ifstream &file)
{
    // Initialize a boolean flag to indicate whether the next token is "not"
    bool isNeg = false;

    // If the next token is "not", parse the "not" token and set the flag to true
    if (NT.compare("not") == 0)
    {
        readToken(file, "not");
        // Set the flag to true
        isNeg = true;
    }
    Bp(file);

    // If the flag is true, build a tree node for the "not" operator
    if (isNeg)
    {
        // Build a tree node for the "not" operator
        buildTree("not", 1);
    }
}

/*
 * The procedure for the Bt non-terminal.
 */
void Bt(ifstream &file)
{
    Bs(file);
    int n = 1;
    while (NT.compare("&") == 0)
    {
        readToken(file, "&");
        Bs(file);
        n++;
    }
    if (n > 1)
    {
        buildTree("&", n);
    }
}

/*
 * The procedure for the B non-terminal.
 */
void B(ifstream &file)
{
    Bt(file);
    int n = 1;
    while (NT.compare("or") == 0)
    {
        readToken(file, "or");
        Bt(file);
        n++;
    }
    if (n > 1)
    {
        buildTree("or", n);
    }
}

/*
 * The procedure for the Tc non-terminal.
 */
void Tc(ifstream &file)
{
    B(file);
    if (NT.compare("->") ==
        0)
    {
        // read token "->"
        readToken(file, "->");

        Tc(file);

        // read "|"
        readToken(file, "|");
        Tc(file);

        // build tree for "->" node
        buildTree("->", 3);
    }
}

/*
 * The procedure for the Ta non-terminal.
 */
void Ta(ifstream &file)
{
    Tc(file);
    while (NT.compare("aug") == 0)
    { // left recursion
        readToken(file, "aug");
        Tc(file);
        buildTree("aug", 2);
    }
}

/*
 * The procedure for the T non-terminal.
 */
void T(ifstream &file)
{
    Ta(file);
    int n = 1;
    while (NT.compare(",") == 0)
    { // combo of left recursion AND common prefix
        n++;
        readToken(file, ",");
        Ta(file);
    }
    if (n != 1)
    {
        buildTree("tau", n);
    }
}

/*
 * The procedure for the Ew non-terminal.
 */
void Ew(ifstream &file)
{
    T(file);
    if (NT.compare("where") == 0)
    { // common prefix
        readToken(file, "where");
        Dr(file);
        buildTree("where", 2);
    }
}

void E(ifstream &file)
{
    int N = 0;
    // Check for the "let" and "fn" non-terminals
    if (NT.compare("let") == 0)
    {
        // Parse the "let" non-terminal
        readToken(file, "let");
        D(file);
        readToken(file, "in");
        E(file);
        buildTree("let", 2);
    }
    else if (NT.compare("fn") == 0)
    {
        // Parse the "fn" non-terminal
        readToken(file, "fn");
        do
        {
            // Parse the sequence of "Vb" non-terminals
            Vb(file);
            N++;
        } while (nextTokenType.compare(IDENTIFIER) == 0 || NT.compare("(") == 0);
        readToken(file, ".");
        E(file);
        buildTree("lambda", N + 1); // number of 'Vb's plus the 'E'
    }
    else
    {
        // Parse the sequence of "Ew" non-terminals
        Ew(file);
    }
}

void readToken(ifstream &file, string token)
{
    // Check if the expected token matches the current token or the next token type
    if (token.compare(NT) != 0 && token.compare(nextTokenType) != 0)
    {
        throw exception();
    }

    scan(file);
}