input_output:
    in      $1
    in      $2
sub:
    addi    $1, $1, -1
arith_logic1:
    bge     $1, $2,  sub
    addi    $1,  $1, -5
add:
    addi    $1,  $1, 1
arith_logic2:
    ble     $1,  $2,  add
    out     $1
EXIT:
