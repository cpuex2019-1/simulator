
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

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: %s <program name>\n", argv[0]);
        return 1;
    }

    printf("now loading...\n");
    loader ld(argv[1]); // load program
    memory memo;
    reg regs[32];
    freg fregs[32];

    controller controller(argv[1], &ld, &memo, regs, fregs);
    ld.log_level = WARN;
    memo.log_level = WARN;
    controller.log_level = WARN;

    string str;
    printf("start simulation\n");
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

    printf("time %lf [ms]\n", time);
    printf("%lld instructions\n", count);
    return 0;
}
