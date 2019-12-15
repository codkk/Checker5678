
// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // 从 Windows 头中排除极少使用的资料
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 某些 CString 构造函数将是显式的

// 关闭 MFC 对某些常见但经常可放心忽略的警告消息的隐藏
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC 核心组件和标准组件
#include <afxext.h>         // MFC 扩展


#include <afxdisp.h>        // MFC 自动化类



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC 对 Internet Explorer 4 公共控件的支持
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC 对 Windows 公共控件的支持
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // 功能区和控件条的 MFC 支持



#include "myCameraClass.h"
#include "ImProcChecker.h"
#include "SerialPort.h"
#include "sc_io.h"
#ifdef _DEBUG
#pragma comment(lib,"scio64.lib")
#else
#pragma comment(lib,"scio64.lib")
#endif


#import "C:\Program Files\Common Files\System\ado\msado15.dll" no_namespace rename("EOF", "adoEOF") rename("BOF", "adoBOF")


//3. IO状态定义
#define IO_VALID 0   //有效电平
#define IO_INVALID 1 //无效电平

#define IO_OUT_ON   0 //输出on
#define IO_OUT_OFF  1 //输出off

//4. 输入功能定义
#define IN_CHECKER_BT1 0//测试按钮1
#define IN_CHECKER_BT2 1//测试按钮1
#define IN_SENS_SUKER 2 //吸盘启动
#define IN_SENS_WEIGHT 3 //称重
#define IN_BOTTON_WEIGHT 4 //称重位按钮
#define IN_BUTTON_CCD 5 //ccd工位按钮
#define IN_SENS_SCAN 6 //扫描枪

//5. 输出功能定义
#define OUT_LIGHT_CAM 6  //相机光源
#define OUT_SUKER 5  //吸气
#define OUT_LIGHT_WEIGHT_OK 3 //称重ok灯
#define OUT_LIGHT_WEIGHT_NG 4 //称重ok灯
#define OUT_LIGHT_BARCODE_OK 0//条码检测 
#define OUT_LIGHT_ACC_OK 3//配件齐全检测
#define OUT_LIGHT_COVERY_OK 7//保护膜检测
#define OUT_LIGHT_RED 5
#define OUT_LIGHT_GREEN 6
#define OUT_LIGHT_BLUE 7

//6. 模板路径名
#define PATH_MODEL_LOGO "Task\\logo.shm"
#define PATH_MODEL_TRI "Task\\cardtri.shm"
#define PATH_MODEL_STOP "Task\\stop.shm"

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


