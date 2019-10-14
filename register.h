// register.h:
//
#ifndef REGISTER_H
#define REGISTER_H

#include "global.h"

union IntAndFloat {
    int i;
    float f;
};

class reg {
  public:
    int data;
    reg();
};

class freg {
  public:
    IntAndFloat data;
    freg();
};

#endif
