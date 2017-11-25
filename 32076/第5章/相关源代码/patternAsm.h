#ifndef patternAsm_h__
#define patternAsm_h__

#include "../../../nCom/nlinkedlist.h"
#include "../../../nCom/nautolargebuffer.h"
#include "../../../ncvm/ncasm/ncasm86.h"
#include "../../../nCom/avltree.h"
#include "../../../3rdparty/distorm/mnemonics.h"
#include "../../../nCom/nexpress.h"

enum patternMode{
	mode_sequence,	//顺序模式
	mode_reverse	//逆序模式
};

enum pattern_inst_type{
	pattern_inst_none,	//空语句
	pattern_inst_instruction,//指令匹配语句
	pattern_inst_control,	//控制语句
	pattern_inst_express,	//表达式语句
	pattern_inst_memexpr	//内存表达式语句
};

#define op_varid_base	(1 << 24)	//计算变量内存索引id
enum pattern_op_type{	//变量类型
	pattern_op_none,
	pattern_op_any,	//任何操作数变量
	pattern_op_reg,	//寄存器操作数变量
	pattern_op_imm,	//常量变量
	pattern_op_mem,	//内存操作数变量
	pattern_op_size,	//大小变量
	pattern_op_mnemonic,	//指令变量
	pattern_op_memexpr,	//内存表达式变量
	pattern_op_explicit	//直接语句变量
};

//脚本变量结构，该结构存储所有变量类型值
struct pasm_operand{
	int					type;	//
	int					size;
	int					opcode;
	int					base;
	int					index;
	int					dispvar;
	int					scale;
	int					seg;
	int64_t				imm;

	explicit struct pasm_operand()
	{
		type = O_NONE;
		size = 0;
		base = R_NONE;
		index = R_NONE;
		scale = 0;
		imm = 0;
		seg = 0;
		dispvar = 0;
		opcode = I_UNDEFINED;
	}
};

#define  PATTERN_CFLAG_NONE				0
#define  PATTERN_CFLAG_NOREF			0x1	//不允许引用
#define  PATTERN_CFLAG_NOT				0x2	//反匹配
#define  PATTERN_CFLAG_INALLOP			0x4		//在所有操作数当中匹配
#define  PATTERN_CFLAG_REF				0x8 	//引用匹配
#define  PATTERN_CFLAG_STRICT			0x10	//严格操作数匹配
#define  PATTERN_CFLAG_8BIT				0x20	//操作数大小常量
#define  PATTERN_CFLAG_16BIT			0x40
#define  PATTERN_CFLAG_32BIT			0x80
#define  PATTERN_CFLAG_64BIT			0x100
//单一指令操作数定义
struct pattern_op{
	pattern_op_type type;	//操作数类型
	int				id;	//操作数变量id
	int				szid;	//操作数大小变量id
	long			cflags;	//操作数属性
};
//指令表达结构
struct pattern_inst_body{
	ncasm::x86::inst			ins;	//指令反汇编结构
	int							szid;	//size变量的id	-1表示没有
	int							mnemonicid;	//指令变量的id
	long						cflags;	//指令标记
	pattern_op					ops[OPERANDS_NO];	//操作数
	struct pattern_inst_body()
	{
		szid = 0;
		mnemonicid = 0;
		cflags = PATTERN_CFLAG_NONE;
		for (int i = 0; i < OPERANDS_NO; i++)
		{
			ops[i].type = pattern_op_none;
			ops[i].szid = 0;
			ops[i].cflags = 0;
			ops[i].id = 0;
		}
			
	}
};

struct pasm_opcode{
	ncasm::x86::mnemonic	mnem;
};

#define		PATTERN_FLAG_NOESPCHANGE	0x1	//不允许ESP有变化
#define		PATTERN_FLAG_NOEFLAGSCHG	0x2	//不允许Eflags有变化
#define		PATTERN_FLAG_NOT			0x4
#define		PATTERN_FLAG_LESSMATCH		0x8
struct patternInst
{
	pattern_inst_type			type;	//语句类型（匹配语句，控制语句，表达式语句等）
	int							flags;	//标记

	NLinkedList<pattern_inst_body>		insts;	//具体的匹配指令细节
	NLinkedList<pattern_inst_body>		rps;	//替换指令的具体系列

	//pattern_inst_instruction 类型
	int			minc;	//最小匹配次数
	int			maxc;	//最大匹配次数
	void*		userdata;
	int			matched_count;	//当前指令已经成功匹配次数	//for runtime

	//pattern_inst_express	//表达式语句使用
	int			varid;
	nexpr_expr*	expr;

	//memory express	//内存表达式语句使用
	pasm_operand* memexpr;

	//mnemonic control	//指令匹配语句使用
	NLinkedList<pasm_opcode>	mnemops;

	struct patternInst()
	{
		type = pattern_inst_none;
		flags = 0;
		userdata = 0;
		minc = 1;
		maxc = 1;
		expr = 0;
		memexpr = 0;
	}
	~patternInst()
	{
		if (expr)
		{
			delete expr;
			expr = 0;
		}
		if (memexpr)
		{
			delete memexpr;
			memexpr = 0;
		}
	}
};
#define  PATTERN_SEQ_FLAG_NONE	0
#define  PATTERN_SEQ_FLAG_OFF	0x1

#define		MAX_VAR_NEWONCE	0x10
struct seqRuntime{
	patternInst*				inst;		//当前指令位置
	int							count;		//匹配次数
	NAvlList<int,pasm_operand>	vars;		//变量
	NAvlList<int,pasm_operand>	opvars;	//动态变量匹配中使用
	int		navar;						//本次匹配中新申请的变量数量
	int		nvars[MAX_VAR_NEWONCE];		//申请的具体变量id,在匹配失败的时候将删除
	struct seqRuntime()
	{
		inst = 0;
		count = 0;
		navar = 0;
	}

};

struct patternSequence
{
	//编译时数据，这些数据在编译花型过程中产生，是我们比对的依据
	int								id;				//序列ID
	patternMode						mode;			//对比模式
	long							flags;			//该花型的全局标记
	int								maximumInst;	//最大对比指令数量
	NLinkedList<patternInst>		insts;			//花型所包含的具体语句
	//运行时数据，这包括在对比的时候生成的匹配数据和变量
	seqRuntime						crt;
	//这里我们维护一个双向链表来循环，主要目的是提升效率
	struct patternSequence*			prev;
	struct patternSequence*			next;

	std::string						comment;
	std::string						pattern;
	int								uuid;
	NAvlList<int,pasm_operand>		dyops;	//动态变量编译时产生

	struct patternSequence()
	{
		mode = mode_sequence;
		next = 0;
		maximumInst = 30;
		flags = PATTERN_SEQ_FLAG_NONE;
		uuid = -1;
	}
	patternInst* newinst(pattern_inst_type type)
	{
		patternInst* inst = insts.Allocate();
		insts.AddAtLast(inst);
		inst->type = type;
		return inst;
	}
};

typedef index_map<std::string,short> mnemonicmap;
class patternAsm{
public:
	patternAsm();
	virtual ~patternAsm();
	//花型载入及编译相关函数
	void loadMnemonic();
	void clearPatterns();
	int loadPatternFromFile(const wchar_t* filename);
	int loadPattern(const char* lpsrc, int szsrc);
	int parsePattern(const char* lpsrc, int szsrc);
	int compile(patternSequence* seq,const char* lpsrc, int szsrc);
	int compile(patternSequence* seq,NStringListA* nsl);
	//表达式计算相关函数
	int calculateExpress(patternSequence* seq,nexpr_expr* expr,int64_t& val);
	int calculateMemoryExpress(patternSequence* seq,pasm_operand* expr,pasm_operand* op);

	//花型匹配测试相关函数
	//初始化匹配，初始化过程中会选取可用花型以及设定匹配模式
	int initForTest(bool breverse);
	//测试具体的某一条指令，就像程序执行一样，测试过程中，
	// 如果发现不匹配的花型，将会被选取的花型链表中剔除
	// 我们不断测试剩下的匹配花型，直到剩下1个或者没有花型匹配我们就认为匹配结束了。
	// 不过我们的patternAsm类并不做这方面的管理，这需要使用该脚本的代码来自行管理
	int testInstruction(const char* lpop,int szop,ULONG_PTR pc,bool isEspChanged,bool isEflagsChanged,void* userdata,patternSequence*& sr);
	int testForSequence(patternSequence* seq,ncasm::x86::inst* ins,bool isEspChanged,bool isEflagsChanged,void* userdata);
	int testForInstruction(patternSequence* rt,patternInst* pi,ncasm::x86::inst* ins,bool isEspChanged,bool isEflagsChanged,void* userdata);

	int isMatchInst(patternSequence* seq,patternInst* pi,ncasm::x86::inst* ins,bool isEspChanged,bool isEflagsChanged);
	int isMatchInstBody(patternSequence* seq,patternInst* pi,pattern_inst_body* ib,ncasm::x86::inst* ins);
	int isMatchInstOperand(patternSequence* seq,patternInst* pi,pattern_inst_body* ib,pattern_op* pop,int ivop,ncasm::x86::operand* iop,int ix86op,ncasm::x86::inst* ins);
	//变量管理相关函数
	int varid(pattern_op_type type,int id);
	int checkAndSaveVar(patternSequence* seq,pattern_op* pop,ncasm::x86::inst* ins,ncasm::x86::operand* iop);
	int saveMemoryExprVar(patternSequence* seq,pattern_op* pop,ncasm::x86::inst* ins,ncasm::x86::operand* iop);
	int resolveVar(patternSequence* seq,pasm_operand* op,pattern_op* pop);
	//替换指令生成函数
	int genReplaceOpcode(patternSequence* seq,pattern_inst_body* pi,char* lpop,int szop,ULONG_PTR pc);

	patternSequence* getPattern(int pid);
private:
	//词法分析相关函数
  int parseLine(patternSequence* seq, const char* lpline, int len);
  int parseInst(patternSequence* seq,pattern_inst_body* inst,NStringA& line);
  int parseInstOption(patternInst* inst,NStringA& line);
  int parseExpress(patternInst* inst,NStringA& line);
  int parseMemoryExpress(patternInst* inst,NStringA& line);
  int parseMemoryOperand(pasm_operand* op,NStringA& line);
  int parseControlExpress(patternInst* inst,NStringA& line);
private:
	NAvlList<int, patternSequence>	m_Patterns;	//所有编译好的花型
	//这里我们维护一个简单的列表来表示当前可用的花型
	patternSequence*				m_seqList;
	mnemonicmap		m_mnemonics;
	ncasm::x86		m_x86asm;

};


#endif // patternAsm_h__
