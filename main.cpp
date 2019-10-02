
// main.cpp
//
#include <iostream>
#include <fstream>
#include <string>
#include "loader.h"

using namespace std;


void print_usage_while();

int main(int argc, char *argv[]){
    if (argc < 2) {
        cerr << "usage: " << argv[0] << " <program name>" << endl;
        return 1;
    }

    // load program
    loader ld(argv[1]);


    string str;
    cout << "\n >> ";;
    while (getline(cin, str)){
        if(str == "s" || str == "step" || str == "" ){ // run step by step

            ld.print_label_map();
            ld.print_program_map();
        }else if (str == "r" || str == "run"){ // run all

        }else if (str == "exit"){ // run all
            return 0;
        }else{
            print_usage_while();
        }
        cout << "\n >> ";;
    }
    return 0;
}


void print_usage_while(){
    cerr << "usage:\n"
        << "\t\"s\"|\"step\"|\"\": run step by step\n"
        << "\t\"r\"|\"run\": run all"
        << "\t\"exit\": exit program"
        << endl;
}
