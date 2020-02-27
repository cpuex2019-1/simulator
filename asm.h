// asm.h
//
#ifndef ASM_h
#define ASM_h

enum Asm {
    LW,
    SW,
    LF,
    SF,
    ADDLW,
    ADDLF,
    ADD_OR_MOV,
    SUB,
    MUL,
    XOR,
    ADDI,
    SLLI_OR_NOP,
    SRLI,
    SRAI,
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
    SLT,
    SLTF,
    BEQ,
    BNE,
    BLE_OR_BGE,
    BEQF,
    BLTF,
    OUTB,
    INF,
    IN
};

#endif
