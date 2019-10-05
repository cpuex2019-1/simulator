
// main.cpp
//

#include "controller.h"
#include "global.h"
#include "loader.h"
#include "memory.h"
#include "print.h"
#include "register.h"
#include <bitset>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <time.h>
using namespace std;

Log log_level = DEBUG;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: %s <program name>\n", argv[0]);
        return 1;
    }
    print_usage();

    loader ld(argv[1], &log_level); // load program
    memory memo(&log_level);
    reg regs[32];

    controller controller(&ld, &memo, regs, &log_level);

    string str;

    bool end_flag = false;
    print_prompt();
    while (getline(cin, str)) {
        if (str == "s" || str == "step" || str == "") { // run step by step

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

            printf("time %lf [ms]\n", time);
            printf("%d instructions\n", count);

            end_flag = true;

        } else if (str == "r" || str == "reg") { // print register
            printf("which register? ( input 0~31 or a (all) ) : ");
            getline(cin, str);

            if (str == "all" || str == "a") {
                for (int i = 0; i < 32; i++) {
                    int reg_data = regs[i].data;
                    printf(" $%2d\tint:%9d\thex(16):%8x\tbinary:", i, reg_data,
                           reg_data);
                    print_binary(reg_data);
                    printf("\n");
                }
            } else {
                try {
                    int reg_num = stoi(str);
                    int reg_data = regs[reg_num].data;
                    printf("register: $%2d\tint:%9d\thex(16):%8x\tbinary:",
                           reg_num, reg_data, ((unsigned int)reg_data));
                    print_binary(reg_data);
                    printf("\n");
                } catch (const std::invalid_argument &e) {
                    if (log_level >= ERROR) {
                        printf("ERROR\tinvalid argument: [%s]\n", e.what());
                    }
                }
            }

        } else if (str == "m" || str == "memo") { // print memory
            printf("input start address: ");
            getline(cin, str);
            string start_addr_str = str;
            printf("input end address: ");
            getline(cin, str);
            string end_addr_str = str;

            int start_addr;
            int end_addr;
            try {
                start_addr = stoi(start_addr_str);
                end_addr = stoi(end_addr_str);
                memo.print_word_by_addr(start_addr, end_addr);
            } catch (const std::invalid_argument &e) {
                if (log_level >= ERROR) {
                    printf("ERROR\tinvalid argument: %s\n", e.what());
                    printf("ERROR\tPlease input Multiples of 4\n");
                }
            }

        } else if (str == "p" || str == "program") { // print program
            ld.print_label_map();
            ld.print_raw_program();

        } else if (str == "l" || str == "log") {
            printf("select log level (warn < info < debug < trace) : ");
            getline(cin, str);
            if (str == "warn") { //
                log_level = WARN;
            } else if (str == "info") { //
                log_level = INFO;
            } else if (str == "debug") { //
                log_level = DEBUG;
            } else if (str == "TRACE") {
                log_level = TRACE;
            } else {
                if (log_level >= ERROR) {
                    printf("ERROR\tinvalid argument: %s\n", str.c_str());
                }
            }

        } else if (str == "exit") { // exit
            return 0;

        } else {
            if (log_level >= ERROR) {
                printf("ERROR\tinvalid command: %s\n", str.c_str());
                print_usage();
            }
        }

        if (end_flag) {
            printf("\nprogram end!\n");
        }
        print_prompt();
    }
    return 0;
}
