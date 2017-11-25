.on	//	.on or .off 表示是否启用该花型
push reg1	->nop
pop  reg1	->nop
--------------------------------
.on
pusha		->nop
popa		->nop
------------------------
.on
push reg1 	->nop
pop  reg2	->mov reg2,reg1
------------------------
.on
and reg1,reg1 || or reg1,reg1	-> nop
------------------------
.on
sub reg1,reg1 -> mov reg1,0
------------------------
.on
push {size1}reg1		->push op2
mov  {size1}[esp],op2	->nop
-----------------------------
.on
push imm1				->push imm3
xor  dword [esp],imm2	->nop
imm3= imm1 ^ imm2
-------------------------------
.on
push imm1				->push imm3
or  dword [esp],imm2	->nop
imm3= imm1 | imm2
-------------------------------
.on
push imm1				->push imm3
sub  dword [esp],imm2	->nop
imm3= imm1 - imm2
-------------------------------
.on
push imm1				->push imm3
add  dword [esp],imm2	->nop
imm3= imm1 + imm2
-------------------------------
.on
push imm1				->push imm3
and  dword [esp],imm2	->nop
imm3= imm1 & imm2
-------------------------------
.on
push imm1				->push imm3
not  dword [esp]		->nop
imm3= ~imm1
-------------------------------
.on
push imm1				->push imm3
inc  dword [esp]		->nop
imm3= imm1 + 1
-------------------------------
.on
push imm1				->push imm3
dec  dword [esp]		->nop
imm3= imm1 - 1
-------------------------------
.on
push {size1}op1			->push reg1
mov {size1}[esp],reg1	->nop
----------------------------
.on
sub esp,4					->push reg1
{*}{stack}::any {*}{!r}reg1
mov dword ptr [esp],reg1	->nop
------------------------------
.on
mov {size1}reg1,imm1			
{*}{stack}::any {*}{!r}reg1
{stack}::{mnem1}any op1,reg1	->{mnem1}any op1,imm1
mnem1!=xchg|cmpxchg
--------------------------------
.on
.max 20
push {size1}reg1				->nop
{*}{stack}::any {*}{!r}reg1
mov reg1,op1					->nop
{*}{stack}::any {*}{!r}reg1
pop {size1}reg1					->nop
--------------------------------
.on
push	{size1}reg1							->{mnem1}any {size1}[esp],imm1
mov		reg1,imm1							->nop
{stack}::{mnem1}any {size1}[esp+4],reg1		->nop
pop 	reg1								->nop
-------------------------------
add reg1,0 || sub reg1,0	-> nop
---------------------------
