
// Checker5678.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号


// CChecker5678App: 
// 有关此类的实现，请参阅 Checker5678.cpp
//
typedef void(*pOpenLoginDlg)(CString& Name, CString& Password, CString & StrCode, int & admin);
typedef void(*pOpenUserMangemantDlg)(CString Name, CString Password, CString StrCode, int admin);

class CChecker5678App : public CWinApp
{
public:
	CChecker5678App();

// 重写
public:
	virtual BOOL InitInstance();
	BOOL loadLibrary();
	BOOL menageuser();
	BOOL login();
// 实现
//权限管理
	HMODULE                   dllHandle;
	pOpenLoginDlg             Lib_OpenLoginDlg;
	pOpenUserMangemantDlg     Lib_OpenUserMangemantDlg;
	int                 m_Admin;             //-1 没有找到对应的用户或密码   0  超级管理者, 1  管理者, 2  操作者
	CString             m_AdminCodeStr;      //管理使用权限编码
	CString             m_Name;      //管理使用权限编码
	CString             m_Password;      //管理使用权限编码
	DECLARE_MESSAGE_MAP()
};

extern CChecker5678App theApp;