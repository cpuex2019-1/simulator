// asm.h
//
#ifndef ASM_h
#define ASM_h

enum Asm {
    LW,
    SW,
    LF,
    SF,
    ADD_OR_MOV,
    SUB,
    MUL,
    XOR,
    ADDI,
    SLLI_OR_NOP,
    ORI,
    FADD,
    FSUB,
    FMUL,
    FDIV,
    FNEG,
    FABS,
    SQRT,
    FLOOR,
    FTOI,
    ITOF,
    MOVF,
    J,
    JAL,
    JR_OR_JALR,
    SLTF,
    BEQ,
    BNE,
    BGE,
    BLE,
    BEQF,
    BLTF,
    OUTB,
    INF,
    IN
};

#endif
