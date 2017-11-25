.comment mov ss:[var1],[var2]
.pattern mov ss:[%s],%s
.uuid	1
{*}{-}::any
mov eax,dword ptr [ebp]
{*}{-}::any
mov dl,byte ptr [ebp+4]
{*}{-}::any
add ebp,6
{*}{-}::any
mov byte ptr ss:[eax],dl
{*}{-}::any
--------------------------
.comment block2
{*}{-}::any
mov reg1,word ptr [ebp]
{*}{-}::any
mov reg2,word ptr [ebp+2]
{*}{-}::any
mov word ptr [ebp+4],reg1