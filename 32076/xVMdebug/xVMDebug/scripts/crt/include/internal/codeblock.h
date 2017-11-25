#ifndef codeblock_h__
#define codeblock_h__


#ifdef _M_IX86
#define REG_MAX_NUM 8
typedef	unsigned long cb_addr;
#else
#define REG_MAX_NUM 16
typedef	unsigned long long cb_addr;
#endif

enum cb_seq_var_type{
	cbvar_none = 0,
	cbvar_int,
	cbvar_str,
	cbvar_setuuid,	//通过参数回传代码块功能ID
	cbvar_setpattern,	//回传pattern
	cbvar_setcomment	//回传comment
};

enum cb_debugmode{
	cb_dbg_byvm = 0,
	cb_dbg_direct
};
#define max_var_num		4

#define cbseq_flag_temp	0x4
struct cb_SeqExecute{
	int			seqid;	//代码块编号
	int			uuid;
	long		flags;	//标记
	cb_addr		ip;		//OPCODE地址
	long		eflags;	//eflag
	cb_addr		rip;
	cb_addr		regs[REG_MAX_NUM];	//对应上下文寄存器
	struct
	{
		int		type;	//变量类型
		union
		{
			signed long long sqword;
			char	byte[16];	//最长16个直接
		};
	}vars[max_var_num];	//最大4个变量
};
//直接建立代码块
int		cb_make_codeblock(int seqID,vaddr entry,int uuid,long flags,const wchar_t* pattern,const wchar_t* comment);
//发送块执行记录
void	cb_send_execlog(cb_SeqExecute* se);
//设定偏好的调试方式(虚拟方式/直接方式/混合方式)
void	cb_set_debugmode(int mode);
//是否已经触发断点
int		cb_isBreak(vaddr oip);
//获取运行状态(步过/运行/单步)
int		cb_getRunState();

#endif // codeblock_h__
