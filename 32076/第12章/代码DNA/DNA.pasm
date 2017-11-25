.comment mov ss:[var1],[var2]	//说明
.pattern mov ss:[%s],%s			//我们可以通过脚本来格式化数据
.uuid	1	//功能编号
{*}{-}::any	//允许任何指令，但是以零断言方式匹配
mov eax,dword ptr [ebp]	//直接匹配
{*}{-}::any		//允许任何指令，但是以零断言方式匹配
mov dl,byte ptr [ebp+4]	//直接匹配
{*}{-}::any	//允许任何指令，但是以零断言方式匹配
add ebp,6	//直接匹配
{*}{-}::any	//允许任何指令，但是以零断言方式匹配
mov byte ptr :[eax],dl //直接匹配,ring3下我们取消段寄存器限定
--------------------------
.comment push vesp
.uuid 	21
.pattern push vesp
{*}{-}::any
mov eax,ebp
{*}{-}::any
sub ebp,4
{*}{-}::any
mov dword ptr :[ebp],eax
---------------------------------
.comment setop
.uuid 	44
.pattern setop
{*}{-}::any
mov esi,dword ptr :[ebp]
{*}{-}::any
add ebp,4
---------------------------------
.comment not_not_and_word
.uuid 	20
.pattern not_not_and_word
{*}{-}::any
mov ax,word ptr [ebp]
{*}{-}::any
mov dx,word ptr [ebp+2]
{*}{-}::any
not al
{*}{-}::any
not dl
{*}{-}::any
sub ebp,2
{*}{-}::any
and al,dl
{*}{-}::any
mov word ptr [ebp+4],ax
{*}{-}::any
pop dword ptr [ebp]
-------------------------------
.comment shl_word
.uuid 	30
.pattern shl_word
{*}{-}::any
mov ax,word ptr [ebp]
{*}{-}::any
mov cl,byte ptr [ebp+2]
{*}{-}::any
sub ebp,2
{*}{-}::any
shl ax,cl
{*}{-}::any
mov word ptr [ebp+4],ax
{*}{-}::any
pop dword ptr [ebp]
------------------------------------
.comment shl_byte
.uuid 	40
.pattern shl_byte
{*}{-}::any
mov al,byte ptr [ebp]
{*}{-}::any
mov cl,byte ptr [ebp+2]
{*}{-}::any
sub ebp,2
{*}{-}::any
shl al,cl
{*}{-}::any
mov word ptr [ebp+4],ax
{*}{-}::any
pop dword ptr [ebp]


