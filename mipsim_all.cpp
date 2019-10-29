
// main.cpp
//

#include "controller_all.h"
#include "global.h"
#include "loader.h"
#include "memory.h"
#include <bitset>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <time.h>
using namespace std;

Log log_level = WARN;
int break_p = -1;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: %s <program name>\n", argv[0]);
        return 1;
    }

    printf("now loading...\n");
    loader *ld = new loader(argv[1], &log_level); // load program
    memory memo(&log_level);
    controller controller(argv[1], ld, &memo, &log_level);

    printf("start simulation\n");
    string str;
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
