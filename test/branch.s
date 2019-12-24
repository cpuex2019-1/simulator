input_output:
    in      $1
    in      $2
    inf      $f0
    inf      $f1
    inf      $f2
subf:
    fsub    $f1, $f1, $f0
branch1f:
    beqf    $f1, $f2,  subf
    fsub    $f1, $f1, $f0
    fsub    $f1, $f1, $f0
addf:
    fadd    $f1, $f1, $f0
branch2f:
    bltf     $f1,  $f2,  addf
    outf    $f1
sub:
    addi    $1, $1, -1
branch1:
    bge     $1, $2,  sub
    addi    $1,  $1, -5
add:
    addi    $1,  $1, 1
branch2:
    ble     $1,  $2,  add
    out     $1
EXIT:
