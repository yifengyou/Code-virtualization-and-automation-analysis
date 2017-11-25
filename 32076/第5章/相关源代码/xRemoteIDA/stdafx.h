// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件: 
#include <windows.h>

#define __NT__
#define  __IDP__
#include <ida.hpp>
#include <idp.hpp>
#include <err.h>
#include <md5.h>
#include <dbg.hpp>
#include <auto.hpp>
#include <name.hpp>
#include <frame.hpp>
#include <loader.hpp>
#include <diskio.hpp>
#include <struct.hpp>
#include <typeinf.hpp>
#include <demangle.hpp>
#include <nalt.hpp>
#include <bytes.hpp>
#pragma comment(lib,"ida.lib")
#pragma comment(lib,"pro.lib")


#include "../../../../nNetLib/npipeclient.h"
#include "../../../../nCom/npacketbaseT.h"


// TODO:  在此处引用程序需要的其他头文件
