#ifndef xstate_h__
#define xstate_h__
//申请一个状态实例
void*	xst_allocstate();
//设置某个范围内的状态
int		xst_set(void* lpst,int oft,int size,int state);
//取消某个范围内的状态
int		xst_unset(void* lpst,int oft,int size);
//测试状态
int		xst_hit(void* lpst,int* oft,int size,int* state);
#endif // xstate_h__
