.on
mov reg1,reg2	->nop
{*}{-}{stack}::any {*}{!r}reg1,{*}{!r}reg2
add reg1,imm1			->nop
{*}{-}{stack}::any {*}{!r}reg1,{*}{!r}reg2
{mnem1}any op1,{size1}[reg1]	->{mnem1}any op1,{size1}[reg2+imm1]