add reg1,imm1	->add reg1,imm3
sub reg1,imm2	->nop
imm3 =imm1-imm2
------------------------
add reg1,0 || sub reg1,0	-> nop
------------------------
add reg1,imm1	->add reg1,imm3
add reg1,imm2	->nop
imm3 =imm1+imm2
---------------------------
xor reg1,imm1	->xor reg1,imm3
xor reg1,imm2	->nop
imm3=imm1 ^ imm2