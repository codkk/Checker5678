
// Checker5678.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CChecker5678App: 
// �йش����ʵ�֣������ Checker5678.cpp
//
typedef void(*pOpenLoginDlg)(CString& Name, CString& Password, CString & StrCode, int & admin);
typedef void(*pOpenUserMangemantDlg)(CString Name, CString Password, CString StrCode, int admin);

class CChecker5678App : public CWinApp
{
public:
	CChecker5678App();

// ��д
public:
	virtual BOOL InitInstance();
	BOOL loadLibrary();
	BOOL menageuser();
	BOOL login();
// ʵ��
//Ȩ�޹���
	HMODULE                   dllHandle;
	pOpenLoginDlg             Lib_OpenLoginDlg;
	pOpenUserMangemantDlg     Lib_OpenUserMangemantDlg;
	int                 m_Admin;             //-1 û���ҵ���Ӧ���û�������   0  ����������, 1  ������, 2  ������
	CString             m_AdminCodeStr;      //����ʹ��Ȩ�ޱ���
	CString             m_Name;      //����ʹ��Ȩ�ޱ���
	CString             m_Password;      //����ʹ��Ȩ�ޱ���
	DECLARE_MESSAGE_MAP()
};

extern CChecker5678App theApp;