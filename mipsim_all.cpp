
// main.cpp
//

#include "controller_all.h"
#include "global.h"
#include "memory.h"
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

    controller controller(argv[1]);

    controller.log_level = WARN;

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
