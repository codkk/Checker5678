
// Checker5678.cpp : 定义应用程序的类行为。
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


// CChecker5678App 构造

CChecker5678App::CChecker5678App()
{
	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CChecker5678App 对象

CChecker5678App theApp;


// CChecker5678App 初始化

BOOL CChecker5678App::InitInstance()
{

	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。  否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// 创建 shell 管理器，以防对话框包含
	// 任何 shell 树视图控件或 shell 列表视图控件。
	CShellManager *pShellManager = new CShellManager;

	// 激活“Windows Native”视觉管理器，以便在 MFC 控件中启用主题
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	//CoInitialize(NULL);
	if (AfxOleInit() == FALSE)
	{
		AfxMessageBox(_T("AfxOleInit FIAL!"));
	}
	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

	//验证权限
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
		// TODO: 在此放置处理何时用
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置处理何时用
		//  “取消”来关闭对话框的代码
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "警告: 对话框创建失败，应用程序将意外终止。\n");
		TRACE(traceAppMsg, 0, "警告: 如果您在对话框上使用 MFC 控件，则无法 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS。\n");
	}

	// 删除上面创建的 shell 管理器。
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

#ifndef _AFXDLL
	ControlBarCleanUp();
#endif

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
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

	//加载图像图
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
