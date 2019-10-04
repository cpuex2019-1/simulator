
// main.cpp
//

#include "controller.h"
#include "loader.h"
#include "memory.h"
#include "register.h"
#include <bitset>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <time.h>
using namespace std;

void print_usage();

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cerr << "usage: " << argv[0] << " <program name>" << endl;
        return 1;
    }
    print_usage();

    // load program
    loader ld(argv[1]);
    memory memo;
    reg regs[32];
    bool verbose = true;
    controller controller(&ld, &memo, regs, &verbose);

    string str;

    bool end_flag = false;
    cout << "\n >> ";
    while (getline(cin, str)) {
        if (str == "s" || str == "step" || str == "") { // run step by step

            if (verbose) {
                ld.print_label_map();
                ld.print_raw_program();
            }

            if (!controller.exec_step()) {
                end_flag = true;
            };

        } else if (str == "a" || str == "all") { // run all
            clock_t start = clock();
            int count = 0;
            while (controller.exec_step()) {
                count++;
            };
            clock_t end = clock();
            const double time =
                static_cast<double>(end - start) / CLOCKS_PER_SEC * 1000.0;
            cout << "time " << time << "[ms]\n";
            cout << count << "instructions" << endl;

            end_flag = true;

        } else if (str == "r" || str == "reg") { // print register
            cout << "which register? ( input 0~31 or a (all) ) :";
            getline(cin, str);

            std::ios::fmtflags flagsSaved = std::cout.flags();

            if (str == "all" || str == "a") {
                for (int i = 0; i < 32; i++) {
                    int reg_data = regs[i].data;
                    cout << " $" << i << "\tint:" << std::setw(9) << reg_data;
                    cout.fill('0');
                    cout << "\thex(16):0x" << std::setw(8) << std::hex
                         << reg_data << "\tbinary:" << std::bitset<32>(reg_data)
                         << endl;
                    cout.fill(' ');
                    std::cout.flags(flagsSaved);
                }
            } else {
                try {
                    int reg_num = stoi(str);
                    int reg_data = regs[reg_num].data;
                    cout << "\nregister: $" << reg_num
                         << "\tint:" << std::setw(9) << reg_data;
                    cout.fill('0');
                    cout << "\thex(16):0x" << std::setw(8) << std::hex
                         << reg_data << "\tbinary:" << std::bitset<32>(reg_data)
                         << endl;
                    cout.fill(' ');
                    std::cout.flags(flagsSaved);
                } catch (const std::invalid_argument &e) {
                    cout << "[" << str << "]: "
                         << "invalid argument" << endl;
                }
            }

        } else if (str == "m" || str == "memo") { // print memory
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
            } catch (const std::invalid_argument &e) {
                cout << "[" << str << "]: "
                     << "invalid argument" << endl;
            }

        } else if (str == "verb") { //
            verbose = true;
        } else if (str == "normal") { //
            verbose = false;
        } else if (str == "exit") { // exit
            return 0;
        } else {
            print_usage();
        }
        if (end_flag) {
            cout << "program end!" << endl;
        }
        cout << "\n >> ";
        ;
    }
    return 0;
}

void print_usage() {
    cerr << " How to use mipsim:\n"
         << "\ts | step | \\n\t: run step by step\n"
         << "\ta | all\t\t: run all\n"
         << "\tr | reg\t\t: print register\n"
         << "\tnormal\t\t: normal mode\n"
         << "\tverb\t\t: verbose mode\n"
         << "\texit\t\t: exit program" << endl;
}
