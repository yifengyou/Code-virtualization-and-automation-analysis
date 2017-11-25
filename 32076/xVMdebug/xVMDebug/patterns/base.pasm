.on	//	.on or .off 表示是否启用该花型
push reg1	->nop
{*}{stack}::any {*}{!r}reg1
pop  reg1	->nop
------------------------
jmp imm1	-> nop
{?}::any
-------------------------
.on
pusha		->nop
popa		->nop
-------------------------
mov reg1,op2			->nop
{*}::any {*}{!r}reg1
mov reg1,op3
-----------------------------
mov {size1}[esp],op2	->nop
{*}{-}{stack}::any
mov {size1}[esp],op3
-------------------------
push {size1}op1			->push op2
{*}{-}{stack}::any
mov {size1}[esp],op2	->nop