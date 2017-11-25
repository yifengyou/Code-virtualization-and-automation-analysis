.mode reserve
mov {size1}reg1,op1
{*}{-}{stack}::any {*}{!r}reg1
{stack}::{strict}{mnem1}any {size1}reg1,op2 -> nop
mnem1!=xchg|cmpxchg
-------------------------------
.on
mov reg1,reg2					->nop
{*}{-}{stack}::any {*}{!r}reg1,{*}{!r}reg2
add reg1,imm1					->nop
{*}{-}{stack}::any {*}{!r}reg1,{*}{!r}reg2
{mnem1}any {size1}[reg1],op1	->{mnem1}any {size1}[reg2+imm1],op1
-------------------------
.on
mov reg1,reg2	->nop
{*}{-}{stack}::any {*}{!r}reg1,{*}{!r}reg2
add reg1,imm1			->nop
{*}{-}{stack}::any {*}{!r}reg1,{*}{!r}reg2
{mnem1}any op1,{size1}[reg1]	->{mnem1}any op1,{size1}[reg2+imm1]
-------------------------
mov reg1,imm1		->mov reg1,imm3
{*}{-}{stack}::any {*}{!r}reg1
add reg1,imm2		->nop
imm3=imm1+imm2
----------------------------
.on
push imm1					->push imm3
{*}{-}{stack}::any
shl	dword ptr [esp],imm2	->nop
imm3=imm1 < imm2
-------------------------
.on
push imm1					->push imm3
{*}{-}{stack}::any
add	dword ptr [esp],imm2	->nop
imm3=imm1 + imm2
-------------------------
.on
push imm1					->push imm3
{*}{-}{stack}::any
sub dword ptr [esp],imm2	->nop
imm3=imm1 - imm2
-------------------------
.on
push imm1					->push imm3
{*}{-}{stack}::any
neg dword ptr [esp]			->nop
imm3=-imm1
-------------------------
.on
push imm1					->push imm3
{*}{-}{stack}::any
not dword ptr [esp]			->nop
imm3=~imm1
-------------------------
.on
push imm1					->push imm3
{*}{-}{stack}::any
shr	dword ptr [esp],imm2	->nop
imm3=imm1 > imm2
--------------------------
add reg1,imm1	->add reg1,imm3
{*}{-}{stack}::any {*}{!r}reg1
sub reg1,imm2	->nop
imm3 =imm1-imm2
--------------------------
add reg1,imm1	->add reg1,imm3
{*}{-}{stack}::any {*}{!r}reg1
sub reg1,imm2	->nop
imm3 =imm1+imm2

