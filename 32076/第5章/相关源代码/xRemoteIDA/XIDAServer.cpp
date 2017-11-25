#include "stdafx.h"
#include "XIDAServer.h"
#include "xRemoteIDA.h"
#include "../../xVMRuntime/xvmruntime.h"
XIDAServer::XIDAServer()
{
}


XIDAServer::~XIDAServer()
{
}

BOOL XIDAServer::onInstOpen(nnet_inst inst, s_base_inst* binst)
{
	fnSetInstOptions(inst,0,0,0,0,MAX_PIPE_PACKET,0,MAX_PIPE_PACKET);
	return TRUE;
}

void XIDAServer::onInstClose(nnet_inst inst, s_base_inst* binst)
{

}

void XIDAServer::onInstError(nnet_inst inst, s_base_inst* binst, int err)
{

}


BOOL getLoadedModuleInfo(xida_msg_info& info)
{
	uval_t uv = inf.baseaddr;

	ea_t start = inf.ominEA;
	ea_t end = inf.omaxEA;
	if (start == 0 || end == 0)
		return 0;
	info.modname[0] = 0;
	get_root_filename(info.modname,sizeof(info.modname) - 1);

	char bufpath[MAX_PATH];
	ssize_t szbuf = get_input_file_path(bufpath,sizeof(bufpath)-1);
	memset(info.peHdr,0,sizeof(info.peHdr));
	FILE* fp = fopen(bufpath,"rb");
	if (fp)
	{
		fseek(fp,0,SEEK_SET);
		fread(info.peHdr,0x1000,1,fp);
		fclose(fp);
	} else
		return FALSE;
	return TRUE;
}

void jumpto_sync(ULONG_PTR addr)
{
	struct ida_local jmp_exec : public exec_request_t
	{
		ULONG_PTR jmptoAddr;
		int idaapi execute(void)
		{
			jumpto(jmptoAddr + get_imagebase());
			switchto_tform(get_current_tform(),true);
			return 0;
		}
		jmp_exec(ULONG_PTR addr)
			:jmptoAddr(addr)
		{
		}
	};
	
	jmp_exec jt(addr);
	execute_sync(jt,MFF_FAST);
}
int XIDAServer::onInstRead(nnet_inst inst, s_base_inst* binst, const char* pdata, size_t pszdata)
{
	NStackAutoLocker sl(&m_odlk);
	if (pszdata < sizeof(xida_msg_hdr))
	{
		fnInstClose(inst);
		return -1;
	}
	switch (((xida_msg_hdr*)pdata)->msgid)
	{
	case xida_msgid_getinfo:
	{
		xida_msg_getinfo* gi = (xida_msg_getinfo*)pdata;
		NCLStringA ncla;
		ncla.set(gi->computerName, strlen(gi->computerName));
		ncla.append("_");
		char buf[16] = {0};
		itoa(gi->pid, buf, 16);
		ncla.append(buf);
		m_odlk.Lock();
		remote_ollydbg* rod = m_odbgs.new_back(ncla);
		m_odlk.Unlock();
		if (rod->sid != ncla || rod->pipe == 0)
		{
			rod->sid = ncla;
			if (rod->pipe != 0)
			{
				fnInstClose(rod->pipe);
				rod->pipe = 0;
			}
			TCHAR name[64];
			NString stmp = gi->computerName;
			_stprintf(name,REMOTEIDA_PIPE_NOTICE, stmp.c_str(),gi->pid);
			rod->pipe = ConnectToPipe(0,name);
		}



		xida_msg_info info;
		getLoadedModuleInfo(info);
		fpInstWrite(inst,&info,sizeof(info));
	}break;
	case xida_msgid_getsymbol:
	{
		xida_msg_symbol sym;
		xida_msg_syminfo* gsym = (xida_msg_syminfo*)pdata;
		sym.symname[0] = 0;
		ea_t eaddr = get_imagebase() + gsym->rva;

		if (gsym->flags & (sym_normal | sym_locallab))
		{
			get_name((gsym->flags & sym_locallab)?eaddr:BADADDR,eaddr,sym.symname,sizeof(sym.symname) - 1);
		}
		if (sym.symname[0] == 0 && (gsym->flags & sym_stack))
		{
			func_t* fc = get_func(eaddr); 
			if (fc)
			{
				for (int i = 0; i < 2;i++)
				{
					ea_t bda = calc_stkvar_struc_offset(fc,eaddr,i);
					if (bda != BADADDR)
					{
						member_t* mb = get_best_fit_member(get_frame(fc),bda);
						if (mb)
						{
							ssize_t spre = get_member_name(mb->id,sym.symname,sizeof(sym.symname));
							while (struc_t* stch = get_sptr(mb))
							{
								if (spre >= sizeof(sym.symname)-2) break;
								sym.symname[spre++] = '.';
								member_t* submb = get_best_fit_member(stch,bda - mb->soff);
								if (!submb) break;
								mb = submb;
								spre += get_member_name(mb->id,sym.symname+spre,sizeof(sym.symname) - spre);
							}
							break;
						}
					}
				}
			}
		}
		
		fpInstWrite(inst,&sym,sizeof(sym));
	}break;
	case xida_msgid_findsym:
	{
		xida_msg_syminfo sym;
		xida_msg_findsym* gsym = (xida_msg_findsym*)pdata;
		sym.rva = -1;
		ea_t eaddr = get_name_ea(BADADDR,gsym->symname);
		if (eaddr != BADADDR)
			sym.rva = eaddr - get_imagebase();
		fpInstWrite(inst,&sym,sizeof(sym));
	}break;
	case xida_msgid_setip:
	{
		xida_msg_setip* setip = (xida_msg_setip*)pdata;
		jumpto_sync(setip->rva);

	}break;
	case xida_msgid_getcmt:
	{
		xida_msg_getcmt* getcmt = (xida_msg_getcmt*)pdata;
		xida_msg_comment cmt;
		cmt.comment[0] = 0;
		ea_t eaddr = getcmt->rva + get_imagebase();
		ssize_t nlen = get_cmt(eaddr,false,cmt.comment,sizeof(cmt.comment));
		if (nlen == -1)
			nlen = get_cmt(eaddr,true,cmt.comment,sizeof(cmt.comment));
		if (nlen > 0)
			cmt.comment[nlen] = 0;
		fpInstWrite(inst,&cmt,sizeof(cmt));
	}break;
	}
	return pszdata;
}

void XIDAServer::broadcast(NPacketBuffer* pk)
{
	NStackAutoLocker sl(&m_odlk);
	sl.Lock();
	for (remote_ollydbg* rod = m_odbgs.first(); rod;rod = m_odbgs.next(rod))
	{
		fpInstWrite(rod->pipe,pk->data(),pk->size());
	}
}
