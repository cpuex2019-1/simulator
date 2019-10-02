
// main.cpp
//
#include <iostream>
#include <fstream>
#include <string>
#include "loader.h"
#include "memory.h"

using namespace std;


void print_usage_while();

int main(int argc, char *argv[]){
    if (argc < 2) {
        cerr << "usage: " << argv[0] << " <program name>" << endl;
        return 1;
    }

    // load program
    loader ld(argv[1]);
    memory memory;


    string str;
    cout << "\n >> ";;
    while (getline(cin, str)){
        if(str == "s" || str == "step" || str == "" ){ // run step by step

            /* debug for loader
            ld.print_label_map();
            ld.print_program_map();
            cout << "get line by main:" << ld.get_line_num_by_label("main") << endl;
            */

            /*
            // debug for memory
            memory.write_word(0,10);
            memory.write_word(4,5);
            memory.write_word(8,2);
            memory.print_word_by_addr(0,20);
            */

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
        << "\ts | step | \\n\t: run step by step\n"
        << "\tr | run\t\t: run all\n"
        << "\texit\t\t: exit program"
        << endl;
}
