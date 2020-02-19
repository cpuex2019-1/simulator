input_output:
    in      $1
    outb    $1
    inf     $f1
    itof   $f2, $1
    ftoi   $2, $f1
memo:
    addi    $29, $0, 10000
    addi    $3, $0, 16256   # 1.0の上位16bit
    slli    $3, $3, 16
    addi    $4, $0, 16448   # 3.0の上位16bit
    slli    $4, $4, 16
    sw      $3, 0($29)
    sw      $4, -4($29)
    lf      $f3, 0($29)
    lf      $f4, -4($29)
    lw      $5, 0($29)
    sf      $f3, -8($29)
    lf      $f5, -8($29)

arith:
    add     $3, $0, $1
    sub     $3, $2, $1
    mul     $3, $2, $1
    xor     $3, $2, $1
    addi    $3, $0, 10000
    slli    $3, $1, 16
    ori    $3, $2,10

arith_float:
    fadd     $f6, $f5, $f4
    fsub     $f6, $f5, $f4
    fmul     $f6, $f5, $f4
    fdiv     $f6, $f5, $f4
    fneg     $f6, $f4
    fabs     $f7, $f6
    sqrt     $f6, $f4
    floor    $f7, $f6
    ftoi     $3, $f6
    itof     $f6, $2
    movf    $f7, $f4

jump_j:
    j jump_jal_jr
jump_jal_jr:
    jal jump_jr
jump_jal_jalr:
    jal jump_jalr
branch_bne:
    sltf $3, $f2, $f1
    bne $3, $2 ,branch_bge
jump_jalr:
    jalr $30,$31
jump_jr:
    jr $31

branch_bge:
    bge $2, $3, branch_ble
branch_beqf:
    beqf $f2, $f2, branch_bltf
branch_ble:
    ble $3, $2, branch_beqf
branch_bltf:
    bltf $f3, $f2,EXIT

EXIT:
