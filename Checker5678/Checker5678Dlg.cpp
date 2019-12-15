
// Checker5678Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include <io.h>
#include <direct.h>
#include "Checker5678.h"
#include "Checker5678Dlg.h"
#include "afxdialogex.h"
#include "SetParDlg.h"
#include "AdoDatabase.h"
//#include "MyLogger.h"
#include "SettingParaDlg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define WM_MYMSG_BARCODE WM_USER+100   //刷新显示条形码
#define WM_MYMSG_ACCESSARY WM_USER+101   //刷新显示检测配件结果
#define WM_MYMSG_COVERY WM_USER+102   //刷新显示检测保护膜结果
#define WM_MYMSG_HEAVY WM_USER+103    //刷新显示检查重量结果
#define WM_MYMSG_BOOK WM_USER+105
#define WM_MYMSG_CARD WM_USER+106
#define WM_MYMSG_DRY WM_USER+107

#define WM_THREAD_MESSAGES WM_USER+104
//颜色
#define BG_COLOR_GREEN RGB(0, 255, 0)
#define BG_COLOR_RED RGB(255, 0, 0)

//返回值
#define RES_OK -1  //成功
#define RES_CAM 0  //相机采集错误
#define RES_NG1  1
#define RES_NG2  2
#define RES_NG3  3

#define PATH_DATA "D:\\Data"
#define PATH_PARA_SYS _T("Task\\system.ini")
#define PATH_COUNT _T("Task\\count.ini")
#define TXT_MSG_READING "正在读取中"
#define TXT_MSG_CHECKING "Checking"
#define TXT_MSG_OK "OK"
#define TXT_MSG_CAM "相机取图失败"
#define TXT_MSG_BOOK "NG"
#define TXT_MSG_CARD "NG"
#define TXT_MSG_DRY "NG"
#define TXT_MSG_COVERY "NG"

CCriticalSection g_criSectionIO;  //锁
int CChecker5678Dlg::g_num = 0;
//char Schar[65536];
int m_np = 0;
//工作线程
DWORD WINAPI AutoTestHandleThread(LPVOID lpParam)
{
	CChecker5678Dlg* pMdlg = (CChecker5678Dlg*)lpParam;
	int ret = pMdlg->MainProcess();
	return ret;
}

//称重线程（串口）
DWORD WINAPI AutoWeightHandleThread(LPVOID lpParam)
{
	CChecker5678Dlg* pMdlg = (CChecker5678Dlg*)lpParam;
	int ret = pMdlg->Weight();
	return ret;
}

//吸盘线程（IO口）
DWORD WINAPI AutoSuckerHandleThread(LPVOID lpParam)
{
	CChecker5678Dlg* pMdlg = (CChecker5678Dlg*)lpParam;
	int ret = pMdlg->Sucker();
	return ret;
}

// 判断文件夹是否存在
bool IsDirExist(const char *pszDir)
{
	if (pszDir == NULL)
		return false;

	return (_access(pszDir, 0) == 0);	// io.h
}

// 创建目录
bool CreateMyDirectory(const char *dir)
{
	if (NULL == dir)
		return false;

	char path[MAX_PATH];
	int nPathLen = 0;

	strcpy(path, dir);
	if ((nPathLen = strlen(path)) < 1)
		return false;

	for (int i = 0; i < nPathLen; ++i)
	{
		if (path[i] == '\\')
		{
			path[i] = '\0';
			_mkdir(path);	// <direct.h>
			path[i] = '\\';
		}
	}

	return (0 == _mkdir(path) || EEXIST == errno); // EEXIST => errno.h    errmo => stdlib.h
}

CChecker5678Dlg::CChecker5678Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CHECKER5678_DIALOG, pParent)
	, m_strMsgAccessary(_T(TXT_MSG_CHECKING))
	, m_strMsgCovery(_T(TXT_MSG_CHECKING))
	, m_strMsgBarCode(_T(TXT_MSG_READING))
	, m_bToCheckBarcode(TRUE)
	, m_toCheckAcc(TRUE)
	, m_bToCheckCovery(TRUE)
	, m_fWeight(0)
	, m_iTotal(0)
	, m_strMsgBook(_T(TXT_MSG_CHECKING))
	, m_strMsgCard(_T(TXT_MSG_CHECKING))
	, m_strMsgDry(_T(TXT_MSG_CHECKING))
	, m_bCurrent(FALSE)
{
	m_MinLength = 6;
	m_bToCheckBarcode2 = true;
	m_toCheckAcc2 = true;
	m_bToCheckCovery2 = true;
	m_pCam = NULL;
	m_iIndex = 0;		 //相机序号
	m_fExporTime = 55000;
	m_bexit = false;
	m_isPutOn = true;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bCamConneted = false;
	m_brPASS = CreateSolidBrush(BG_COLOR_GREEN);
	m_brFAIL = CreateSolidBrush(BG_COLOR_RED);
	m_hTestEvent = CreateEventW(NULL, TRUE, TRUE, NULL);
	m_bCheckBarcode = false;
	m_bCheckAccessary = false;
	m_bCheckCovery = false;
	m_bCheckBook = false;
	m_bCheckCard = false;
	m_bCheckDry = false;
	m_MaxWeight = 60.0;
	m_MinWeight = 20.0;
	m_bCheckWeight = false;
	com_weight = 0;
	com_boad = 9600;
	m_handle = NULL; 
	m_cardconnect = false;
	m_bSucker = false;
	//m_myLoger = MyLogger::getInstance();
	m_bConnectDB = false;
	m_bToStart = false;
	m_fTime = 2000;
	m_fDelayTime = 800;
	m_bWeight = false;
	m_btoShowMsg = true;
	m_iOkCount = 0;
	m_iNgCount =0;
	m_bButtonTrig = false;
}

CChecker5678Dlg::~CChecker5678Dlg()
{
	DeleteObject(m_brPASS);
	DeleteObject(m_brFAIL);
	CloseHandle(m_hTestEvent);
	//释放相机
	if (m_pCam)
	{
		m_pCam->StopGrabbing();
		m_pCam->CloseDevice();
		m_pCam->DestroyDevice();
		delete m_pCam;
		m_pCam = NULL;
	}
	//释放IO控制板卡资源
	if (m_handle)
		NIO_Close(m_handle);
}

void CChecker5678Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_SHOW, m_show);
	DDX_Text(pDX, IDC_EDIT_RESULT1, m_strMsgAccessary);
	DDX_Text(pDX, IDC_EDIT_RESULT2, m_strMsgCovery);
	DDX_Text(pDX, IDC_EDIT_RESULT3, m_strMsgBarCode);
	DDX_Check(pDX, IDC_CHECK1, m_bToCheckBarcode);
	DDX_Check(pDX, IDC_CHECK2, m_toCheckAcc);
	DDX_Check(pDX, IDC_CHECK3, m_bToCheckCovery);
	DDX_Text(pDX, IDC_EDIT_RESULT4, m_fWeight);
	DDX_Control(pDX, IDC_LISTMSG, m_listMsg);
	DDX_Text(pDX, IDC_EDIT_TOTAL, m_iTotal);
	DDX_Text(pDX, IDC_EDIT_RESULT6, m_strMsgBook);
	DDX_Text(pDX, IDC_EDIT_RESULT7, m_strMsgCard);
	DDX_Text(pDX, IDC_EDIT_RESULT8, m_strMsgDry);
	DDX_Check(pDX, IDC_CHECK4, m_bCurrent);
	DDX_Control(pDX, IDC_BUTTON_DRAWAREA, m_btn_draw1);
}

BEGIN_MESSAGE_MAP(CChecker5678Dlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//ON_BN_CLICKED(IDCANCEL, &CChecker5678Dlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CChecker5678Dlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_START, &CChecker5678Dlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_TEST, &CChecker5678Dlg::OnBnClickedButtonTest)
	ON_WM_CLOSE()
	ON_MESSAGE(WM_MYMSG_BARCODE, &CChecker5678Dlg::ShowResultBarcode)
	ON_MESSAGE(WM_MYMSG_ACCESSARY, &CChecker5678Dlg::ShowResultAccess)
	ON_MESSAGE(WM_MYMSG_COVERY, &CChecker5678Dlg::ShowResultCovery)
	ON_MESSAGE(WM_MYMSG_HEAVY, &CChecker5678Dlg::ShowResultHaevy)
	ON_MESSAGE(WM_MYMSG_BOOK, &CChecker5678Dlg::ShowResultBook)
	ON_MESSAGE(WM_MYMSG_CARD, &CChecker5678Dlg::ShowResultCard)
	ON_MESSAGE(WM_MYMSG_DRY, &CChecker5678Dlg::ShowResultDry)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_CHECK1, &CChecker5678Dlg::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_CHECK2, &CChecker5678Dlg::OnBnClickedCheck2)
	ON_BN_CLICKED(IDC_CHECK3, &CChecker5678Dlg::OnBnClickedCheck3)
	ON_BN_CLICKED(IDC_BUTTON_START2, &CChecker5678Dlg::OnBnClickedButtonStart2)
	ON_MESSAGE(WM_COMM_RXSTR, &CChecker5678Dlg::OnReceiveStr)
	ON_BN_CLICKED(IDC_BUTTON_START3, &CChecker5678Dlg::OnBnClickedButtonStart3)
	ON_BN_CLICKED(IDC_BUTTON2, &CChecker5678Dlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CChecker5678Dlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON_TEST2, &CChecker5678Dlg::OnBnClickedButtonTest2)
	ON_BN_CLICKED(IDC_BUTTON_DRAWAREA, &CChecker5678Dlg::OnBnClickedButtonDrawarea)
	ON_BN_CLICKED(IDC_BUTTON_DRAWAREA2, &CChecker5678Dlg::OnBnClickedButtonDrawarea2)
	ON_BN_CLICKED(IDC_BUTTON_START4, &CChecker5678Dlg::OnBnClickedButtonStart4)
	ON_BN_CLICKED(IDC_BUTTON_TEST3, &CChecker5678Dlg::OnBnClickedButtonTest3)
	ON_BN_CLICKED(IDC_BUTTON5, &CChecker5678Dlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &CChecker5678Dlg::OnBnClickedButton6)
END_MESSAGE_MAP()


// CChecker5678Dlg 消息处理程序

BOOL CChecker5678Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	//::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BITMAP1));
	//m_btn_draw1.AutoLoad(IDC_BUTTON_START, this);
	//HBITMAP   hBitmap;
	//hBitmap = LoadBitmap(AfxGetInstanceHandle(),
	//	MAKEINTRESOURCE(IDB_BITMAP1));
	////((CButton *)GetDlgItem(IDC_BUTTON_DRAWAREA))->SetBitmap(hBitmap);
	//m_btn_draw1.LoadBitmaps(IDB_BITMAP1);
	//m_btn_draw1.SizeToContent();
	//m_btn_draw1.LoadBitmaps(IDB_BITMAP1);
	// TODO: 在此添加额外的初始化代码
	//字体
	//CStaticShow(IDC_EDIT_RESULT3, _T(TXT_MSG_READING), 200);
	//CStaticShow(IDC_EDIT_RESULT1, _T(TXT_MSG_CHECKING), 200);
	//CStaticShow(IDC_EDIT_RESULT2, _T(TXT_MSG_CHECKING), 200);
	//CStaticShow(IDC_EDIT_RESULT6, _T(TXT_MSG_CHECKING), 200);
	//CStaticShow(IDC_EDIT_RESULT7, _T(TXT_MSG_CHECKING), 200);
	//CStaticShow(IDC_EDIT_RESULT8, _T(TXT_MSG_CHECKING), 200);
	//CStaticShow(IDC_EDIT_RESULT4, _T("0.0"), 400);
	//窗口
	CRect rect;
	m_show.GetClientRect(&rect);
	//m_mydll.SetHalconDispWnd(m_show.GetSafeHwnd(), rect);
	ImProcChecker::SetDispWnd((Hlong)m_show.GetSafeHwnd(), rect);
	//数据文件保存位置
	if (!IsDirExist(PATH_DATA))
	{
		if (!CreateMyDirectory(PATH_DATA))
		{
			AfxMessageBox(_T("创建DATA文件夹失败"));
		}
	}
	//按时间（天）创建数据文件
	//连接相机
	LoadPara(PATH_PARA_SYS);
	m_iIndex = g_num++;
	m_pCam = NULL;
	m_pCam = new myCameraClass(m_iIndex);
	if (MV_OK == m_pCam->CameraInit(NULL/*GetDlgItem(IDC_STATIC_SHOW)*/))  //不传显示窗口句柄
	{
		m_pCam->SetExposureTime(m_fExporTime);
		m_pCam->SetAcquisitionMode(MV_ACQ_MODE_CONTINUOUS);
		//m_pCam->RegisterImageCallBack(ImageCallBackEx, this);
		m_pCam->SetTriggerMode(MV_TRIGGER_MODE_OFF);
		//m_pCam->SetTriggerSource(MV_TRIGGER_SOURCE_SOFTWARE);
		m_pCam->StartGrabbing();  //开始视频流
		m_bCamConneted = true;
	}
	else
	{
		m_bCurrent = TRUE;
		m_bCamConneted = false;
		AfxMessageBox(_T("连接相机失败."));
	}
	
	//绑定相机到算法
	ImProcChecker::SetCam(m_pCam);

	//
	//LoadCount(PATH_COUNT);

	//连接称重计串口

	m_ProcMeasureUSB.LoadPara(PATH_PARA_MEASURE_USB);
	m_ProcMeasureCircle.LoadPara(PATH_PARA_CIRCLE);
	//加载任务
	//m_ProcCheck1_Book.SetType(1);
	////m_ProcCheck2_CardTitle.SetType(2);
	//m_ProcCheck3_CardTri.SetType(3);
	//m_ProcCheck4_Dry.SetType(4);
	//m_ProcCheck5_Covery.SetType(5);
	//m_ProcCheck6_Code.SetType(6);
	//m_ProcCheck7_Stop.SetType(7);
	//if (!m_ProcCheck1_Book.LoadModel(PATH_MODEL1)) //说明书
	//{
	//	AfxMessageBox(_T("加载说明书LOGO模板失败，请检查logo.shm文件。"));
	//}
	////m_ProcCheck2_CardTitle.LoadModel(PATH_MODEL2); //卡片标题
	//if (!m_ProcCheck3_CardTri.LoadModel(PATH_MODEL3)) //卡片三角
	//{
	//	AfxMessageBox(_T("加载卡片图案模板失败，请检查cardtri.shm文件。"));
	//}
	//if (!m_ProcCheck7_Stop.LoadModel(PATH_MODEL7))
	//{
	//	AfxMessageBox(_T("加载模具图案模板失败，请检查stop.shm文件。"));
	//}

	//if (!m_ProcCheck1_Book.LoadDeformModel(PATH_MODEL_LOGO_DFM))//加载logo的可形变模板
	//{
	//	AfxMessageBox(_T("加载说明书LOGO模板失败，请检查stop.dfm文件。"));
	//}

	//m_ProcCheck1_Book.LoadPara(PATH_PARA_LOGO); //说明书
	////m_ProcCheck2_CardTitle.LoadPara(PATH_PARA_CARDTITLE); //卡片标题
	//m_ProcCheck3_CardTri.LoadPara(PATH_PARA_CARDTRI); //卡片三角
	//m_ProcCheck4_Dry.LoadPara(PATH_PARA_DRY); //干燥剂
	//m_ProcCheck5_Covery.LoadPara(PATH_PARA_COVER); //保护膜
	//m_ProcCheck6_Code.LoadPara(PATH_PARA_CODE);
	//m_ProcCheck7_Stop.LoadPara(PATH_PARA_STOP);

	m_bexit = false;
	ResetEvent(m_hTestEvent);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AutoTestHandleThread, this, 0, NULL);
	UpdateData(FALSE);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CChecker5678Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		//CRect rect;
		//CPaintDC dc(this);
		//GetClientRect(&rect);
		//dc.FillSolidRect(rect, RGB(0, 32, 135));
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CChecker5678Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}




void CChecker5678Dlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}


void CChecker5678Dlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	//判断是否停止
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_hTestEvent, 10)) //如果停止
	{
		AfxMessageBox(_T("请先使程序停止，在关闭程序。"));
		return;
	}
	//LOG4CPLUS_DEBUG(m_myLoger->logger, "退出===================");
	m_bexit = true;
	SetEvent(m_hTestEvent);
	m_SerialPort.ClosePort();
	Sleep(500);
	InitIO();
	//LOG4CPLUS_DEBUG(m_myLoger->logger, "退出 完成==============");
	CDialogEx::OnOK();
}


BOOL CChecker5678Dlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
		case VK_RETURN: //屏蔽回车键
			return TRUE;
		case VK_ESCAPE: //屏蔽ESC键
			return TRUE;
		default:
			break;
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

//开始
void CChecker5678Dlg::OnBnClickedButtonStart()
{	
	m_listMsg.ResetContent();
	UpdateData();
	if (!m_bCurrent)
	{
		if (!m_ProcMeasureUSB.GrabOneAndShow())
		{
			ShowMsg(_T("相机采集失败."));
			AfxMessageBox(_T("相机采集失败."));
			return;
		}
	}


	if (m_ProcMeasureUSB.MeasureUSB() != 0)
	{
		MeasureResult res;
		m_ProcMeasureUSB.GetReult(res);
		CString str;
		str.Format(_T("识别失败， 返回码： %d"), res.errorCode);
		ShowMsg(str);
	}
	else
	{
		MeasureResult res;
		m_ProcMeasureUSB.GetReult(res);
		CString str;
		str.Format(_T("length： %.4f"), res.length);
		ShowMsg(str);
		str.Format(_T("width： %.4f"), res.width);
		ShowMsg(str);
		str.Format(_T("phi： %.4f"), res.angle);
		ShowMsg(str);
		str.Format(_T("cent_x： %.4f"), res.cent_x);
		ShowMsg(str);
		str.Format(_T("cent_y： %.4f"), res.cent_y);
		ShowMsg(str);
		str.Format(_T("dis_rr： %.4f"), res.dis_rr);
		ShowMsg(str);
		str.Format(_T("up margin： %.4f"), res.up_margin);
		ShowMsg(str);
		str.Format(_T("down margin： %.4f"), res.dn_margin);
		ShowMsg(str);

	}
	return;
	//if (!m_bCamConneted)
	//{
	//	AfxMessageBox(_T("相机连接不成功，请排除后重启软件。"));
	//	return;
	//}
	CString str;
	GetDlgItemTextW(IDC_BUTTON_START, str);
	if (str == _T("开始START"))
	{
		//初始化变量
		m_bCheckBarcode = false;
		m_bCheckAccessary = false;
		m_bCheckBook = false;
		m_bCheckCard = false;
		m_bCheckDry = false;
		m_bCheckCovery = false;
		m_listMsg.ResetContent();
		m_btoShowMsg = true;
		//初始化IO
		MyWriteBit(OUT_LIGHT_CAM, IO_OUT_ON); //打开光源
		Sleep(100);
		//发送开始信号
		SetEvent(m_hTestEvent);
		//界面显示
		SetDlgItemTextW(IDC_BUTTON_START, _T("停止STOP"));
		//LOG4CPLUS_DEBUG(m_myLoger->logger, "开始===================");
	}
	else
	{
		//发送停止
		ResetEvent(m_hTestEvent);
		//界面显示
		SetDlgItemTextW(IDC_BUTTON_START, _T("开始START"));
		//LOG4CPLUS_DEBUG(m_myLoger->logger, "停止===================");
	}

	Sleep(500);
}

int CChecker5678Dlg::Mainread()
{
	int state = 0;
	int ret = 0;
	CheckResult res;
	m_bCheckBarcodeOnce = false;
	m_bCheckAccessaryOnce = false;
	m_bCheckCoveryOnce = false;
	m_testres.vTestResult.clear();
	m_testres = TestRes();
	COleDateTime t;
	CString strtime;
	while (1)
	{
		//判断点击了停止
		if (WAIT_TIMEOUT == WaitForSingleObject(m_hTestEvent, 20))
		{
			MyWriteBit(OUT_LIGHT_CAM, IO_OUT_OFF); //关闭光源
			//清空状态
			state = 0;
		}
		//等待开始
		WaitForSingleObject(m_hTestEvent, INFINITE);

		//退出
		if (m_bexit) break;

		switch (state)
		{
		case -1://定格状态直到检测到停止标志
				

				break;
		case 0:
				m_bCheckBarcodeOnce = false; 
				m_bCheckAccessaryOnce = false;
				m_bCheckCoveryOnce = false;

				ImProcChecker::g_hoBarCodeArea.Clear();
				ImProcChecker::g_hoBlobArea.Clear();
				m_ProcCheck1_Book.m_hoShape.Clear();
				m_ProcCheck1_Book.m_hoDeformShape.Clear();
				//m_ProcCheck2_CardTitle.m_hoShape.Clear();
				m_ProcCheck3_CardTri.m_hoShape.Clear();
				m_testres.vTestResult.clear();
				m_testres = TestRes();

				//0 读取一维码,读取成功则 跳转到 检查配件环节
				if (!m_bToCheckBarcode) //如果不检测条形码，则跳到检查配件
				{
					//SendMessage(WM_MYMSG_BARCODE, 0, 0);
					state = 1;
					break;
				}

				if (ReadBarCode()!= RES_OK)//检测不到条形码
				{
					state = 0;
					SendMessage(WM_MYMSG_BARCODE, 0, 0);
					break;
				}
				//发送消息给界面显示一维码
				m_ProcCheck6_Code.GetReult(res);
				//res.code = "1234567890ABC";
				SendMessage(WM_MYMSG_BARCODE, 1, (LPARAM)&res.code);
				m_bCheckBarcodeOnce = true;
				//m_testres.nSerialNumber.Format(_T("%s"), res.code);
				m_testres.nSerialNumber = res.code.c_str();
				t = COleDateTime::GetCurrentTime();
				strtime.Format(_T("%04d%02d%02d %02d:%02d:%02d"), t.GetYear(), t.GetMonth(), t.GetDay(), t.GetHour(), t.GetMinute(), t.GetSecond());
				m_testres.nTestTime = strtime;
				m_listMsg.ResetContent();
				//m_btoShowMsg = true;
				ShowMsg(_T(" 检测到条码：") + m_testres.nSerialNumber);
				state = 1;
				break;
		case 1:
				if (!m_toCheckAcc) //如果不检测配件，则跳到检查保护膜
				{
					//SendMessage(WM_MYMSG_ACCESSARY, 0, 4);
					state = 2;
					break;
				}
				
				//1. 检查是否有说明书，干燥剂， 卡片
				ret = 0;
				ret = CheckAccessary();
				showContour();//显示所有轮廓
				if (ret != RES_OK)//缺失
				{
					state = 1;
					SendMessage(WM_MYMSG_ACCESSARY, 0, ret);
					break;
				}
				SendMessage(WM_MYMSG_ACCESSARY, 1, ret); //成功
				m_bCheckAccessaryOnce = true;
				//m_btoShowMsg = true;
				//保存说明书和干燥剂的状态
				m_testres.vTestResult.push_back(_T("OK")); //说明书OK
				m_testres.vTestResult.push_back(_T("OK")); //干燥剂OK
				ShowMsg(_T(" 检测到全部配件OK"));
				state = -1;	//转到定格状态
				SaveOnlineData(_T(PATH_DATA), m_testres);
				//m_btoShowMsg = true;
				ShowMsg(_T(" 已保存记录到本地"));
				//m_btoShowMsg = true;
				ShowMsg(_T(" 请下料"));
				m_iTotal++;
				SaveCount(PATH_COUNT);

				break;
		case 2://
				if (!m_bToCheckCovery) //如果不检测保护膜，则跳到检查条形码
				{
					//SendMessage(WM_MYMSG_COVERY, 0, 0);
					state = 0;
					break;
				}
				
				//2. 检查保护膜
				ret = CheckCovery();
				showContour();//显示所有轮廓
				if (ret != RES_OK)
				{
					SendMessage(WM_MYMSG_COVERY, 0, 0);
					state = 2;
					break;
				}
				else//检测到保护膜
				{
					//m_btoShowMsg = true;
					ShowMsg(_T(" 检测到薄膜"));
					state = -1;	//转到定格状态
					SendMessage(WM_MYMSG_COVERY, 1, 0);
					m_bCheckCoveryOnce = true;
					SaveOnlineData(_T(PATH_DATA), m_testres);
					//m_btoShowMsg = true;
					ShowMsg(_T(" 已保存记录到本地"));
					//m_btoShowMsg = true;
					ShowMsg(_T(" 请下料"));
					m_iTotal++;
					SaveCount(PATH_COUNT);
				}

				break;
		default:
			//LOG4CPLUS_DEBUG(m_myLoger->logger, "getFixReady()  == ST_READY in ");
			state = 0;
			break;
		}

		Sleep(10);
		if (CheckStop() == RES_OK)//检测停止信号
		{
			state = 0;
			//保存数据
			
			//清空所有信息
			SendMessage(WM_MYMSG_BARCODE, 0, 0);
			m_testres.vTestResult.clear();
			m_testres = TestRes();
			m_listMsg.ResetContent();
			ShowMsg(_T(" 等待产品.."));
		}
		else
		{
			showContour();//显示所有轮廓
		}
	}
	return 0;
}

int CChecker5678Dlg::Mainread2()
{
	int state = 0;
	int ret = 0;
	CheckResult res;
	m_bCheckBarcodeOnce = false;
	m_bCheckAccessaryOnce = false;
	m_bCheckCoveryOnce = false;
	m_testres.vTestResult.clear();
	m_testres = TestRes();
	COleDateTime t;
	CString strtime;
	while (1)
	{
		//判断点击了停止
		if (WAIT_TIMEOUT == WaitForSingleObject(m_hTestEvent, 20))
		{
			MyWriteBit(OUT_LIGHT_CAM, IO_OUT_OFF); //关闭光源
												   //清空状态
			state = 0;
		}
		//等待开始
		WaitForSingleObject(m_hTestEvent, INFINITE);

		//退出
		if (m_bexit) break;

		//检查按钮是否按下
		if (MyReadBit(IN_BUTTON_CCD) == IO_VALID)
		{

		}

		switch (state)
		{
		case -1://定格状态直到检测到停止标志


			break;
		case 0:
			m_bCheckBarcodeOnce = false;
			m_bCheckAccessaryOnce = false;
			m_bCheckCoveryOnce = false;

			ImProcChecker::g_hoBarCodeArea.Clear();
			ImProcChecker::g_hoBlobArea.Clear();
			m_ProcCheck1_Book.m_hoShape.Clear();
			m_ProcCheck1_Book.m_hoDeformShape.Clear();
			//m_ProcCheck2_CardTitle.m_hoShape.Clear();
			m_ProcCheck3_CardTri.m_hoShape.Clear();
			m_testres.vTestResult.clear();
			m_testres = TestRes();

			//0 读取一维码,读取成功则 跳转到 检查配件环节
			if (!m_bToCheckBarcode) //如果不检测条形码，则跳到检查配件
			{
				//SendMessage(WM_MYMSG_BARCODE, 0, 0);
				state = 1;
				break;
			}

			if (ReadBarCode() != RES_OK)//检测不到条形码
			{
				state = 0;
				SendMessage(WM_MYMSG_BARCODE, 0, 0);
				break;
			}
			//发送消息给界面显示一维码
			m_ProcCheck6_Code.GetReult(res);
			//res.code = "1234567890ABC";
			SendMessage(WM_MYMSG_BARCODE, 1, (LPARAM)&res.code);
			m_bCheckBarcodeOnce = true;
			//m_testres.nSerialNumber.Format(_T("%s"), res.code);
			m_testres.nSerialNumber = res.code.c_str();
			t = COleDateTime::GetCurrentTime();
			strtime.Format(_T("%04d%02d%02d %02d:%02d:%02d"), t.GetYear(), t.GetMonth(), t.GetDay(), t.GetHour(), t.GetMinute(), t.GetSecond());
			m_testres.nTestTime = strtime;
			m_listMsg.ResetContent();
			//m_btoShowMsg = true;
			ShowMsg(_T(" 检测到条码：") + m_testres.nSerialNumber);
			state = 1;
			break;
		case 1:
			if (!m_toCheckAcc) //如果不检测配件，则跳到检查保护膜
			{
				//SendMessage(WM_MYMSG_ACCESSARY, 0, 4);
				state = 2;
				break;
			}

			//1. 检查是否有说明书，干燥剂， 卡片
			ret = 0;
			ret = CheckAccessary();
			showContour();//显示所有轮廓
			if (ret != RES_OK)//缺失
			{
				state = 1;
				SendMessage(WM_MYMSG_ACCESSARY, 0, ret);
				break;
			}
			SendMessage(WM_MYMSG_ACCESSARY, 1, ret); //成功
			state = 2;
			m_bCheckAccessaryOnce = true;
			//m_btoShowMsg = true;
			ShowMsg(_T(" 检测到全部配件OK,请放薄膜"));
			break;
		case 2:
			if (!m_bToCheckCovery) //如果不检测保护膜，则跳到检查条形码
			{
				//SendMessage(WM_MYMSG_COVERY, 0, 0);
				state = 0;
				break;
			}

			//2. 检查保护膜
			ret = CheckCovery();
			showContour();//显示所有轮廓
			if (ret != RES_OK)
			{
				SendMessage(WM_MYMSG_COVERY, 0, 0);
				state = 2;
				break;
			}
			else//检测到保护膜
			{
				//m_btoShowMsg = true;
				ShowMsg(_T(" 检测到薄膜"));
				state = -1;	//转到定格状态
				SendMessage(WM_MYMSG_COVERY, 1, 0);
				m_bCheckCoveryOnce = true;
				SaveOnlineData(_T(PATH_DATA), m_testres);
				//m_btoShowMsg = true;
				ShowMsg(_T(" 已保存保存记录到本地"));
				//m_btoShowMsg = true;
				ShowMsg(_T(" 请下料"));
				m_iTotal++;
				SaveCount(PATH_COUNT);
			}

			break;
		default:
			//LOG4CPLUS_DEBUG(m_myLoger->logger, "getFixReady()  == ST_READY in ");
			state = 0;
			break;
		}

		Sleep(10);
		if (CheckStop() == RES_OK)//检测停止信号
		{
			state = 0;
			//保存数据

			//清空所有信息
			SendMessage(WM_MYMSG_BARCODE, 0, 0);
			m_testres.vTestResult.clear();
			m_testres = TestRes();
			m_listMsg.ResetContent();
			ShowMsg(_T(" 等待产品.."));
		}
		else
		{
			showContour();//显示所有轮廓
		}
	}
	return 0;
}

int CChecker5678Dlg::MainProcess()
{
	int state = 0;
	int ret = 0;
	CheckResult res;
	m_bCheckBarcodeOnce = false;
	m_bCheckAccessaryOnce = false;
	m_bCheckCoveryOnce = false;
	m_testres.vTestResult.clear();
	m_testres = TestRes();
	COleDateTime t;
	CString strtime;
	while (1)
	{
		//判断点击了停止
		if (WAIT_TIMEOUT == WaitForSingleObject(m_hTestEvent, 20))
		{
			MyWriteBit(OUT_LIGHT_CAM, IO_OUT_OFF); //关闭光源
												   //清空状态
			state = 0;
		}
		//等待开始
		WaitForSingleObject(m_hTestEvent, INFINITE);

		//退出
		if (m_bexit) break;
	}
	return 0;
}

int CChecker5678Dlg::Weight()
{
	m_bWeight = false;
	while (1)
	{
		//退出
		if (m_bexit) break;

		//称重感应信号
		if (MyReadBit(IN_SENS_WEIGHT) == IO_VALID)
		{
			m_bWeight = true;
		}
		else
		{
			m_bWeight = false;
		}

		//发送消息
		Sleep(200);
	}
	return 0;
}

int CChecker5678Dlg::Sucker()
{
	m_bSucker = true;
	while (1)
	{
		//退出
		if (m_bexit) break;
		if(MyReadBit(IN_SENS_SUKER) == IO_VALID)//如果检测到吸住信号，则启动吸盘
		{
			Sleep(m_fDelayTime); //迟滞200毫秒再检测
			if (MyReadBit(IN_SENS_SUKER) == IO_VALID)//如果有信号，则
			{
				if (m_bSucker)
				{
					m_bSucker = false;
					//计时几秒钟后自动断开
					MyWriteBit(OUT_SUKER, IO_OUT_ON);
					
					//不断查询是否有信号
					double MyTime = m_fTime/1000; //单位秒
					double StepTime = 0.800; //查询间隔
					LARGE_INTEGER MynFreq;
					LARGE_INTEGER MynBeginTime;
					LARGE_INTEGER MyStepBeginTime;
					LARGE_INTEGER MynEndTime;
					double MynFreq_time = 0;
					double MyStepFreq_time = 0;
					QueryPerformanceFrequency(&MynFreq);
					QueryPerformanceCounter(&MynBeginTime); //起始时间
					QueryPerformanceCounter(&MyStepBeginTime);
					while (MynFreq_time < MyTime) //总时间不能超过设置的时间
					{
						QueryPerformanceCounter(&MynEndTime);//结束时间
						MynFreq_time = (double)(MynEndTime.QuadPart - MynBeginTime.QuadPart) / (double)MynFreq.QuadPart;
						MyStepFreq_time = (double)(MynEndTime.QuadPart - MyStepBeginTime.QuadPart) / (double)MynFreq.QuadPart;
						
						if (MyStepFreq_time >= StepTime) //间隔时间就检查一次
						{
							if (MyReadBit(IN_SENS_SUKER) == IO_INVALID)//检测到无信号，则跳出循环
							{
								Sleep(100); //迟滞100毫秒再检测，防止抖动
								if (MyReadBit(IN_SENS_SUKER) == IO_INVALID)
								{
									break;
								}
							}
							QueryPerformanceCounter(&MyStepBeginTime);//否则重新计时
						}
					}
					//Sleep(m_fTime);
					MyWriteBit(OUT_SUKER, IO_OUT_OFF);
				}
			}

		}
		else
		{
			m_bSucker = true;
		}
		
		Sleep(200);
	}
	return 0;
}

//-1: 读取成功
//0： 取图失败
//1: 读取失败
BOOL CChecker5678Dlg::ReadBarCode()
{
	//debug
	//return -1;

	BOOL flag = FALSE;
	//采图
	HObject Imgsrc;
	if (!m_ProcCheck6_Code.GrabImageFromCam(Imgsrc))
	{
		//LOG4CPLUS_DEBUG(m_myLoger->logger, "ReadBarCode 相机错误");
		return RES_CAM;
	}
	if (m_ProcCheck6_Code.CheckBarcode(Imgsrc))
	{
		return RES_OK;
	}
	else
	{
		return  RES_NG1;
	}
	return RES_OK;
}

//-1: 成功
//0: 取图失败
//1: 说明书缺失
//2：卡片缺失
//3：干燥剂缺失
BOOL CChecker5678Dlg::CheckAccessary()
{
	//debug
	//return -1;

	BOOL flag = RES_OK;
	HObject Imgsrc;
	if (!m_ProcCheck1_Book.GrabImageFromCam(Imgsrc))
	{
		//LOG4CPLUS_DEBUG(m_myLoger->logger, "CheckAccessary 相机错误");
		return  RES_CAM;
	}
	ImProcChecker::ContoursAll();
	//检查说明书
	if (m_ProcCheck1_Book.CheckDeformShape(Imgsrc))
	{
		SendMessage(WM_MYMSG_BOOK, 1, 0); //成功
	}
	else
	{
		if (m_ProcCheck1_Book.m_hoDeformShape.IsInitialized())
			m_ProcCheck1_Book.m_hoDeformShape.Clear();
		SendMessage(WM_MYMSG_BOOK, 0, RES_NG1); //缺失
		flag = RES_NG1; //说明书缺失
	}
	//if (m_ProcCheck1_Book.CheckShape(Imgsrc))
	//{
	//	SendMessage(WM_MYMSG_BOOK, 1, 0); //成功
	//}
	//else
	//{
	//	if (m_ProcCheck1_Book.m_hoShape.IsInitialized())
	//		m_ProcCheck1_Book.m_hoShape.Clear();
	//	SendMessage(WM_MYMSG_BOOK, 0, RES_NG1); //缺失
	//	flag =  RES_NG1; //说明书缺失
	//}
	//检查卡片
	if (0)
	{
		if (m_ProcCheck3_CardTri.CheckShape(Imgsrc)/* || m_ProcCheck2_CardTitle.CheckShape(Imgsrc)*/ )
		{
			//flag = RES_OK; //
			SendMessage(WM_MYMSG_CARD, 1, 0); //成功
		}
		else
		{
			if (m_ProcCheck3_CardTri.m_hoShape.IsInitialized())
				m_ProcCheck3_CardTri.m_hoShape.Clear();
			SendMessage(WM_MYMSG_CARD, 0, RES_NG1); //缺失
			flag =  RES_NG2; //卡片缺失
		}

	}

	//检查干燥剂
	if (m_ProcCheck4_Dry.CheckBlob(Imgsrc))
	{
		SendMessage(WM_MYMSG_DRY, 1, 0); //成功
	}
	else
	{
		if (ImProcChecker::g_hoBlobArea.IsInitialized())
			ImProcChecker::g_hoBlobArea.Clear();
		SendMessage(WM_MYMSG_DRY, 0, RES_NG1); //缺失
		flag =  RES_NG3; //干燥剂缺失
	}

	return flag;
}

//-1: 成功
//0: 相机取图失败
//1: 保护模检查缺失
BOOL CChecker5678Dlg::CheckCovery()
{
	//debug
	//return -1;

	BOOL flag = FALSE;
	HObject Imgsrc;
	if (!m_ProcCheck1_Book.GrabImageFromCam(Imgsrc))
	{
		//LOG4CPLUS_DEBUG(m_myLoger->logger, "CheckAccessary 相机错误");
		return RES_CAM;
	}
	ImProcChecker::ContoursAll();
	if (m_ProcCheck5_Covery.CheckGray(Imgsrc))
	{
		
	}
	else
	{
		return RES_NG1;
	}
	return RES_OK;
}

//-1: 成功
//0: 相机取图失败
//1: 没有检测到停止标志
BOOL CChecker5678Dlg::CheckStop()
{
	BOOL flag = FALSE;
	HObject Imgsrc;
	if (!m_ProcCheck1_Book.GrabImageFromCam(Imgsrc))
	{
		//LOG4CPLUS_DEBUG(m_myLoger->logger, "CheckAccessary 相机错误");
		return RES_CAM;
	}
	//ImProcChecker::ContoursAll();

	if (m_ProcCheck7_Stop.CheckShape(Imgsrc))
	{
		if (ImProcChecker::g_hoContoursAll.IsInitialized())
			ImProcChecker::g_hoContoursAll.Clear();
	}
	else
	{
		return RES_NG1;
	}
	return RES_OK;
}

BOOL CChecker5678Dlg::LoadPara(wchar_t * pParaPath)
{
	CString strPath;
	strPath.Format(_T("%s"), pParaPath);

	CFileFind find;
	int result = find.FindFile(strPath);
	if (result == 0)
	{
		WritePrivateProfileString(_T("扫码"), _T("MinLength"), _T("6"), strPath);
		WritePrivateProfileString(_T("称重"), _T("Max"), _T("9999"), strPath);
		WritePrivateProfileString(_T("称重"), _T("Min"), _T("0"), strPath);
		WritePrivateProfileString(_T("称重"), _T("Com"), _T("9"), strPath);
		WritePrivateProfileString(_T("称重"), _T("Board"), _T("9600"), strPath);
		WritePrivateProfileString(_T("相机"), _T("Exp"), _T("60000"), strPath);
		WritePrivateProfileString(_T("数据库"), _T("ConnectStr"), _T("Provider=SQLOLEDB;Server=mfcetn03;Database=VEN;uid=etuser;pwd=mflex123"), strPath);
		WritePrivateProfileString(_T("数据库"), _T("TableName"), _T("ET_Record_Temp"), strPath);
		WritePrivateProfileString(_T("吸盘"), _T("Time"), _T("2000"), strPath);
	}


	//加载检测区域
	CString SName;
	DWORD i = GetPrivateProfileString(_T("称重"), _T("Max"), _T("0"), SName.GetBuffer(128), 128, strPath);
	SName.ReleaseBuffer();
	m_MaxWeight = _ttof(SName);

	i = GetPrivateProfileString(_T("称重"), _T("Min"), _T("0"), SName.GetBuffer(128), 128, strPath);
	SName.ReleaseBuffer();
	m_MinWeight = _ttof(SName);

	i = GetPrivateProfileString(_T("称重"), _T("Com"), _T("0"), SName.GetBuffer(128), 128, strPath);
	SName.ReleaseBuffer();
	com_weight = _ttoi(SName);

	i = GetPrivateProfileString(_T("称重"), _T("Board"), _T("0"), SName.GetBuffer(128), 128, strPath);
	SName.ReleaseBuffer();
	com_boad = _ttoi(SName);

	i = GetPrivateProfileString(_T("相机"), _T("Exp"), _T("60000"), SName.GetBuffer(128), 128, strPath);
	SName.ReleaseBuffer();
	m_fExporTime = _ttof(SName);

	GetPrivateProfileString(_T("数据库"), _T("ConnectStr"), _T("Provider=SQLOLEDB;Server=mfcetn03;Database=VEN;uid=etuser;pwd=mflex123"), SName.GetBuffer(1024), 1024, strPath);
	SName.ReleaseBuffer();
	m_strConnect = (SName);

	GetPrivateProfileString(_T("数据库"), _T("Tablename"), _T("ET_Record_Temp"), SName.GetBuffer(1024), 1024, strPath);
	SName.ReleaseBuffer();
	m_strTablename = (SName);

	i = GetPrivateProfileString(_T("吸盘"), _T("Time"), _T("2000"), SName.GetBuffer(128), 128, strPath);
	SName.ReleaseBuffer();
	m_fTime = _ttof(SName);

	i = GetPrivateProfileString(_T("吸盘"), _T("DelayTime"), _T("800"), SName.GetBuffer(128), 128, strPath);
	SName.ReleaseBuffer();
	m_fDelayTime = _ttof(SName);
	//i = GetPrivateProfileString(_T("扫码"), _T("MinLength"), _T("6"), SName.GetBuffer(128), 128, strPath);
	//SName.ReleaseBuffer();
	//m_MinLength = _ttoi(SName);
	return 0;
}

BOOL CChecker5678Dlg::SavePara(wchar_t * pParaPath)
{
	CString strPath;
	strPath.Format(_T("%s"), pParaPath);

	CString strValue;
	strValue.Format(_T("%f"), m_MaxWeight);
	WritePrivateProfileString(_T("称重"), _T("Max"), strValue, strPath);
	strValue.Format(_T("%f"), m_MinWeight);
	WritePrivateProfileString(_T("称重"), _T("Min"), strValue, strPath);
	strValue.Format(_T("%f"), m_fExporTime);
	WritePrivateProfileString(_T("相机"), _T("Exp"), strValue, strPath);
	strValue.Format(_T("%f"), m_fTime);
	WritePrivateProfileString(_T("吸盘"), _T("Time"), strValue, strPath);
	strValue.Format(_T("%f"), m_fDelayTime);
	WritePrivateProfileString(_T("吸盘"), _T("DelayTime"), strValue, strPath);
	return TRUE;
}

BOOL CChecker5678Dlg::LoadCount(wchar_t * pParaPath)
{
	CString strPath;
	strPath.Format(_T("%s"), pParaPath);

	CFileFind find;
	int result = find.FindFile(strPath);
	if (result == 0)
	{
		WritePrivateProfileString(_T("计数"), _T("TOTAL"), _T("0"), strPath);
		WritePrivateProfileString(_T("计数"), _T("OK"), _T("0"), strPath);
		WritePrivateProfileString(_T("计数"), _T("NG"), _T("0"), strPath);
	}


	//加载检测区域
	CString SName;
	DWORD i = GetPrivateProfileString(_T("计数"), _T("TOTAL"), _T("0"), SName.GetBuffer(128), 128, strPath);
	SName.ReleaseBuffer();
	m_iTotal = _ttoi(SName);

	i = GetPrivateProfileString(_T("计数"), _T("OK"), _T("0"), SName.GetBuffer(128), 128, strPath);
	SName.ReleaseBuffer();
	m_iOkCount = _ttoi(SName);

	i = GetPrivateProfileString(_T("计数"), _T("NG"), _T("0"), SName.GetBuffer(128), 128, strPath);
	SName.ReleaseBuffer();
	m_iNgCount = _ttoi(SName);
	return TRUE;
}

BOOL CChecker5678Dlg::SaveCount(wchar_t * pParaPath)
{
	CString strPath;
	strPath.Format(_T("%s"), pParaPath);

	CString strValue;
	strValue.Format(_T("%d"), m_iTotal);
	WritePrivateProfileString(_T("计数"), _T("TOTAL"), strValue, strPath);
	strValue.Format(_T("%d"), m_iOkCount);
	WritePrivateProfileString(_T("计数"), _T("OK"), strValue, strPath);
	strValue.Format(_T("%d"), m_iNgCount);
	WritePrivateProfileString(_T("计数"), _T("NG"), strValue, strPath);

	return TRUE;
}

bool CChecker5678Dlg::SaveOnlineData(CString strFilePath, TestRes nRes)
{
	if (nRes.nSerialNumber.GetLength() == 0) return true;
	CStdioFile file;
	COleDateTime t = COleDateTime::GetCurrentTime();
	CString str;
	str.Format(_T("\\%04d%02d.csv"), t.GetYear(), t.GetMonth());
	//判断CSV文件是否存在，如果不存在，则下面创建后会新增表头
	char p[1024];
	bool bAddtile = false;
	CString cstrPath = strFilePath + str;
	int len = WideCharToMultiByte(CP_ACP, 0, cstrPath, -1, NULL, 0, NULL, NULL);//宽字符转
	char* strTemp = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, cstrPath, -1, strTemp, len, NULL, NULL);

	if (!IsDirExist(strTemp))
	{
		bAddtile = true;
	}

	delete[]strTemp;

	//创建csv文件
	BOOL ret = file.Open(strFilePath + str, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite);
	if (ret == 0) return false;
	file.SeekToEnd();

	if (bAddtile) //新增表头
	{
		str.Format(_T("TestTime,SerialNumber,Manual,Desiccant,Weight\n"));
		file.WriteString(str);
		file.Flush();
	}
	//写入数据
	str.Format(_T("%s,%s,"), nRes.nTestTime, nRes.nSerialNumber);
	for (int i = 0; i < nRes.vTestResult.size(); i++) {
		str.Format(_T("%s%s,"), str, nRes.vTestResult[i]);
	}
	str += _T("\n");
	file.WriteString(str);
	file.Flush();
	file.Close();
	return true;
}

bool CChecker5678Dlg::SaveOnlineDataSQL(CString strFilePath, TestRes nRes, CString &error)
{
	CStdioFile file;
	CString str, szChar;
	CString buf;
	//CString strFilePath = CSV_OLINEDATA;//_T("./Data");
	int p = nRes.vTestResult.size();
	//p = 0;
	COleDateTime t = COleDateTime::GetCurrentTime();

	//打开数据库 
	AdoDatabase adb;
	adb.m_connectstr = m_strConnect;//"Provider=SQLOLEDB;Server=mfcetn03;Database=VEN;uid=etuser;pwd=mflex123";
	if (0 == adb.ConnectDB())
	{
		//1.先查询有无此条码记录

		//有则更新记录

		//2.如果没有，则插入新记录
		str.Format(_T("insert into  %s(SerialNumber, TestTime, TestResult, TestType, PartNumber, Line, IP_Address, Machine, Operator, OnLine"), m_strTablename);

		CString strsub;
		for (int i = 0; i < p; i++) {
			strsub.Format(_T(",Result%d"), i + 1);
			str += strsub;
		}

		str.Format(_T("%s) values('%s','%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', 'Y'"), str, nRes.nSerialNumber, nRes.nTestTime, nRes.nTestResult,
			nRes.nTestType, nRes.nPartNumber, nRes.nLine, nRes.nIP_Address, nRes.nMachine, nRes.nOperator);

		for (int i = 0; i < p; i++) {
			strsub.Format(_T(",'%s'"), nRes.vTestResult[i]);
			str += strsub;
		}

		str += _T(")");
		if (0 == adb.InsertStrData(str))
		{
			//插入成功
			//m_ListBox.AddString(_T("数据库写入成功"));
		}
		else
		{
			//m_ListBox.AddString(_T("数据库写入失败，已保存在本地CSV文件"));
			error = adb.m_errormessage;
			adb.CloseConn();
			return false;
		}
		adb.CloseConn();
	}
	else
	{
		GetDlgItem(IDC_STATIC_DBCONNECT)->SetWindowText(_T("数据库连接失败"));
		error = adb.m_errormessage;
		//m_ListBox.AddString(_T("数据库连接失败，已保存在本地CSV文件"));
		return false;
	}
	return true;
}


void CChecker5678Dlg::OnBnClickedButtonTest()
{
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_hTestEvent, 10)) //如果停止
	{
		AfxMessageBox(_T("请先使程序停止"));
		return;
	}
	//从文件中加载文件
	HObject img;
	m_ProcMeasureUSB.GrabOneAndShow();
	////if (m_ProcCheck1.GrabImageFromFile(NULL, img))
	//{
	//	//识别
	//	//m_ProcCheck1.CheckShape(img);
	//	//m_ProcCheck2.CheckShape(img);
	//	//m_ProcCheck3.CheckShape(img);
	//	//m_ProcCheck4.CheckBlob(img);
	//}

}


void CChecker5678Dlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//if (WAIT_OBJECT_0 == WaitForSingleObject(m_hTestEvent, 10)) //如果停止
	//{
	//	AfxMessageBox(_T("请先使程序停止，在关闭程序。"));
	//	return;
	//}

	//m_bexit = true;
	//SetEvent(m_hTestEvent);
	//Sleep(500);
	CDialogEx::OnClose();
}

LRESULT CChecker5678Dlg::ShowResultBarcode(WPARAM wParam, LPARAM lParam)
{
	int state = (int)wParam;
	if (0 == state)//读取中
	{
		m_strMsgBarCode = TXT_MSG_READING;
		//同时颜色显示
		m_bCheckBarcode = false;
		
		m_strMsgAccessary = TXT_MSG_CHECKING;
		m_bCheckAccessary = false;

		m_strMsgBook = TXT_MSG_CHECKING;
		m_strMsgCard = TXT_MSG_CHECKING;
		m_strMsgDry = TXT_MSG_CHECKING;
		m_bCheckBook = false;
		m_bCheckCard = false;
		m_bCheckDry = false;

		m_strMsgCovery = TXT_MSG_CHECKING;
		m_bCheckCovery = false;
		MyWriteBit(OUT_LIGHT_COVERY_OK, IO_OUT_OFF);
	}
	else
	{
		string* strData = (string *)lParam;
		//显示读取条形码内容
		m_strMsgBarCode = strData->c_str();
		//同时颜色显示
		m_bCheckBarcode = true;
	}
	UpdateData(FALSE);
	return 0;
}

LRESULT CChecker5678Dlg::ShowResultAccess(WPARAM wParam, LPARAM lParam)
{
	int state = (int)wParam;
	if (1 == state)//读取中
	{
		m_strMsgAccessary = TXT_MSG_OK;
		//同时颜色显示
		m_bCheckAccessary = true;
	}
	else
	{
		int strData = (int)lParam;
		switch (strData)
		{
		case 0:
			m_strMsgAccessary = TXT_MSG_CAM;
			break;
		case 1:
			m_strMsgAccessary = TXT_MSG_BOOK;
			break;
		case 2:
			m_strMsgAccessary = TXT_MSG_CARD;
			break;
		case 3:
			m_strMsgAccessary = TXT_MSG_DRY;
			break;
		default:
			m_strMsgAccessary = TXT_MSG_CHECKING;
			break;
		}
		//同时颜色显示
		m_bCheckAccessary = false;

		m_strMsgCovery = TXT_MSG_CHECKING;
		m_bCheckCovery = false;
	}
	UpdateData(FALSE);
	return 0;
}

LRESULT CChecker5678Dlg::ShowResultCovery(WPARAM wParam, LPARAM lParam)
{
	int state = (int)wParam;
	if (1 == state)//读取中
	{
		m_strMsgCovery = TXT_MSG_OK;
		//同时颜色显示
		m_bCheckCovery = true;
		MyWriteBit(OUT_LIGHT_COVERY_OK, IO_OUT_ON);
	}
	else
	{
		m_strMsgCovery = TXT_MSG_COVERY;
		m_bCheckCovery = false;
	}
	UpdateData(FALSE);
	return 0;
}

LRESULT CChecker5678Dlg::ShowResultBook(WPARAM wParam, LPARAM lParam)
{
	int state = (int)wParam;
	if (1 == state)//读取中
	{
		m_strMsgBook = TXT_MSG_OK;
		//同时颜色显示
		m_bCheckBook = true;
	}
	else
	{
		int strData = (int)lParam;
		switch (strData)
		{
		case 0:
			m_strMsgBook = TXT_MSG_CAM;
			break;
		case 1:
			m_strMsgBook = TXT_MSG_BOOK;
			break;
		default:
			m_strMsgBook = TXT_MSG_CHECKING;
			break;
		}
		//同时颜色显示
		m_bCheckBook = false;

		m_strMsgCovery = TXT_MSG_CHECKING;
		m_bCheckCovery = false;
	}
	UpdateData(FALSE);
	return 0;
}

LRESULT CChecker5678Dlg::ShowResultCard(WPARAM wParam, LPARAM lParam)
{
	int state = (int)wParam;
	if (1 == state)//读取中
	{
		m_strMsgCard = TXT_MSG_OK;
		//同时颜色显示
		m_bCheckCard = true;
	}
	else
	{
		int strData = (int)lParam;
		switch (strData)
		{
		case 0:
			m_strMsgCard = TXT_MSG_CAM;
			break;
		case 1:
			m_strMsgCard = TXT_MSG_CARD;
			break;
		default:
			m_strMsgCard = TXT_MSG_CHECKING;
			break;
		}
		//同时颜色显示
		m_bCheckCard = false;

		m_strMsgCovery = TXT_MSG_CHECKING;
		m_bCheckCovery = false;
	}
	UpdateData(FALSE);
	return 0;
}

LRESULT CChecker5678Dlg::ShowResultDry(WPARAM wParam, LPARAM lParam)
{
	int state = (int)wParam;
	if (1 == state)//读取中
	{
		m_strMsgDry = TXT_MSG_OK;
		//同时颜色显示
		m_bCheckDry = true;
	}
	else
	{
		int strData = (int)lParam;
		switch (strData)
		{
		case 0:
			m_strMsgDry = TXT_MSG_CAM;
			break;
		case 1:
			m_strMsgDry = TXT_MSG_DRY;
			break;
		default:
			m_strMsgDry = TXT_MSG_CHECKING;
			break;
		}
		//同时颜色显示
		m_bCheckDry = false;

		m_strMsgCovery = TXT_MSG_CHECKING;
		m_bCheckCovery = false;
	}
	UpdateData(FALSE);
	return 0;
}

LRESULT CChecker5678Dlg::ShowResultHaevy(WPARAM wParam, LPARAM lParam)
{
	//如果感应器触发，则且
	int state = (int)wParam;
	Sleep(10);
	if (IO_INVALID == MyReadBit(IN_SENS_WEIGHT))
	{
		MyWriteBit(OUT_LIGHT_WEIGHT_OK, IO_OUT_OFF);
		MyWriteBit(OUT_LIGHT_WEIGHT_NG, IO_OUT_OFF);
		return 0;
	}
	else
	{
		
	}

	if (1 == state)//读取中
	{
		MyWriteBit(OUT_LIGHT_WEIGHT_OK, IO_OUT_ON);
		Sleep(10);
		MyWriteBit(OUT_LIGHT_WEIGHT_NG, IO_OUT_OFF);
	}
	else
	{
		MyWriteBit(OUT_LIGHT_WEIGHT_OK, IO_OUT_OFF);
		Sleep(10);
		MyWriteBit(OUT_LIGHT_WEIGHT_NG, IO_OUT_ON);
	}
	return 0;
}

void CChecker5678Dlg::CStaticShow(int ID, CString str, int size) {
	CFont font;
	font.CreatePointFont(size, L"黑体");
	GetDlgItem(ID)->SetWindowText(str);
	GetDlgItem(ID)->SetFont(&font);
	font.Detach();
}

LRESULT CChecker5678Dlg::OnReceiveStr(WPARAM str, LPARAM commInfo)
{
	struct serialPortInfo
	{
		UINT portNr;//串口号
		DWORD bytesRead;//读取的字节数
	}*pCommInfo;
	pCommInfo = (serialPortInfo*)commInfo;
	DWORD ooo = pCommInfo->bytesRead;

	int len = ooo;
	if (len)
	{
		char* Rv_Cstr = (char*)str;
		
		//memcpy(Schar, Rv_Cstr, len);

		//截取
		CString substr[12800];
		int sBufSize = len;// strlen(Rv_Cstr);
		DWORD dBufSize = MultiByteToWideChar(CP_ACP, 0, Rv_Cstr, sBufSize, NULL, 0);
		wchar_t * dBuf = new wchar_t[dBufSize+1];
		wmemset(dBuf, 0, dBufSize);
		//进行转换
		int nRet = MultiByteToWideChar(CP_ACP, 0, Rv_Cstr, sBufSize, dBuf, dBufSize);
		CString Cstr = (CString)dBuf;
		int leng = Cstr.GetLength();
		int index = Cstr.ReverseFind(_T('g'));
		CString SubStr = Cstr;
		delete[]dBuf;
		dBuf = NULL;
		if (index != -1)
		{
			if ((index < leng)&&(index>8))
			{
				SubStr = Cstr.Mid(index - 8, 8);

				int len = WideCharToMultiByte(CP_ACP, 0, SubStr, -1, NULL, 0, NULL, NULL);//宽字符转
				char* strTemp = new char[len + 1];
				WideCharToMultiByte(CP_ACP, 0, SubStr, -1, strTemp, len, NULL, NULL);
				double fweight = atof(strTemp);
				delete[]strTemp;
				//比较,改变颜色
				if (fweight < m_MaxWeight && (fweight > m_MinWeight))
				{
					m_bCheckWeight = true;
					//MyWriteBit(OUT_LIGHT_WEIGHT_OK, IO_OUT_ON);
					SendMessage(WM_MYMSG_HEAVY, 1, 0);
				}
				else
				{
					m_bCheckWeight = false;
					//MyWriteBit(OUT_LIGHT_WEIGHT_OK, IO_OUT_OFF);
					SendMessage(WM_MYMSG_HEAVY, 0, 0);
				}

				//显示
				m_fWeight = fweight;
				UpdateData(FALSE);
			}
		}


		//if (sBufSize >= 16)
		//{
		//	char weight[1024];
		//	memset(weight, '\0', 1024);
		//	void *p = memccpy(weight, Schar + 7, '\r', 8);
		//	double fweight = atof(weight);

		//	
		//	UpdateData(FALSE);
		//}

	}

	return 0;
}

HBRUSH CChecker5678Dlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	//if (pWnd->GetDlgCtrlID() == IDC_EDIT_RESULT3) //条形码
	//{
	//	CString str;
	//	pDC->SetTextColor(RGB(0, 0, 0));     //设置字体颜色	
	//	pDC->SetBkMode(TRANSPARENT); //设置字体背景为透明
	//	if (m_bCheckBarcode)
	//		return m_brPASS; // 设置背景色
	//	else
	//		return m_brFAIL;
	//}
	//else if (pWnd->GetDlgCtrlID() == IDC_EDIT_RESULT1) //配件
	//{
	//	CString str;
	//	pDC->SetTextColor(RGB(0, 0, 0));     //设置字体颜色	
	//	pDC->SetBkMode(TRANSPARENT); //设置字体背景为透明
	//	if (m_bCheckAccessary)
	//		return m_brPASS; // 设置背景色
	//	else
	//		return m_brFAIL;
	//}
	//else if (pWnd->GetDlgCtrlID() == IDC_EDIT_RESULT2) //保护膜
	//{
	//	CString str;
	//	pDC->SetTextColor(RGB(0, 0, 0));     //设置字体颜色	
	//	pDC->SetBkMode(TRANSPARENT); //设置字体背景为透明
	//	if (m_bCheckCovery)
	//		return m_brPASS; // 设置背景色
	//	else
	//		return m_brFAIL;
	//}
	//else if (pWnd->GetDlgCtrlID() == IDC_EDIT_RESULT4) //保护膜
	//{
	//	CString str;
	//	pDC->SetTextColor(RGB(0, 0, 0));     //设置字体颜色	
	//	pDC->SetBkMode(TRANSPARENT); //设置字体背景为透明
	//	if (m_bCheckWeight)
	//		return m_brPASS; // 设置背景色
	//	else
	//		return m_brFAIL;
	//}
	//else if (pWnd->GetDlgCtrlID() == IDC_EDIT_RESULT6) //book
	//{
	//	CString str;
	//	pDC->SetTextColor(RGB(0, 0, 0));     //设置字体颜色	
	//	pDC->SetBkMode(TRANSPARENT); //设置字体背景为透明
	//	if (m_bCheckBook)
	//		return m_brPASS; // 设置背景色
	//	else
	//		return m_brFAIL;
	//}
	//else if (pWnd->GetDlgCtrlID() == IDC_EDIT_RESULT7) //card
	//{
	//	CString str;
	//	pDC->SetTextColor(RGB(0, 0, 0));     //设置字体颜色	
	//	pDC->SetBkMode(TRANSPARENT); //设置字体背景为透明
	//	if (m_bCheckCard)
	//		return m_brPASS; // 设置背景色
	//	else
	//		return m_brFAIL;
	//}
	//else if (pWnd->GetDlgCtrlID() == IDC_EDIT_RESULT8) //dry
	//{
	//	CString str;
	//	pDC->SetTextColor(RGB(0, 0, 0));     //设置字体颜色	
	//	pDC->SetBkMode(TRANSPARENT); //设置字体背景为透明
	//	if (m_bCheckDry)
	//		return m_brPASS; // 设置背景色
	//	else
	//		return m_brFAIL;
	//}
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}


void CChecker5678Dlg::OnBnClickedCheck1()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	m_bToCheckBarcode2 = m_bToCheckBarcode;
}


void CChecker5678Dlg::OnBnClickedCheck2()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	m_toCheckAcc2 = m_toCheckAcc;
}


void CChecker5678Dlg::OnBnClickedCheck3()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	m_bToCheckCovery2 = m_bToCheckCovery;
}


void CChecker5678Dlg::OnBnClickedButtonStart2()
{
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_hTestEvent, 10)) //如果停止
	{
		AfxMessageBox(_T("请先使程序停止"));
		return;
	}
	if (2 == theApp.m_Admin)
		return;
	//LOG4CPLUS_DEBUG(m_myLoger->logger, "设置===================");
	//打开光源
	MyWriteBit(OUT_LIGHT_CAM, IO_OUT_ON);
	
	SetParDlg dlg;
	dlg.m_pCam = m_pCam;
	dlg.m_pMainDlg = this;
	//赋值
	dlg.m_iMinLength = (int)m_ProcCheck6_Code.m_Para.paraN[0];

	dlg.m_lfMinScore_book = (double)m_ProcCheck1_Book.m_Para.paraN[0];

	dlg.m_lfMinScore_card = (double)m_ProcCheck3_CardTri.m_Para.paraN[0];

	dlg.m_iMinGray = (int)m_ProcCheck4_Dry.m_Para.paraN[0];
	dlg.m_iMaxGray =(int)m_ProcCheck4_Dry.m_Para.paraN[1];
	dlg.m_iClosingSize = (int)m_ProcCheck4_Dry.m_Para.paraN[2];
	dlg.m_iMinArea = (int)m_ProcCheck4_Dry.m_Para.paraN[3];
	dlg.m_iMaxArea = (int)m_ProcCheck4_Dry.m_Para.paraN[4];
	dlg.m_lfMinRect = (double)m_ProcCheck4_Dry.m_Para.paraN[5];

	dlg.m_lfMinWeight = m_MinWeight;
	dlg.m_lfMaxWeight = m_MaxWeight;

	dlg.m_lfExpo = m_fExporTime;

	dlg.m_lfTime = m_fTime;
	dlg.m_fDelayTime = m_fDelayTime;

	dlg.m_lfMinGray_Cover = m_ProcCheck5_Covery.m_Para.paraN[0];
	if (IDOK == dlg.DoModal())
	{
		MyWriteBit(OUT_LIGHT_CAM, IO_OUT_OFF);
		//保存参数
		m_ProcCheck6_Code.m_Para.paraN[0] = (double)dlg.m_iMinLength;

		m_ProcCheck1_Book.m_Para.paraN[0] = (double)dlg.m_lfMinScore_book;

		//m_ProcCheck2_CardTitle.m_Para.paraN[0] = (double)dlg.m_lfMinScore_card;
		m_ProcCheck3_CardTri.m_Para.paraN[0] = (double)dlg.m_lfMinScore_card;

		m_ProcCheck4_Dry.m_Para.paraN[0] = (double)dlg.m_iMinGray;
		m_ProcCheck4_Dry.m_Para.paraN[1] = (double)dlg.m_iMaxGray;
		m_ProcCheck4_Dry.m_Para.paraN[2] = (double)dlg.m_iClosingSize;
		m_ProcCheck4_Dry.m_Para.paraN[3] = (double)dlg.m_iMinArea;
		m_ProcCheck4_Dry.m_Para.paraN[4] = (double)dlg.m_iMaxArea;
		m_ProcCheck4_Dry.m_Para.paraN[5] = (double)dlg.m_lfMinRect;

		m_ProcCheck5_Covery.m_Para.paraN[0] = dlg.m_lfMinGray_Cover;

		m_MinWeight = dlg.m_lfMinWeight;
		m_MaxWeight = dlg.m_lfMaxWeight;

		m_fExporTime = dlg.m_lfExpo;

		m_fTime = dlg.m_lfTime ;
		m_fDelayTime = m_fDelayTime;
		SavePara(PATH_PARA_SYS);
		m_ProcCheck1_Book.SavePara(PATH_PARA_LOGO); //说明书
		//m_ProcCheck2_CardTitle.SavePara(PATH_PARA_CARDTITLE); //卡片标题
		m_ProcCheck3_CardTri.SavePara(PATH_PARA_CARDTRI); //卡片三角
		m_ProcCheck4_Dry.SavePara(PATH_PARA_DRY); //干燥剂
		m_ProcCheck5_Covery.SavePara(PATH_PARA_COVER); //保护膜
		m_ProcCheck6_Code.SavePara(PATH_PARA_CODE);
		m_ProcCheck7_Stop.SavePara(PATH_PARA_STOP);

		//LOG4CPLUS_DEBUG(m_myLoger->logger, "设置 退出==============");
		CRect rect;
		m_show.GetClientRect(&rect);
		//m_mydll.SetHalconDispWnd(m_show.GetSafeHwnd(), rect);
		ImProcChecker::SetDispWnd((Hlong)m_show.GetSafeHwnd(), rect);

		//LOG4CPLUS_DEBUG(m_myLoger->logger, "重新加载参数===========");
		m_ProcCheck1_Book.LoadPara(PATH_PARA_LOGO); //说明书
		//m_ProcCheck2_CardTitle.LoadPara(PATH_PARA_CARDTITLE); //卡片标题
		m_ProcCheck3_CardTri.LoadPara(PATH_PARA_CARDTRI); //卡片三角
		m_ProcCheck4_Dry.LoadPara(PATH_PARA_DRY); //干燥剂
		m_ProcCheck5_Covery.LoadPara(PATH_PARA_COVER); //保护膜
		m_ProcCheck6_Code.LoadPara(PATH_PARA_CODE);
		m_ProcCheck7_Stop.LoadPara(PATH_PARA_STOP);

		//加载模板
		//m_ProcCheck1_Book.LoadModel(PATH_MODEL1); //说明书
		//m_ProcCheck1_Book.LoadDeformModel(PATH_MODEL_LOGO_DFM);
		////m_ProcCheck2_CardTitle.LoadModel(PATH_MODEL2); //卡片标题
		//m_ProcCheck3_CardTri.LoadModel(PATH_MODEL3); //卡片三角
		//m_ProcCheck7_Stop.LoadModel(PATH_MODEL7);
		if (!m_ProcCheck1_Book.LoadModel(PATH_MODEL1)) //说明书
		{
			AfxMessageBox(_T("加载说明书LOGO模板失败，请检查logo.shm文件。"));
		}
		//m_ProcCheck2_CardTitle.LoadModel(PATH_MODEL2); //卡片标题
		if (!m_ProcCheck3_CardTri.LoadModel(PATH_MODEL3)) //卡片三角
		{
			AfxMessageBox(_T("加载卡片图案模板失败，请检查cardtri.shm文件。"));
		}
		if (!m_ProcCheck7_Stop.LoadModel(PATH_MODEL7))
		{
			AfxMessageBox(_T("加载模具图案模板失败，请检查stop.shm文件。"));
		}
		if (!m_ProcCheck1_Book.LoadDeformModel(PATH_MODEL_LOGO_DFM))//加载logo的可形变模板
		{
			AfxMessageBox(_T("加载说明书LOGO模板失败，请检查logo.dfm文件。"));
		}
		//LOG4CPLUS_DEBUG(m_myLoger->logger, "重新加载参数 完成======");
		return;
	}
	MyWriteBit(OUT_LIGHT_CAM, IO_OUT_OFF);
	CRect rect;
	m_show.GetClientRect(&rect);
	//m_mydll.SetHalconDispWnd(m_show.GetSafeHwnd(), rect);
	ImProcChecker::SetDispWnd((Hlong)m_show.GetSafeHwnd(), rect);
	return;
}

short CChecker5678Dlg::MyWriteBit(int bitno, int on_off)
{
	if (!m_handle) return -1;
	g_criSectionIO.Lock();  //锁
	short rtdat = NIO_SetDOBit(m_handle, bitno, on_off);
	if (RTN_CMD_SUCCESS == rtdat)
	{
		g_criSectionIO.Unlock();
		return 0;
	}
	else if (RTN_CMD_ERROR == rtdat)
	{
		g_criSectionIO.Unlock();
		return -1;
	}
	else
	{
		g_criSectionIO.Unlock();
		return -1;
	}
}

short CChecker5678Dlg::MyReadBit(int bitno)
{
	if (!m_handle) return -1;
	g_criSectionIO.Lock();  //锁
	U16 out_val = 0;
	short rtdat = NIO_GetDIBit(m_handle, bitno, &out_val);
	if (RTN_CMD_SUCCESS == rtdat)
	{
		if (out_val == 0)
		{
			g_criSectionIO.Unlock();  //锁
			return IO_VALID; //无效电平
		}
		else
		{
			g_criSectionIO.Unlock();
			return IO_INVALID;   //有效电平
		}

	}
	else if (RTN_CMD_ERROR == rtdat)
	{
		g_criSectionIO.Unlock();
		return -1;
	}
	else
	{
		g_criSectionIO.Unlock();
		return -1;
	}
	g_criSectionIO.Unlock();
	return -1;
}
BOOL CChecker5678Dlg::InitMotion() {

	//使用高川IO
	m_handle = NULL;
	unsigned short device_num = 0;
	TDevInfo devinfo_list[16];
	short rtdat = NIO_Search(&device_num, &devinfo_list[0]);
	if (RTN_CMD_SUCCESS == rtdat)//* 扫描系统中的SC设备
	{
		if (device_num > 0)
		{
			char str_id_name[256];
			sprintf_s(str_id_name, "%s", devinfo_list[0].idStr);
			rtdat = NIO_OpenByID(str_id_name, &m_handle);
			if (RTN_CMD_SUCCESS == rtdat)//* 根据SC设备名称打开设备
			{
				if (m_handle != NULL)
				{
					return TRUE;
				}
				else
				{
					AfxMessageBox(_T("初始化控制卡失败！"));
					return FALSE;
				}
			}
			else if (RTN_CMD_ERROR == rtdat)
			{
				AfxMessageBox(_T("初始化控制卡失败！"));
				return FALSE;
			}
			else
			{
				AfxMessageBox(_T("初始化控制卡失败！"));
				return FALSE;
			}
		}
		else
		{
			return FALSE;
		}
	}
	else if (RTN_CMD_ERROR == rtdat)
	{
		return FALSE;
	}
	else
	{
		return FALSE;
	}
	return TRUE;

}

BOOL CChecker5678Dlg::InitIO()
{
	//关灯
	for(int i=0; i<32; i++)
		MyWriteBit(i, IO_OUT_OFF);

	return 0;
}

BOOL CChecker5678Dlg::showContour()
{
	SetColor(ImProcChecker::WindowHandle, "green");
	if (ImProcChecker::g_hoBarCodeArea.IsInitialized())
		DispObj(ImProcChecker::g_hoBarCodeArea, ImProcChecker::WindowHandle);
	if (ImProcChecker::g_hoBlobArea.IsInitialized())
		DispObj(ImProcChecker::g_hoBlobArea, ImProcChecker::WindowHandle);
	if (m_ProcCheck1_Book.m_hoShape.IsInitialized())
		DispObj(m_ProcCheck1_Book.m_hoShape, ImProcChecker::WindowHandle);
	if (m_ProcCheck1_Book.m_hoDeformShape.IsInitialized())
		DispObj(m_ProcCheck1_Book.m_hoDeformShape, ImProcChecker::WindowHandle);

	//if (m_ProcCheck2_CardTitle.m_hoShape.IsInitialized())
	//	DispObj(m_ProcCheck2_CardTitle.m_hoShape, ImProcChecker::WindowHandle);
	if (m_ProcCheck3_CardTri.m_hoShape.IsInitialized())
		DispObj(m_ProcCheck3_CardTri.m_hoShape, ImProcChecker::WindowHandle);
	return 0;
}

void CChecker5678Dlg::OnBnClickedButtonStart3()
{
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_hTestEvent, 10)) //如果停止
	{
		AfxMessageBox(_T("请先使程序停止"));
		return;
	}
	if (2 == theApp.m_Admin)
		return;
	// TODO: 在此添加控件通知处理程序代码
	theApp.menageuser();
	//
	theApp.login();
	if (2 == theApp.m_Admin)
	{
		//操作者禁用设置
		GetDlgItem(IDC_BUTTON_START2)->EnableWindow(FALSE);
		//操作者禁用
		//GetDlgItem(IDC_BUTTON_START3)->EnableWindow(FALSE);
	}
	else
	{
		//操作者禁用设置
		GetDlgItem(IDC_BUTTON_START2)->EnableWindow(TRUE);
		//操作者禁用设置
		//GetDlgItem(IDC_BUTTON_START3)->EnableWindow(TRUE);
	}
}


void CChecker5678Dlg::OnBnClickedButton2()
{
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_hTestEvent, 10)) //如果停止
	{
		AfxMessageBox(_T("请先使程序停止"));
		return;
	}
	// TODO: 在此添加控件通知处理程序代码
	MyWriteBit(OUT_LIGHT_CAM, IO_OUT_OFF);
}


void CChecker5678Dlg::OnBnClickedButton3()
{
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_hTestEvent, 10)) //如果停止
	{
		AfxMessageBox(_T("请先使程序停止"));
		return;
	}
	// TODO: 在此添加控件通知处理程序代码
	MyWriteBit(OUT_LIGHT_CAM, IO_OUT_ON);
}

void CChecker5678Dlg::MenageSave()
{
}

void CChecker5678Dlg::ShowMsg(LPCTSTR msg)
{
	//if (!m_btoShowMsg) return;
	//else
	//{
	//	m_btoShowMsg = false;
	//}
	COleDateTime t;
	CString strtime;
	t = COleDateTime::GetCurrentTime();
	strtime.Format(_T("%04d%02d%02d %02d:%02d:%02d: "), t.GetYear(), t.GetMonth(), t.GetDay(), t.GetHour(), t.GetMinute(), t.GetSecond());
	m_listMsg.InsertString(m_listMsg.GetCount(), strtime + msg);
}


void CChecker5678Dlg::OnBnClickedButtonTest2()
{
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_hTestEvent, 10)) //如果停止
	{
		AfxMessageBox(_T("请先使程序停止"));
		return;
	}
	//从文件中加载文件
	HObject img;
	m_ProcMeasureUSB.GrabImageFromFile(0, img);
}


void CChecker5678Dlg::OnBnClickedButtonDrawarea()
{
	if (!m_ProcMeasureUSB.Show())
	{
		AfxMessageBox(_T("显示失败"));
		return;
	}
	//框选检测区域
	if (IDOK != AfxMessageBox(_T("请框选检测目标所在区域，右键结束框选。"), MB_OKCANCEL))
	{
		return;
	}
	m_brect = false;
	DetectArea rect;
	DetectArea rect_refer;
	if (!m_ProcMeasureUSB.DrawRect(rect))
	{
	}

	//框选参考区域
	if (IDOK != AfxMessageBox(_T("请框选一个参考区域"), MB_OKCANCEL))
	{
		return;
	}
	if (!m_ProcMeasureUSB.DrawRect(rect_refer))
	{
		m_brect = false;
		return;
	}
	m_brect = true;

	m_ProcMeasureUSB.m_Para.decArea = rect;
	m_ProcMeasureUSB.m_Para.refArea = rect_refer;
	//保存
	if (m_ProcMeasureUSB.SavePara(PATH_PARA_MEASURE_USB))
		AfxMessageBox(_T("保存成功"));
	else
		AfxMessageBox(_T("保存失败"));

	m_ProcMeasureUSB.LoadPara(PATH_PARA_MEASURE_USB);
}


void CChecker5678Dlg::OnBnClickedButtonDrawarea2()
{
	if (!m_ProcMeasureCircle.Show())
	{
		AfxMessageBox(_T("显示失败"));
		return;
	}
	//框选检测区域
	if (IDOK != AfxMessageBox(_T("请框选检测目标所在区域，右键结束框选。"), MB_OKCANCEL))
	{
		return;
	}
	m_brect = false;
	DetectArea rect;
	DetectArea rect_refer;
	if (!m_ProcMeasureCircle.DrawRect(rect))
	{
	}

	//框选参考区域
	if (IDOK != AfxMessageBox(_T("请框选一个参考区域"), MB_OKCANCEL))
	{
		return;
	}
	if (!m_ProcMeasureCircle.DrawRect(rect_refer))
	{
		m_brect = false;
		return;
	}
	m_brect = true;

	m_ProcMeasureCircle.m_Para.decArea = rect;
	m_ProcMeasureCircle.m_Para.refArea = rect_refer;
	//保存
	if (m_ProcMeasureCircle.SavePara(PATH_PARA_CIRCLE))
		AfxMessageBox(_T("保存成功"));
	else
		AfxMessageBox(_T("保存失败"));

	m_ProcMeasureCircle.LoadPara(PATH_PARA_CIRCLE);
}


void CChecker5678Dlg::OnBnClickedButtonStart4()
{
	m_listMsg.ResetContent();
	UpdateData();
	if (!m_bCurrent)
	{
		if (!m_ProcMeasureCircle.GrabOneAndShow())
		{
			ShowMsg(_T("相机采集失败."));
			AfxMessageBox(_T("相机采集失败."));
			return;
		}
	}

	if (m_ProcMeasureCircle.MeasureCircle() != 0)
	{
		MeasureResult res;
		m_ProcMeasureCircle.GetReult(res);
		CString str;
		str.Format(_T("识别失败， 返回码： %d"), res.errorCode);
		ShowMsg(str);
	}
	else
	{
		MeasureResult res;
		m_ProcMeasureCircle.GetReult(res);
		CString str;
		str.Format(_T("length： %.4f"), res.length);
		ShowMsg(str);
		str.Format(_T("cent_x： %.4f"), res.cent_x);
		ShowMsg(str);
		str.Format(_T("cent_y： %.4f"), res.cent_y);
		ShowMsg(str);

	}
	return;
}


void CChecker5678Dlg::OnBnClickedButtonTest3()
{
	m_ProcMeasureUSB.SaveImage("");
}

//setting para
void CChecker5678Dlg::OnBnClickedButton5()
{
	SettingParaDlg dlg;
	dlg.para1 = m_ProcMeasureUSB.m_Para.paraN[0];
	dlg.para2 = m_ProcMeasureUSB.m_Para.paraN[1];
	dlg.para3 = m_ProcMeasureUSB.m_Para.paraN[2];
	dlg.para4 = m_ProcMeasureUSB.m_Para.paraN[3];
	dlg.para5 = m_ProcMeasureUSB.m_Para.paraN[4];
	dlg.para6 = m_ProcMeasureUSB.m_Para.paraN[5];
	dlg.para7 = m_ProcMeasureUSB.m_Para.paraN[6];
	if (dlg.DoModal() == IDOK)
	{
		m_ProcMeasureUSB.m_Para.paraN[0] = dlg.para1;
		m_ProcMeasureUSB.m_Para.paraN[1] = dlg.para2;
		m_ProcMeasureUSB.m_Para.paraN[2] = dlg.para3;
		m_ProcMeasureUSB.m_Para.paraN[3] = dlg.para4;
		m_ProcMeasureUSB.m_Para.paraN[4] = dlg.para5;
		m_ProcMeasureUSB.m_Para.paraN[5] = dlg.para6;
		m_ProcMeasureUSB.m_Para.paraN[6] = dlg.para7;
		// TODO: 在此添加控件通知处理程序代码
		if (m_ProcMeasureUSB.SavePara(PATH_PARA_MEASURE_USB))
			AfxMessageBox(_T("保存成功"));
		else
			AfxMessageBox(_T("保存失败"));
	}

}

//seting para
void CChecker5678Dlg::OnBnClickedButton6()
{
	SettingParaDlg dlg;
	dlg.para1 = m_ProcMeasureCircle.m_Para.paraN[0];
	dlg.para2 = m_ProcMeasureCircle.m_Para.paraN[1];
	dlg.para3 = m_ProcMeasureCircle.m_Para.paraN[2];
	dlg.para4 = m_ProcMeasureCircle.m_Para.paraN[3];
	if (dlg.DoModal() == IDOK)
	{
		m_ProcMeasureCircle.m_Para.paraN[0] = dlg.para1;
		m_ProcMeasureCircle.m_Para.paraN[1] = dlg.para2;
		m_ProcMeasureCircle.m_Para.paraN[2] = dlg.para3;
		m_ProcMeasureCircle.m_Para.paraN[3] = dlg.para4;
		// TODO: 在此添加控件通知处理程序代码
		if (m_ProcMeasureCircle.SavePara(PATH_PARA_MEASURE_USB))
			AfxMessageBox(_T("保存成功"));
		else
			AfxMessageBox(_T("保存失败"));
	}
}
