arith_logic:
    addi    $1, $0, 2
    addi    $2, $0, 3
    j   bne
    add     $3, $2, $1
    sub     $3, $2, $1
    mul     $3, $2, $1
    div     $3, $2, $1
    mod     $3, $2, $1
    slt     $3, $2, $1
    and     $3, $2, $1
    andi    $3, $2, 10
    or     $3, $2, $1
    ori    $3, $2, 10
    nor     $3, $2, $1
    xor     $3, $2, $1
    xori    $3, $2, 10
    addi    $2, $0, -3
    sra    $3, $2, 2
    srl    $3, $2, 2
    sll    $3, $2, 2
data_move:
    sw  $2, 0($29)
    sb  $2, -4($29)
    lw  $3, 0($29)
    lb  $3, -4($29)
    mov $3, $0
bne:
    bne $2,  $1,  beq
    bc  beq
        addi    $1, $0, 2
beq:
    beq $2,  $1,  bne
    bc  beq
