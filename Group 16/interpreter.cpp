#include <iostream>
#include <fstream>
#include <cstring>
#include "lexicon.hpp"
#include "parser.hpp"
#include "asttost.hpp"
#include "flattenst.hpp"
#include "csemachine.hpp"


using namespace std;

int main(int argc, char* argv[]){
    
    // Check if the correct number of command line arguments are provided
    if (argc == 3){
        string flag = argv[1];
        string filename = argv[2];

        // Open the input file
        ifstream file(filename);

        // Check if the file is successfully opened
        if (!file) {
            cout << "Failed to open the file." << endl;
            return 1;
        };
        
        scan(file); //Prepare the first token by placing it within 'NT'
        E(file);    //Call the first non-terminal procedure to start parsing

        // Check if the end of the file has been reached
        if (checkIfEOF(file)) {
            if (flag == "-ast"){
                printTree();
            }
    
        } else {
            cout <<"\n\nERROR! EOF not reached but went through the complete grammar! Will exit now!!\n\n";
            exit(0);
        };

        // close the input file
        file.close();

        return 0;
    }else {
        string filename = argv[1];

        ifstream file(filename);

        if (!file) {
            cout << "Failed to open the file." << endl;
            return 1;
        };
        
        // Prepare the first token by placing it within 'NT'
        scan(file); 
        // Call the first non-terminal procedure to start parsing
        E(file);    

        if (checkIfEOF(file)) {
            convertASTToStandardizedTree();
            flattenStandardizedTree();
            runCSEMachine();
            cout << "\n";
        } else {
            cout <<"\n\nERROR! EOF not reached but went through the complete grammar! Will exit now!!\n\n";
            exit(0);
        };
        
        file.close();

        return 0;
    }
    
};


