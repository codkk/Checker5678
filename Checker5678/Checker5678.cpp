
// Checker5678.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "Checker5678.h"
#include "Checker5678Dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define TEMP_TEST

// CChecker5678App

BEGIN_MESSAGE_MAP(CChecker5678App, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CChecker5678App ����

CChecker5678App::CChecker5678App()
{
	// ֧����������������
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CChecker5678App ����

CChecker5678App theApp;


// CChecker5678App ��ʼ��

BOOL CChecker5678App::InitInstance()
{

	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()��  ���򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// ���� shell ���������Է��Ի������
	// �κ� shell ����ͼ�ؼ��� shell �б���ͼ�ؼ���
	CShellManager *pShellManager = new CShellManager;

	// ���Windows Native���Ӿ����������Ա��� MFC �ؼ�����������
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	//CoInitialize(NULL);
	if (AfxOleInit() == FALSE)
	{
		AfxMessageBox(_T("AfxOleInit FIAL!"));
	}
	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

	//��֤Ȩ��
#ifdef TEMP_TEST

#else
	loadLibrary();
	if (Lib_OpenLoginDlg)
		Lib_OpenLoginDlg(m_Name, m_Password, m_AdminCodeStr, m_Admin);
#endif // TEMP_TEST


	//debug
	m_Admin = 0;
	if (m_Admin == -1)
	{
		FreeLibrary(dllHandle);
		if (pShellManager != NULL)
		{
			delete pShellManager;
		}
		return FALSE;
	}
	else
	{

	}


	CChecker5678Dlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "����: �Ի��򴴽�ʧ�ܣ�Ӧ�ó���������ֹ��\n");
		TRACE(traceAppMsg, 0, "����: ������ڶԻ�����ʹ�� MFC �ؼ������޷� #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS��\n");
	}

	// ɾ�����洴���� shell ��������
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

#ifndef _AFXDLL
	ControlBarCleanUp();
#endif

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}


BOOL CChecker5678App::loadLibrary()
{
	/* code */
	TCHAR FilePath[MAX_PATH] = { 0 };
	GetModuleFileNameW(NULL, FilePath, MAX_PATH);
	(_tcsrchr(FilePath, _T('\\')))[1] = 0;
	TCHAR Lib_File[MAX_PATH] = { 0 };
	_swprintf(Lib_File, _T("%sMFCDataBase.dll"), FilePath);
	dllHandle = LoadLibraryW(Lib_File);

	//����ͼ��ͼ
	if (dllHandle == NULL) {
		AfxMessageBox(L"LoadLibraryA failed!\n");
		return false;
	}


	Lib_OpenLoginDlg = (pOpenLoginDlg)::GetProcAddress(dllHandle, "OpenLoginDlg");
	if (NULL == Lib_OpenLoginDlg)
	{
		AfxMessageBox(L"OpenLoginDlg failed!\n");
		return false;
	}
	Lib_OpenUserMangemantDlg = (pOpenUserMangemantDlg)::GetProcAddress(dllHandle, "OpenUserMangemantDlg");
	if (NULL == Lib_OpenUserMangemantDlg)
	{
		AfxMessageBox(L"OpenUserMangemantDlg failed!\n");
		return false;
	}

	return TRUE;
}

BOOL CChecker5678App::menageuser()
{
	Lib_OpenUserMangemantDlg(m_Name, m_Password, m_AdminCodeStr, m_Admin);
	return TRUE;
}

BOOL CChecker5678App::login()
{
	if (Lib_OpenLoginDlg)
		Lib_OpenLoginDlg(m_Name, m_Password, m_AdminCodeStr, m_Admin);
	if (m_Admin == -1)
	{
		return FALSE;
	}
	else
		return TRUE;
}
