input_output:
    inb     $1
    outb    $1
    in      $1
    out     $1
    inf     $f1
    outf    $f1
arith_float:
    addi    $4, $0, 16256   # 1.0の上位16bit
    slli    $4, $4, 16
    addi    $5, $0, 16448   # 3.0の上位16bit
    slli    $5, $5, 16

    fadd     $f6, $f5, $f4
    fsub     $f6, $f5, $f4
    fmul     $f6, $f5, $f5
    fdiv     $f6, $f4, $f5
    sqrt     $f6, $f5

arith_logic1:
    addi    $1, $0, 2
    addi    $2, $0, 3
    addi    $8, $0, ha(arith_logic2)
    addi    $9, $0, lo(arith_logic2)
    bc  arith_logic3
arith_logic2:
    add     $3, $0, $1
    sub     $3, $2, $1
    mul     $3, $2, $1
    div     $3, $2, $1
    mod     $3, $2, $1
    bne $2,  $1,  arith_logic5
arith_logic3:
    slt     $3, $2, $1
    and     $3, $2, $1
    andi    $3, $2, 10
    or     $3, $2, $1
    ori    $3, $2,10
    nor     $3, $2, $1
    beq $0,  $0,  arith_logic2
arith_logic4:
    xor     $3, $2, $1
    xori    $3, $2, 2
    addi    $2, $0, 2
    srai    $3, $2, 2
    jal data_move2
data_move1:
lw  $3, 0($29)
lb  $3, -4($29)
        jr $30
arith_logic5:
    srli    $3, $2, 2
    slli    $3, $2, 2
    sra    $3, $2, $1
    srl    $3, $2, $1
    sll    $3, $2, $1
    j arith_logic4
data_move2:

    sw  $2, 0($29)
    sb  $2, -4($29)
    mov $3, $0
    jalr $30,$31
EXIT:
