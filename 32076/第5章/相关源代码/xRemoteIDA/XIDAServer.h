#pragma once

#include "../../../../nNetLib/nNetBase.h"
#include "../../../../nCom/npacketbaseT.h"
#include "../../../../nCom/nsafelocker.h"
struct remote_ollydbg{
	nnet_inst pipe;
	NCLStringA	sid;
	struct remote_ollydbg()
	{
		pipe = 0;
	}
};


class XIDAServer :
	public NNetBase
{
public:
	XIDAServer();
	virtual ~XIDAServer();
	void broadcast(NPacketBuffer* pk);
protected:
	//实例化网络通信库远端连接建立通知虚函数
	virtual	BOOL	onInstOpen(nnet_inst inst, s_base_inst* binst);
	//实例化网络通信库远端连接关闭通知虚函数
	virtual void	onInstClose(nnet_inst inst, s_base_inst* binst);
	//实例化网络通信库远端连接异常通知虚函数
	virtual void	onInstError(nnet_inst inst, s_base_inst* binst, int err);
	//实例化网络通信库远端连接数据接收通知虚函数
	virtual int	onInstRead(nnet_inst inst, s_base_inst* binst, const char* pdata, size_t pszdata);

	NSafeLocker		m_odlk;
	NAvlList<NCLStringA,remote_ollydbg>	m_odbgs;
};

