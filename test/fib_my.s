Main:
    addi    $2, $0, 30
    jal fib.10
  j Exit
#   main program ends
fib.10:
    addi    $30, $0, 0
    slli $30, $30, 16
    ori $30, $30, 1
    slt $30, $30, $2
    bne $30, $0, eq_else.24
    jr $31
eq_else.24:
    addi    $5, $2, -1
    sw  $2, 0($3)
    mov $30, $31
    mov $2, $5
    sw  $30, 4($3)
    addi    $3, $3, 8
    jal fib.10
    addi    $3, $3, -8
    lw  $31, 4($3)
    lw  $5, 0($3)
    addi    $5, $5, -2
    sw  $2, 4($3)
    mov $30, $31
    mov $2, $5
    sw  $30, 12($3)
    addi    $3, $3, 16
    jal fib.10
    addi    $3, $3, -16
    lw  $31, 12($3)
    lw  $5, 4($3)
    add $2, $5, $2
    jr $31
Exit:
