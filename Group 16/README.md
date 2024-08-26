# Interpreter
Compile using following command

> g++ interpreter.cpp lexicon.cpp parser.cpp asttost.cpp flattenst.cpp csemachine.cpp -o rpal.exe

then use following command to run the rpal.exe

> ./rpal.exe file_name

use following command to print the ast

> ./rpal.exe -ast file_name