
// main.cpp
//
#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <iomanip>
#include "loader.h"
#include "memory.h"
#include "register.h"
#include "controller.h"

using namespace std;


void print_usage_while();

int main(int argc, char *argv[]){
    if (argc < 2) {
        cerr << "usage: " << argv[0] << " <program name>" << endl;
        return 1;
    }

    // load program
    loader ld(argv[1]);
    memory memo;
    reg regs[32];
    controller controller(&ld, &memo, regs);

    string str;

    bool end_flag=false;
    cout << "\n >> ";
    while (getline(cin, str)){
        if(str == "s" || str == "step" || str == "" ){ // run step by step

            /* debug for loader*/
            ld.print_label_map();
            ld.print_program_map();

            if(!controller.exec_step()){
                end_flag = true;
            };

        }else if (str == "a" || str == "all"){ // run all

            while(controller.exec_step());
            end_flag = true;

        }else if (str == "r" || str == "reg"){ // print register
            cout << "which register? (input 0~31) :";
            getline(cin, str);

            try {
                int reg_num = stoi(str);
                int reg_data = regs[reg_num].data;
                std::ios::fmtflags flagsSaved = std::cout.flags();
                cout.fill('0');
                cout << "\nregister: $" << reg_num
                << "\n  int:\t\t" << reg_data
                << "\n  hex(16):\t0x" << std::setw(8) << std::hex << reg_data
                << "\n  binary:\t" << std::bitset<32>(reg_data) << endl;
                std::cout.flags(flagsSaved);
            }catch (const std::invalid_argument& e) {
                cout << "[" << str << "]: " << "invalid argument" << endl;
            }

        }else if (str == "m" || str == "memo"){ // print memory
            cout << "input start address:";
            getline(cin, str);
            string start_addr_str = str;
            cout << "input end address:";
            getline(cin, str);
            string end_addr_str = str;

            try {
                int start_addr = stoi(start_addr_str);
                int end_addr = stoi(end_addr_str);
                memo.print_word_by_addr(start_addr, end_addr);
            }catch (const std::invalid_argument& e) {
                cout << "[" << str << "]: " << "invalid argument" << endl;
            }

        }else if (str == "exit"){ // exit
            return 0;
        }else{
            print_usage_while();
        }
        if(end_flag){
            cout << "program end!" << endl;
        }
        cout << "\n >> ";;
    }
    return 0;
}


void print_usage_while(){
    cerr << " How to use mipsim:\n"
        << "\ts | step | \\n\t: run step by step\n"
        << "\ta | all\t\t: run all\n"
        << "\tr | reg\t\t: print register\n"
        << "\tm | memo\t: print memory\n"
        << "\texit\t\t: exit program"
        << endl;
}
