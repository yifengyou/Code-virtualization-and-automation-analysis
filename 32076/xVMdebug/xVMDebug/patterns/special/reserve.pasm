.mode reserve
mov {size1}reg1,imm1
{*}{stack}::any {*}{!r}reg1
{stack}::{strict}any {size1}reg1,op1 -> nop
-------------------------------