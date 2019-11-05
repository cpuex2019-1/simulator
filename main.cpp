
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

int break_p = -1;
Log log_level = DEBUG;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: %s <program name>\n", argv[0]);
        return 1;
    }

    loader ld(argv[1]); // load program
    memory memo;
    reg regs[32];
    freg fregs[32];
    controller controller(argv[1], &ld, &memo, regs, fregs);

    string str;
    bool end_flag = false;
    print_usage();
    print_prompt();
    while (getline(cin, str)) {
        if (str == "s" || str == "step" || str == "") { // run step by step

            if (!end_flag) {
                if (log_level >= TRACE) {
                    // ld.print_label_map();
                    ld.print_raw_program();
                }
                if (controller.exec_step(break_p) == END) {
                    end_flag = true;
                };
            }

        } else if (str == "e" |
                   str ==
                       "execute") { // execute instructions for designated times

            if (!end_flag) {
                printf("How many instructions?: ");
                getline(cin, str);
                long long int times;
                long long int count = 0;
                try {
                    times = stoll(str);
                    clock_t start = clock();
                    Status status = ACTIVE;
                    while (status == ACTIVE && count < times) {
                        status = controller.exec_step(break_p);
                        count++;
                    };
                    clock_t end = clock();
                    const double time = static_cast<double>(end - start) /
                                        CLOCKS_PER_SEC * 1000.0;
                    if (status == BREAK) {
                        printf("\nbreakpoint!\n");
                        string one_raw_program =
                            ld.get_raw_program_by_line_num(controller.line_num);
                        printf("[next instruction]\t%d:\t%s\n\n",
                               controller.line_num, one_raw_program.c_str());

                    } else if (status == END) {
                        end_flag = true;
                    };
                    printf("time %lf [ms]\n", time);
                    printf("%lld instructions\n", count);
                } catch (...) {
                    if (log_level >= ERROR) {
                        printf("ERROR\tinvalid argument: %s\n", str.c_str());
                    }
                }
            }

        } else if (str == "a" || str == "all") { // run all
            if (!end_flag) {
                clock_t start = clock();
                long long int count = 0;
                Status status = ACTIVE;
                while (status == ACTIVE) {
                    status = controller.exec_step(break_p);
                    count++;
                };
                clock_t end = clock();
                const double time =
                    static_cast<double>(end - start) / CLOCKS_PER_SEC * 1000.0;

                if (status == BREAK) {
                    printf("\nbreakpoint!\n");
                    string one_raw_program =
                        ld.get_raw_program_by_line_num(controller.line_num);
                    printf("[next instruction]\t%d:\t%s\n\n",
                           controller.line_num, one_raw_program.c_str());

                } else if (status == END) {
                    end_flag = true;
                };
                printf("time %lf [ms]\n", time);
                printf("%lld instructions\n", count);
            }

        } else if (str == "r" || str == "reg") { // print register
            printf("which register? (g(general) | f(float) | a(all)) : ");
            getline(cin, str);

            if (str == "all" || str == "a") {
                printf("general registers\n");
                for (int i = 0; i < 32; i++) {
                    print_reg(i, regs);
                }
                printf("\nfloat registers\n");
                for (int i = 0; i < 32; i++) {
                    print_freg(i, fregs);
                }
            } else if (str == "float" || str == "f") {
                printf("\nfloat registers\n");
                for (int i = 0; i < 32; i++) {
                    print_freg(i, fregs);
                }
            } else if (str == "general" || str == "g") {
                printf("\nfloat registers\n");
                for (int i = 0; i < 32; i++) {
                    print_reg(i, regs);
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
            printf("now processing program addr: %d\n",
                   controller.line_num * 4);

        } else if (str == "b" || str == "break") { // set breakpoint
            // ld.print_label_map();
            ld.print_raw_program();
            printf("\nset break break_point by program address : ");
            getline(cin, str);

            try {
                break_p = stoi(str) / 4;
            } catch (const std::invalid_argument &e) {
                if (log_level >= ERROR) {
                    printf("ERROR\tinvalid argument: %s\n", e.what());
                    printf("ERROR\tPlease input integer\n");
                }
            }

        } else if (str == "l" || str == "log") {
            printf("select log level (warn < info < debug < trace) : ");
            getline(cin, str);
            if (str == "warn") { //
                log_level = WARN;
                ld.log_level = WARN;
                memo.log_level = WARN;
                controller.log_level = WARN;
            } else if (str == "info") { //
                log_level = INFO;
                ld.log_level = INFO;
                memo.log_level = INFO;
                controller.log_level = INFO;
            } else if (str == "debug") { //
                log_level = DEBUG;
                ld.log_level = DEBUG;
                memo.log_level = DEBUG;
                controller.log_level = DEBUG;
            } else if (str == "trace") {
                log_level = TRACE;
                ld.log_level = TRACE;
                memo.log_level = TRACE;
                controller.log_level = TRACE;
            } else {
                if (log_level >= ERROR) {
                    printf("ERROR\tinvalid argument: %s\n", str.c_str());
                }
            }

        } else if (str == "restart") { // restart
            controller.init();
            end_flag = false;
            printf("\n restart!\n\n");
            print_usage();

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
            controller.print_statistic_to_file();
            // controller.print_jump_times();
            // controller.print_inst_times();
        }
        print_prompt();
    }
    return 0;
}
