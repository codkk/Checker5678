
// Checker5678Dlg.cpp : ʵ���ļ�
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
#define WM_MYMSG_BARCODE WM_USER+100   //ˢ����ʾ������
#define WM_MYMSG_ACCESSARY WM_USER+101   //ˢ����ʾ���������
#define WM_MYMSG_COVERY WM_USER+102   //ˢ����ʾ��Ᵽ��Ĥ���
#define WM_MYMSG_HEAVY WM_USER+103    //ˢ����ʾ����������
#define WM_MYMSG_BOOK WM_USER+105
#define WM_MYMSG_CARD WM_USER+106
#define WM_MYMSG_DRY WM_USER+107

#define WM_THREAD_MESSAGES WM_USER+104
//��ɫ
#define BG_COLOR_GREEN RGB(0, 255, 0)
#define BG_COLOR_RED RGB(255, 0, 0)

//����ֵ
#define RES_OK -1  //�ɹ�
#define RES_CAM 0  //����ɼ�����
#define RES_NG1  1
#define RES_NG2  2
#define RES_NG3  3

#define PATH_DATA "D:\\Data"
#define PATH_PARA_SYS _T("Task\\system.ini")
#define PATH_COUNT _T("Task\\count.ini")
#define TXT_MSG_READING "���ڶ�ȡ��"
#define TXT_MSG_CHECKING "Checking"
#define TXT_MSG_OK "OK"
#define TXT_MSG_CAM "���ȡͼʧ��"
#define TXT_MSG_BOOK "NG"
#define TXT_MSG_CARD "NG"
#define TXT_MSG_DRY "NG"
#define TXT_MSG_COVERY "NG"

CCriticalSection g_criSectionIO;  //��
int CChecker5678Dlg::g_num = 0;
//char Schar[65536];
int m_np = 0;
//�����߳�
DWORD WINAPI AutoTestHandleThread(LPVOID lpParam)
{
	CChecker5678Dlg* pMdlg = (CChecker5678Dlg*)lpParam;
	int ret = pMdlg->MainProcess();
	return ret;
}

//�����̣߳����ڣ�
DWORD WINAPI AutoWeightHandleThread(LPVOID lpParam)
{
	CChecker5678Dlg* pMdlg = (CChecker5678Dlg*)lpParam;
	int ret = pMdlg->Weight();
	return ret;
}

//�����̣߳�IO�ڣ�
DWORD WINAPI AutoSuckerHandleThread(LPVOID lpParam)
{
	CChecker5678Dlg* pMdlg = (CChecker5678Dlg*)lpParam;
	int ret = pMdlg->Sucker();
	return ret;
}

// �ж��ļ����Ƿ����
bool IsDirExist(const char *pszDir)
{
	if (pszDir == NULL)
		return false;

	return (_access(pszDir, 0) == 0);	// io.h
}

// ����Ŀ¼
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
	m_iIndex = 0;		 //������
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
	//�ͷ����
	if (m_pCam)
	{
		m_pCam->StopGrabbing();
		m_pCam->CloseDevice();
		m_pCam->DestroyDevice();
		delete m_pCam;
		m_pCam = NULL;
	}
	//�ͷ�IO���ư忨��Դ
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


// CChecker5678Dlg ��Ϣ�������

BOOL CChecker5678Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��
	//::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BITMAP1));
	//m_btn_draw1.AutoLoad(IDC_BUTTON_START, this);
	//HBITMAP   hBitmap;
	//hBitmap = LoadBitmap(AfxGetInstanceHandle(),
	//	MAKEINTRESOURCE(IDB_BITMAP1));
	////((CButton *)GetDlgItem(IDC_BUTTON_DRAWAREA))->SetBitmap(hBitmap);
	//m_btn_draw1.LoadBitmaps(IDB_BITMAP1);
	//m_btn_draw1.SizeToContent();
	//m_btn_draw1.LoadBitmaps(IDB_BITMAP1);
	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	//����
	//CStaticShow(IDC_EDIT_RESULT3, _T(TXT_MSG_READING), 200);
	//CStaticShow(IDC_EDIT_RESULT1, _T(TXT_MSG_CHECKING), 200);
	//CStaticShow(IDC_EDIT_RESULT2, _T(TXT_MSG_CHECKING), 200);
	//CStaticShow(IDC_EDIT_RESULT6, _T(TXT_MSG_CHECKING), 200);
	//CStaticShow(IDC_EDIT_RESULT7, _T(TXT_MSG_CHECKING), 200);
	//CStaticShow(IDC_EDIT_RESULT8, _T(TXT_MSG_CHECKING), 200);
	//CStaticShow(IDC_EDIT_RESULT4, _T("0.0"), 400);
	//����
	CRect rect;
	m_show.GetClientRect(&rect);
	//m_mydll.SetHalconDispWnd(m_show.GetSafeHwnd(), rect);
	ImProcChecker::SetDispWnd((Hlong)m_show.GetSafeHwnd(), rect);
	//�����ļ�����λ��
	if (!IsDirExist(PATH_DATA))
	{
		if (!CreateMyDirectory(PATH_DATA))
		{
			AfxMessageBox(_T("����DATA�ļ���ʧ��"));
		}
	}
	//��ʱ�䣨�죩���������ļ�
	//�������
	LoadPara(PATH_PARA_SYS);
	m_iIndex = g_num++;
	m_pCam = NULL;
	m_pCam = new myCameraClass(m_iIndex);
	if (MV_OK == m_pCam->CameraInit(NULL/*GetDlgItem(IDC_STATIC_SHOW)*/))  //������ʾ���ھ��
	{
		m_pCam->SetExposureTime(m_fExporTime);
		m_pCam->SetAcquisitionMode(MV_ACQ_MODE_CONTINUOUS);
		//m_pCam->RegisterImageCallBack(ImageCallBackEx, this);
		m_pCam->SetTriggerMode(MV_TRIGGER_MODE_OFF);
		//m_pCam->SetTriggerSource(MV_TRIGGER_SOURCE_SOFTWARE);
		m_pCam->StartGrabbing();  //��ʼ��Ƶ��
		m_bCamConneted = true;
	}
	else
	{
		m_bCurrent = TRUE;
		m_bCamConneted = false;
		AfxMessageBox(_T("�������ʧ��."));
	}
	
	//��������㷨
	ImProcChecker::SetCam(m_pCam);

	//
	//LoadCount(PATH_COUNT);

	//���ӳ��ؼƴ���

	m_ProcMeasureUSB.LoadPara(PATH_PARA_MEASURE_USB);
	m_ProcMeasureCircle.LoadPara(PATH_PARA_CIRCLE);
	//��������
	//m_ProcCheck1_Book.SetType(1);
	////m_ProcCheck2_CardTitle.SetType(2);
	//m_ProcCheck3_CardTri.SetType(3);
	//m_ProcCheck4_Dry.SetType(4);
	//m_ProcCheck5_Covery.SetType(5);
	//m_ProcCheck6_Code.SetType(6);
	//m_ProcCheck7_Stop.SetType(7);
	//if (!m_ProcCheck1_Book.LoadModel(PATH_MODEL1)) //˵����
	//{
	//	AfxMessageBox(_T("����˵����LOGOģ��ʧ�ܣ�����logo.shm�ļ���"));
	//}
	////m_ProcCheck2_CardTitle.LoadModel(PATH_MODEL2); //��Ƭ����
	//if (!m_ProcCheck3_CardTri.LoadModel(PATH_MODEL3)) //��Ƭ����
	//{
	//	AfxMessageBox(_T("���ؿ�Ƭͼ��ģ��ʧ�ܣ�����cardtri.shm�ļ���"));
	//}
	//if (!m_ProcCheck7_Stop.LoadModel(PATH_MODEL7))
	//{
	//	AfxMessageBox(_T("����ģ��ͼ��ģ��ʧ�ܣ�����stop.shm�ļ���"));
	//}

	//if (!m_ProcCheck1_Book.LoadDeformModel(PATH_MODEL_LOGO_DFM))//����logo�Ŀ��α�ģ��
	//{
	//	AfxMessageBox(_T("����˵����LOGOģ��ʧ�ܣ�����stop.dfm�ļ���"));
	//}

	//m_ProcCheck1_Book.LoadPara(PATH_PARA_LOGO); //˵����
	////m_ProcCheck2_CardTitle.LoadPara(PATH_PARA_CARDTITLE); //��Ƭ����
	//m_ProcCheck3_CardTri.LoadPara(PATH_PARA_CARDTRI); //��Ƭ����
	//m_ProcCheck4_Dry.LoadPara(PATH_PARA_DRY); //�����
	//m_ProcCheck5_Covery.LoadPara(PATH_PARA_COVER); //����Ĥ
	//m_ProcCheck6_Code.LoadPara(PATH_PARA_CODE);
	//m_ProcCheck7_Stop.LoadPara(PATH_PARA_STOP);

	m_bexit = false;
	ResetEvent(m_hTestEvent);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AutoTestHandleThread, this, 0, NULL);
	UpdateData(FALSE);
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CChecker5678Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
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

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CChecker5678Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}




void CChecker5678Dlg::OnBnClickedCancel()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CDialogEx::OnCancel();
}


void CChecker5678Dlg::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//�ж��Ƿ�ֹͣ
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_hTestEvent, 10)) //���ֹͣ
	{
		AfxMessageBox(_T("����ʹ����ֹͣ���ڹرճ���"));
		return;
	}
	//LOG4CPLUS_DEBUG(m_myLoger->logger, "�˳�===================");
	m_bexit = true;
	SetEvent(m_hTestEvent);
	m_SerialPort.ClosePort();
	Sleep(500);
	InitIO();
	//LOG4CPLUS_DEBUG(m_myLoger->logger, "�˳� ���==============");
	CDialogEx::OnOK();
}


BOOL CChecker5678Dlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
	if (pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
		case VK_RETURN: //���λس���
			return TRUE;
		case VK_ESCAPE: //����ESC��
			return TRUE;
		default:
			break;
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

//��ʼ
void CChecker5678Dlg::OnBnClickedButtonStart()
{	
	m_listMsg.ResetContent();
	UpdateData();
	if (!m_bCurrent)
	{
		if (!m_ProcMeasureUSB.GrabOneAndShow())
		{
			ShowMsg(_T("����ɼ�ʧ��."));
			AfxMessageBox(_T("����ɼ�ʧ��."));
			return;
		}
	}


	if (m_ProcMeasureUSB.MeasureUSB() != 0)
	{
		MeasureResult res;
		m_ProcMeasureUSB.GetReult(res);
		CString str;
		str.Format(_T("ʶ��ʧ�ܣ� �����룺 %d"), res.errorCode);
		ShowMsg(str);
	}
	else
	{
		MeasureResult res;
		m_ProcMeasureUSB.GetReult(res);
		CString str;
		str.Format(_T("length�� %.4f"), res.length);
		ShowMsg(str);
		str.Format(_T("width�� %.4f"), res.width);
		ShowMsg(str);
		str.Format(_T("phi�� %.4f"), res.angle);
		ShowMsg(str);
		str.Format(_T("cent_x�� %.4f"), res.cent_x);
		ShowMsg(str);
		str.Format(_T("cent_y�� %.4f"), res.cent_y);
		ShowMsg(str);
		str.Format(_T("dis_rr�� %.4f"), res.dis_rr);
		ShowMsg(str);
		str.Format(_T("up margin�� %.4f"), res.up_margin);
		ShowMsg(str);
		str.Format(_T("down margin�� %.4f"), res.dn_margin);
		ShowMsg(str);

	}
	return;
	//if (!m_bCamConneted)
	//{
	//	AfxMessageBox(_T("������Ӳ��ɹ������ų������������"));
	//	return;
	//}
	CString str;
	GetDlgItemTextW(IDC_BUTTON_START, str);
	if (str == _T("��ʼSTART"))
	{
		//��ʼ������
		m_bCheckBarcode = false;
		m_bCheckAccessary = false;
		m_bCheckBook = false;
		m_bCheckCard = false;
		m_bCheckDry = false;
		m_bCheckCovery = false;
		m_listMsg.ResetContent();
		m_btoShowMsg = true;
		//��ʼ��IO
		MyWriteBit(OUT_LIGHT_CAM, IO_OUT_ON); //�򿪹�Դ
		Sleep(100);
		//���Ϳ�ʼ�ź�
		SetEvent(m_hTestEvent);
		//������ʾ
		SetDlgItemTextW(IDC_BUTTON_START, _T("ֹͣSTOP"));
		//LOG4CPLUS_DEBUG(m_myLoger->logger, "��ʼ===================");
	}
	else
	{
		//����ֹͣ
		ResetEvent(m_hTestEvent);
		//������ʾ
		SetDlgItemTextW(IDC_BUTTON_START, _T("��ʼSTART"));
		//LOG4CPLUS_DEBUG(m_myLoger->logger, "ֹͣ===================");
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
		//�жϵ����ֹͣ
		if (WAIT_TIMEOUT == WaitForSingleObject(m_hTestEvent, 20))
		{
			MyWriteBit(OUT_LIGHT_CAM, IO_OUT_OFF); //�رչ�Դ
			//���״̬
			state = 0;
		}
		//�ȴ���ʼ
		WaitForSingleObject(m_hTestEvent, INFINITE);

		//�˳�
		if (m_bexit) break;

		switch (state)
		{
		case -1://����״ֱ̬����⵽ֹͣ��־
				

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

				//0 ��ȡһά��,��ȡ�ɹ��� ��ת�� ����������
				if (!m_bToCheckBarcode) //�������������룬������������
				{
					//SendMessage(WM_MYMSG_BARCODE, 0, 0);
					state = 1;
					break;
				}

				if (ReadBarCode()!= RES_OK)//��ⲻ��������
				{
					state = 0;
					SendMessage(WM_MYMSG_BARCODE, 0, 0);
					break;
				}
				//������Ϣ��������ʾһά��
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
				ShowMsg(_T(" ��⵽���룺") + m_testres.nSerialNumber);
				state = 1;
				break;
		case 1:
				if (!m_toCheckAcc) //���������������������鱣��Ĥ
				{
					//SendMessage(WM_MYMSG_ACCESSARY, 0, 4);
					state = 2;
					break;
				}
				
				//1. ����Ƿ���˵���飬������� ��Ƭ
				ret = 0;
				ret = CheckAccessary();
				showContour();//��ʾ��������
				if (ret != RES_OK)//ȱʧ
				{
					state = 1;
					SendMessage(WM_MYMSG_ACCESSARY, 0, ret);
					break;
				}
				SendMessage(WM_MYMSG_ACCESSARY, 1, ret); //�ɹ�
				m_bCheckAccessaryOnce = true;
				//m_btoShowMsg = true;
				//����˵����͸������״̬
				m_testres.vTestResult.push_back(_T("OK")); //˵����OK
				m_testres.vTestResult.push_back(_T("OK")); //�����OK
				ShowMsg(_T(" ��⵽ȫ�����OK"));
				state = -1;	//ת������״̬
				SaveOnlineData(_T(PATH_DATA), m_testres);
				//m_btoShowMsg = true;
				ShowMsg(_T(" �ѱ����¼������"));
				//m_btoShowMsg = true;
				ShowMsg(_T(" ������"));
				m_iTotal++;
				SaveCount(PATH_COUNT);

				break;
		case 2://
				if (!m_bToCheckCovery) //�������Ᵽ��Ĥ�����������������
				{
					//SendMessage(WM_MYMSG_COVERY, 0, 0);
					state = 0;
					break;
				}
				
				//2. ��鱣��Ĥ
				ret = CheckCovery();
				showContour();//��ʾ��������
				if (ret != RES_OK)
				{
					SendMessage(WM_MYMSG_COVERY, 0, 0);
					state = 2;
					break;
				}
				else//��⵽����Ĥ
				{
					//m_btoShowMsg = true;
					ShowMsg(_T(" ��⵽��Ĥ"));
					state = -1;	//ת������״̬
					SendMessage(WM_MYMSG_COVERY, 1, 0);
					m_bCheckCoveryOnce = true;
					SaveOnlineData(_T(PATH_DATA), m_testres);
					//m_btoShowMsg = true;
					ShowMsg(_T(" �ѱ����¼������"));
					//m_btoShowMsg = true;
					ShowMsg(_T(" ������"));
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
		if (CheckStop() == RES_OK)//���ֹͣ�ź�
		{
			state = 0;
			//��������
			
			//���������Ϣ
			SendMessage(WM_MYMSG_BARCODE, 0, 0);
			m_testres.vTestResult.clear();
			m_testres = TestRes();
			m_listMsg.ResetContent();
			ShowMsg(_T(" �ȴ���Ʒ.."));
		}
		else
		{
			showContour();//��ʾ��������
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
		//�жϵ����ֹͣ
		if (WAIT_TIMEOUT == WaitForSingleObject(m_hTestEvent, 20))
		{
			MyWriteBit(OUT_LIGHT_CAM, IO_OUT_OFF); //�رչ�Դ
												   //���״̬
			state = 0;
		}
		//�ȴ���ʼ
		WaitForSingleObject(m_hTestEvent, INFINITE);

		//�˳�
		if (m_bexit) break;

		//��鰴ť�Ƿ���
		if (MyReadBit(IN_BUTTON_CCD) == IO_VALID)
		{

		}

		switch (state)
		{
		case -1://����״ֱ̬����⵽ֹͣ��־


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

			//0 ��ȡһά��,��ȡ�ɹ��� ��ת�� ����������
			if (!m_bToCheckBarcode) //�������������룬������������
			{
				//SendMessage(WM_MYMSG_BARCODE, 0, 0);
				state = 1;
				break;
			}

			if (ReadBarCode() != RES_OK)//��ⲻ��������
			{
				state = 0;
				SendMessage(WM_MYMSG_BARCODE, 0, 0);
				break;
			}
			//������Ϣ��������ʾһά��
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
			ShowMsg(_T(" ��⵽���룺") + m_testres.nSerialNumber);
			state = 1;
			break;
		case 1:
			if (!m_toCheckAcc) //���������������������鱣��Ĥ
			{
				//SendMessage(WM_MYMSG_ACCESSARY, 0, 4);
				state = 2;
				break;
			}

			//1. ����Ƿ���˵���飬������� ��Ƭ
			ret = 0;
			ret = CheckAccessary();
			showContour();//��ʾ��������
			if (ret != RES_OK)//ȱʧ
			{
				state = 1;
				SendMessage(WM_MYMSG_ACCESSARY, 0, ret);
				break;
			}
			SendMessage(WM_MYMSG_ACCESSARY, 1, ret); //�ɹ�
			state = 2;
			m_bCheckAccessaryOnce = true;
			//m_btoShowMsg = true;
			ShowMsg(_T(" ��⵽ȫ�����OK,��ű�Ĥ"));
			break;
		case 2:
			if (!m_bToCheckCovery) //�������Ᵽ��Ĥ�����������������
			{
				//SendMessage(WM_MYMSG_COVERY, 0, 0);
				state = 0;
				break;
			}

			//2. ��鱣��Ĥ
			ret = CheckCovery();
			showContour();//��ʾ��������
			if (ret != RES_OK)
			{
				SendMessage(WM_MYMSG_COVERY, 0, 0);
				state = 2;
				break;
			}
			else//��⵽����Ĥ
			{
				//m_btoShowMsg = true;
				ShowMsg(_T(" ��⵽��Ĥ"));
				state = -1;	//ת������״̬
				SendMessage(WM_MYMSG_COVERY, 1, 0);
				m_bCheckCoveryOnce = true;
				SaveOnlineData(_T(PATH_DATA), m_testres);
				//m_btoShowMsg = true;
				ShowMsg(_T(" �ѱ��汣���¼������"));
				//m_btoShowMsg = true;
				ShowMsg(_T(" ������"));
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
		if (CheckStop() == RES_OK)//���ֹͣ�ź�
		{
			state = 0;
			//��������

			//���������Ϣ
			SendMessage(WM_MYMSG_BARCODE, 0, 0);
			m_testres.vTestResult.clear();
			m_testres = TestRes();
			m_listMsg.ResetContent();
			ShowMsg(_T(" �ȴ���Ʒ.."));
		}
		else
		{
			showContour();//��ʾ��������
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
		//�жϵ����ֹͣ
		if (WAIT_TIMEOUT == WaitForSingleObject(m_hTestEvent, 20))
		{
			MyWriteBit(OUT_LIGHT_CAM, IO_OUT_OFF); //�رչ�Դ
												   //���״̬
			state = 0;
		}
		//�ȴ���ʼ
		WaitForSingleObject(m_hTestEvent, INFINITE);

		//�˳�
		if (m_bexit) break;
	}
	return 0;
}

int CChecker5678Dlg::Weight()
{
	m_bWeight = false;
	while (1)
	{
		//�˳�
		if (m_bexit) break;

		//���ظ�Ӧ�ź�
		if (MyReadBit(IN_SENS_WEIGHT) == IO_VALID)
		{
			m_bWeight = true;
		}
		else
		{
			m_bWeight = false;
		}

		//������Ϣ
		Sleep(200);
	}
	return 0;
}

int CChecker5678Dlg::Sucker()
{
	m_bSucker = true;
	while (1)
	{
		//�˳�
		if (m_bexit) break;
		if(MyReadBit(IN_SENS_SUKER) == IO_VALID)//�����⵽��ס�źţ�����������
		{
			Sleep(m_fDelayTime); //����200�����ټ��
			if (MyReadBit(IN_SENS_SUKER) == IO_VALID)//������źţ���
			{
				if (m_bSucker)
				{
					m_bSucker = false;
					//��ʱ�����Ӻ��Զ��Ͽ�
					MyWriteBit(OUT_SUKER, IO_OUT_ON);
					
					//���ϲ�ѯ�Ƿ����ź�
					double MyTime = m_fTime/1000; //��λ��
					double StepTime = 0.800; //��ѯ���
					LARGE_INTEGER MynFreq;
					LARGE_INTEGER MynBeginTime;
					LARGE_INTEGER MyStepBeginTime;
					LARGE_INTEGER MynEndTime;
					double MynFreq_time = 0;
					double MyStepFreq_time = 0;
					QueryPerformanceFrequency(&MynFreq);
					QueryPerformanceCounter(&MynBeginTime); //��ʼʱ��
					QueryPerformanceCounter(&MyStepBeginTime);
					while (MynFreq_time < MyTime) //��ʱ�䲻�ܳ������õ�ʱ��
					{
						QueryPerformanceCounter(&MynEndTime);//����ʱ��
						MynFreq_time = (double)(MynEndTime.QuadPart - MynBeginTime.QuadPart) / (double)MynFreq.QuadPart;
						MyStepFreq_time = (double)(MynEndTime.QuadPart - MyStepBeginTime.QuadPart) / (double)MynFreq.QuadPart;
						
						if (MyStepFreq_time >= StepTime) //���ʱ��ͼ��һ��
						{
							if (MyReadBit(IN_SENS_SUKER) == IO_INVALID)//��⵽���źţ�������ѭ��
							{
								Sleep(100); //����100�����ټ�⣬��ֹ����
								if (MyReadBit(IN_SENS_SUKER) == IO_INVALID)
								{
									break;
								}
							}
							QueryPerformanceCounter(&MyStepBeginTime);//�������¼�ʱ
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

//-1: ��ȡ�ɹ�
//0�� ȡͼʧ��
//1: ��ȡʧ��
BOOL CChecker5678Dlg::ReadBarCode()
{
	//debug
	//return -1;

	BOOL flag = FALSE;
	//��ͼ
	HObject Imgsrc;
	if (!m_ProcCheck6_Code.GrabImageFromCam(Imgsrc))
	{
		//LOG4CPLUS_DEBUG(m_myLoger->logger, "ReadBarCode �������");
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

//-1: �ɹ�
//0: ȡͼʧ��
//1: ˵����ȱʧ
//2����Ƭȱʧ
//3�������ȱʧ
BOOL CChecker5678Dlg::CheckAccessary()
{
	//debug
	//return -1;

	BOOL flag = RES_OK;
	HObject Imgsrc;
	if (!m_ProcCheck1_Book.GrabImageFromCam(Imgsrc))
	{
		//LOG4CPLUS_DEBUG(m_myLoger->logger, "CheckAccessary �������");
		return  RES_CAM;
	}
	ImProcChecker::ContoursAll();
	//���˵����
	if (m_ProcCheck1_Book.CheckDeformShape(Imgsrc))
	{
		SendMessage(WM_MYMSG_BOOK, 1, 0); //�ɹ�
	}
	else
	{
		if (m_ProcCheck1_Book.m_hoDeformShape.IsInitialized())
			m_ProcCheck1_Book.m_hoDeformShape.Clear();
		SendMessage(WM_MYMSG_BOOK, 0, RES_NG1); //ȱʧ
		flag = RES_NG1; //˵����ȱʧ
	}
	//if (m_ProcCheck1_Book.CheckShape(Imgsrc))
	//{
	//	SendMessage(WM_MYMSG_BOOK, 1, 0); //�ɹ�
	//}
	//else
	//{
	//	if (m_ProcCheck1_Book.m_hoShape.IsInitialized())
	//		m_ProcCheck1_Book.m_hoShape.Clear();
	//	SendMessage(WM_MYMSG_BOOK, 0, RES_NG1); //ȱʧ
	//	flag =  RES_NG1; //˵����ȱʧ
	//}
	//��鿨Ƭ
	if (0)
	{
		if (m_ProcCheck3_CardTri.CheckShape(Imgsrc)/* || m_ProcCheck2_CardTitle.CheckShape(Imgsrc)*/ )
		{
			//flag = RES_OK; //
			SendMessage(WM_MYMSG_CARD, 1, 0); //�ɹ�
		}
		else
		{
			if (m_ProcCheck3_CardTri.m_hoShape.IsInitialized())
				m_ProcCheck3_CardTri.m_hoShape.Clear();
			SendMessage(WM_MYMSG_CARD, 0, RES_NG1); //ȱʧ
			flag =  RES_NG2; //��Ƭȱʧ
		}

	}

	//�������
	if (m_ProcCheck4_Dry.CheckBlob(Imgsrc))
	{
		SendMessage(WM_MYMSG_DRY, 1, 0); //�ɹ�
	}
	else
	{
		if (ImProcChecker::g_hoBlobArea.IsInitialized())
			ImProcChecker::g_hoBlobArea.Clear();
		SendMessage(WM_MYMSG_DRY, 0, RES_NG1); //ȱʧ
		flag =  RES_NG3; //�����ȱʧ
	}

	return flag;
}

//-1: �ɹ�
//0: ���ȡͼʧ��
//1: ����ģ���ȱʧ
BOOL CChecker5678Dlg::CheckCovery()
{
	//debug
	//return -1;

	BOOL flag = FALSE;
	HObject Imgsrc;
	if (!m_ProcCheck1_Book.GrabImageFromCam(Imgsrc))
	{
		//LOG4CPLUS_DEBUG(m_myLoger->logger, "CheckAccessary �������");
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

//-1: �ɹ�
//0: ���ȡͼʧ��
//1: û�м�⵽ֹͣ��־
BOOL CChecker5678Dlg::CheckStop()
{
	BOOL flag = FALSE;
	HObject Imgsrc;
	if (!m_ProcCheck1_Book.GrabImageFromCam(Imgsrc))
	{
		//LOG4CPLUS_DEBUG(m_myLoger->logger, "CheckAccessary �������");
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
		WritePrivateProfileString(_T("ɨ��"), _T("MinLength"), _T("6"), strPath);
		WritePrivateProfileString(_T("����"), _T("Max"), _T("9999"), strPath);
		WritePrivateProfileString(_T("����"), _T("Min"), _T("0"), strPath);
		WritePrivateProfileString(_T("����"), _T("Com"), _T("9"), strPath);
		WritePrivateProfileString(_T("����"), _T("Board"), _T("9600"), strPath);
		WritePrivateProfileString(_T("���"), _T("Exp"), _T("60000"), strPath);
		WritePrivateProfileString(_T("���ݿ�"), _T("ConnectStr"), _T("Provider=SQLOLEDB;Server=mfcetn03;Database=VEN;uid=etuser;pwd=mflex123"), strPath);
		WritePrivateProfileString(_T("���ݿ�"), _T("TableName"), _T("ET_Record_Temp"), strPath);
		WritePrivateProfileString(_T("����"), _T("Time"), _T("2000"), strPath);
	}


	//���ؼ������
	CString SName;
	DWORD i = GetPrivateProfileString(_T("����"), _T("Max"), _T("0"), SName.GetBuffer(128), 128, strPath);
	SName.ReleaseBuffer();
	m_MaxWeight = _ttof(SName);

	i = GetPrivateProfileString(_T("����"), _T("Min"), _T("0"), SName.GetBuffer(128), 128, strPath);
	SName.ReleaseBuffer();
	m_MinWeight = _ttof(SName);

	i = GetPrivateProfileString(_T("����"), _T("Com"), _T("0"), SName.GetBuffer(128), 128, strPath);
	SName.ReleaseBuffer();
	com_weight = _ttoi(SName);

	i = GetPrivateProfileString(_T("����"), _T("Board"), _T("0"), SName.GetBuffer(128), 128, strPath);
	SName.ReleaseBuffer();
	com_boad = _ttoi(SName);

	i = GetPrivateProfileString(_T("���"), _T("Exp"), _T("60000"), SName.GetBuffer(128), 128, strPath);
	SName.ReleaseBuffer();
	m_fExporTime = _ttof(SName);

	GetPrivateProfileString(_T("���ݿ�"), _T("ConnectStr"), _T("Provider=SQLOLEDB;Server=mfcetn03;Database=VEN;uid=etuser;pwd=mflex123"), SName.GetBuffer(1024), 1024, strPath);
	SName.ReleaseBuffer();
	m_strConnect = (SName);

	GetPrivateProfileString(_T("���ݿ�"), _T("Tablename"), _T("ET_Record_Temp"), SName.GetBuffer(1024), 1024, strPath);
	SName.ReleaseBuffer();
	m_strTablename = (SName);

	i = GetPrivateProfileString(_T("����"), _T("Time"), _T("2000"), SName.GetBuffer(128), 128, strPath);
	SName.ReleaseBuffer();
	m_fTime = _ttof(SName);

	i = GetPrivateProfileString(_T("����"), _T("DelayTime"), _T("800"), SName.GetBuffer(128), 128, strPath);
	SName.ReleaseBuffer();
	m_fDelayTime = _ttof(SName);
	//i = GetPrivateProfileString(_T("ɨ��"), _T("MinLength"), _T("6"), SName.GetBuffer(128), 128, strPath);
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
	WritePrivateProfileString(_T("����"), _T("Max"), strValue, strPath);
	strValue.Format(_T("%f"), m_MinWeight);
	WritePrivateProfileString(_T("����"), _T("Min"), strValue, strPath);
	strValue.Format(_T("%f"), m_fExporTime);
	WritePrivateProfileString(_T("���"), _T("Exp"), strValue, strPath);
	strValue.Format(_T("%f"), m_fTime);
	WritePrivateProfileString(_T("����"), _T("Time"), strValue, strPath);
	strValue.Format(_T("%f"), m_fDelayTime);
	WritePrivateProfileString(_T("����"), _T("DelayTime"), strValue, strPath);
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
		WritePrivateProfileString(_T("����"), _T("TOTAL"), _T("0"), strPath);
		WritePrivateProfileString(_T("����"), _T("OK"), _T("0"), strPath);
		WritePrivateProfileString(_T("����"), _T("NG"), _T("0"), strPath);
	}


	//���ؼ������
	CString SName;
	DWORD i = GetPrivateProfileString(_T("����"), _T("TOTAL"), _T("0"), SName.GetBuffer(128), 128, strPath);
	SName.ReleaseBuffer();
	m_iTotal = _ttoi(SName);

	i = GetPrivateProfileString(_T("����"), _T("OK"), _T("0"), SName.GetBuffer(128), 128, strPath);
	SName.ReleaseBuffer();
	m_iOkCount = _ttoi(SName);

	i = GetPrivateProfileString(_T("����"), _T("NG"), _T("0"), SName.GetBuffer(128), 128, strPath);
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
	WritePrivateProfileString(_T("����"), _T("TOTAL"), strValue, strPath);
	strValue.Format(_T("%d"), m_iOkCount);
	WritePrivateProfileString(_T("����"), _T("OK"), strValue, strPath);
	strValue.Format(_T("%d"), m_iNgCount);
	WritePrivateProfileString(_T("����"), _T("NG"), strValue, strPath);

	return TRUE;
}

bool CChecker5678Dlg::SaveOnlineData(CString strFilePath, TestRes nRes)
{
	if (nRes.nSerialNumber.GetLength() == 0) return true;
	CStdioFile file;
	COleDateTime t = COleDateTime::GetCurrentTime();
	CString str;
	str.Format(_T("\\%04d%02d.csv"), t.GetYear(), t.GetMonth());
	//�ж�CSV�ļ��Ƿ���ڣ���������ڣ������洴�����������ͷ
	char p[1024];
	bool bAddtile = false;
	CString cstrPath = strFilePath + str;
	int len = WideCharToMultiByte(CP_ACP, 0, cstrPath, -1, NULL, 0, NULL, NULL);//���ַ�ת
	char* strTemp = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, cstrPath, -1, strTemp, len, NULL, NULL);

	if (!IsDirExist(strTemp))
	{
		bAddtile = true;
	}

	delete[]strTemp;

	//����csv�ļ�
	BOOL ret = file.Open(strFilePath + str, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite);
	if (ret == 0) return false;
	file.SeekToEnd();

	if (bAddtile) //������ͷ
	{
		str.Format(_T("TestTime,SerialNumber,Manual,Desiccant,Weight\n"));
		file.WriteString(str);
		file.Flush();
	}
	//д������
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

	//�����ݿ� 
	AdoDatabase adb;
	adb.m_connectstr = m_strConnect;//"Provider=SQLOLEDB;Server=mfcetn03;Database=VEN;uid=etuser;pwd=mflex123";
	if (0 == adb.ConnectDB())
	{
		//1.�Ȳ�ѯ���޴������¼

		//������¼�¼

		//2.���û�У�������¼�¼
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
			//����ɹ�
			//m_ListBox.AddString(_T("���ݿ�д��ɹ�"));
		}
		else
		{
			//m_ListBox.AddString(_T("���ݿ�д��ʧ�ܣ��ѱ����ڱ���CSV�ļ�"));
			error = adb.m_errormessage;
			adb.CloseConn();
			return false;
		}
		adb.CloseConn();
	}
	else
	{
		GetDlgItem(IDC_STATIC_DBCONNECT)->SetWindowText(_T("���ݿ�����ʧ��"));
		error = adb.m_errormessage;
		//m_ListBox.AddString(_T("���ݿ�����ʧ�ܣ��ѱ����ڱ���CSV�ļ�"));
		return false;
	}
	return true;
}


void CChecker5678Dlg::OnBnClickedButtonTest()
{
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_hTestEvent, 10)) //���ֹͣ
	{
		AfxMessageBox(_T("����ʹ����ֹͣ"));
		return;
	}
	//���ļ��м����ļ�
	HObject img;
	m_ProcMeasureUSB.GrabOneAndShow();
	////if (m_ProcCheck1.GrabImageFromFile(NULL, img))
	//{
	//	//ʶ��
	//	//m_ProcCheck1.CheckShape(img);
	//	//m_ProcCheck2.CheckShape(img);
	//	//m_ProcCheck3.CheckShape(img);
	//	//m_ProcCheck4.CheckBlob(img);
	//}

}


void CChecker5678Dlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	//if (WAIT_OBJECT_0 == WaitForSingleObject(m_hTestEvent, 10)) //���ֹͣ
	//{
	//	AfxMessageBox(_T("����ʹ����ֹͣ���ڹرճ���"));
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
	if (0 == state)//��ȡ��
	{
		m_strMsgBarCode = TXT_MSG_READING;
		//ͬʱ��ɫ��ʾ
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
		//��ʾ��ȡ����������
		m_strMsgBarCode = strData->c_str();
		//ͬʱ��ɫ��ʾ
		m_bCheckBarcode = true;
	}
	UpdateData(FALSE);
	return 0;
}

LRESULT CChecker5678Dlg::ShowResultAccess(WPARAM wParam, LPARAM lParam)
{
	int state = (int)wParam;
	if (1 == state)//��ȡ��
	{
		m_strMsgAccessary = TXT_MSG_OK;
		//ͬʱ��ɫ��ʾ
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
		//ͬʱ��ɫ��ʾ
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
	if (1 == state)//��ȡ��
	{
		m_strMsgCovery = TXT_MSG_OK;
		//ͬʱ��ɫ��ʾ
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
	if (1 == state)//��ȡ��
	{
		m_strMsgBook = TXT_MSG_OK;
		//ͬʱ��ɫ��ʾ
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
		//ͬʱ��ɫ��ʾ
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
	if (1 == state)//��ȡ��
	{
		m_strMsgCard = TXT_MSG_OK;
		//ͬʱ��ɫ��ʾ
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
		//ͬʱ��ɫ��ʾ
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
	if (1 == state)//��ȡ��
	{
		m_strMsgDry = TXT_MSG_OK;
		//ͬʱ��ɫ��ʾ
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
		//ͬʱ��ɫ��ʾ
		m_bCheckDry = false;

		m_strMsgCovery = TXT_MSG_CHECKING;
		m_bCheckCovery = false;
	}
	UpdateData(FALSE);
	return 0;
}

LRESULT CChecker5678Dlg::ShowResultHaevy(WPARAM wParam, LPARAM lParam)
{
	//�����Ӧ������������
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

	if (1 == state)//��ȡ��
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
	font.CreatePointFont(size, L"����");
	GetDlgItem(ID)->SetWindowText(str);
	GetDlgItem(ID)->SetFont(&font);
	font.Detach();
}

LRESULT CChecker5678Dlg::OnReceiveStr(WPARAM str, LPARAM commInfo)
{
	struct serialPortInfo
	{
		UINT portNr;//���ں�
		DWORD bytesRead;//��ȡ���ֽ���
	}*pCommInfo;
	pCommInfo = (serialPortInfo*)commInfo;
	DWORD ooo = pCommInfo->bytesRead;

	int len = ooo;
	if (len)
	{
		char* Rv_Cstr = (char*)str;
		
		//memcpy(Schar, Rv_Cstr, len);

		//��ȡ
		CString substr[12800];
		int sBufSize = len;// strlen(Rv_Cstr);
		DWORD dBufSize = MultiByteToWideChar(CP_ACP, 0, Rv_Cstr, sBufSize, NULL, 0);
		wchar_t * dBuf = new wchar_t[dBufSize+1];
		wmemset(dBuf, 0, dBufSize);
		//����ת��
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

				int len = WideCharToMultiByte(CP_ACP, 0, SubStr, -1, NULL, 0, NULL, NULL);//���ַ�ת
				char* strTemp = new char[len + 1];
				WideCharToMultiByte(CP_ACP, 0, SubStr, -1, strTemp, len, NULL, NULL);
				double fweight = atof(strTemp);
				delete[]strTemp;
				//�Ƚ�,�ı���ɫ
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

				//��ʾ
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

	// TODO:  �ڴ˸��� DC ���κ�����
	//if (pWnd->GetDlgCtrlID() == IDC_EDIT_RESULT3) //������
	//{
	//	CString str;
	//	pDC->SetTextColor(RGB(0, 0, 0));     //����������ɫ	
	//	pDC->SetBkMode(TRANSPARENT); //�������屳��Ϊ͸��
	//	if (m_bCheckBarcode)
	//		return m_brPASS; // ���ñ���ɫ
	//	else
	//		return m_brFAIL;
	//}
	//else if (pWnd->GetDlgCtrlID() == IDC_EDIT_RESULT1) //���
	//{
	//	CString str;
	//	pDC->SetTextColor(RGB(0, 0, 0));     //����������ɫ	
	//	pDC->SetBkMode(TRANSPARENT); //�������屳��Ϊ͸��
	//	if (m_bCheckAccessary)
	//		return m_brPASS; // ���ñ���ɫ
	//	else
	//		return m_brFAIL;
	//}
	//else if (pWnd->GetDlgCtrlID() == IDC_EDIT_RESULT2) //����Ĥ
	//{
	//	CString str;
	//	pDC->SetTextColor(RGB(0, 0, 0));     //����������ɫ	
	//	pDC->SetBkMode(TRANSPARENT); //�������屳��Ϊ͸��
	//	if (m_bCheckCovery)
	//		return m_brPASS; // ���ñ���ɫ
	//	else
	//		return m_brFAIL;
	//}
	//else if (pWnd->GetDlgCtrlID() == IDC_EDIT_RESULT4) //����Ĥ
	//{
	//	CString str;
	//	pDC->SetTextColor(RGB(0, 0, 0));     //����������ɫ	
	//	pDC->SetBkMode(TRANSPARENT); //�������屳��Ϊ͸��
	//	if (m_bCheckWeight)
	//		return m_brPASS; // ���ñ���ɫ
	//	else
	//		return m_brFAIL;
	//}
	//else if (pWnd->GetDlgCtrlID() == IDC_EDIT_RESULT6) //book
	//{
	//	CString str;
	//	pDC->SetTextColor(RGB(0, 0, 0));     //����������ɫ	
	//	pDC->SetBkMode(TRANSPARENT); //�������屳��Ϊ͸��
	//	if (m_bCheckBook)
	//		return m_brPASS; // ���ñ���ɫ
	//	else
	//		return m_brFAIL;
	//}
	//else if (pWnd->GetDlgCtrlID() == IDC_EDIT_RESULT7) //card
	//{
	//	CString str;
	//	pDC->SetTextColor(RGB(0, 0, 0));     //����������ɫ	
	//	pDC->SetBkMode(TRANSPARENT); //�������屳��Ϊ͸��
	//	if (m_bCheckCard)
	//		return m_brPASS; // ���ñ���ɫ
	//	else
	//		return m_brFAIL;
	//}
	//else if (pWnd->GetDlgCtrlID() == IDC_EDIT_RESULT8) //dry
	//{
	//	CString str;
	//	pDC->SetTextColor(RGB(0, 0, 0));     //����������ɫ	
	//	pDC->SetBkMode(TRANSPARENT); //�������屳��Ϊ͸��
	//	if (m_bCheckDry)
	//		return m_brPASS; // ���ñ���ɫ
	//	else
	//		return m_brFAIL;
	//}
	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	return hbr;
}


void CChecker5678Dlg::OnBnClickedCheck1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData();
	m_bToCheckBarcode2 = m_bToCheckBarcode;
}


void CChecker5678Dlg::OnBnClickedCheck2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData();
	m_toCheckAcc2 = m_toCheckAcc;
}


void CChecker5678Dlg::OnBnClickedCheck3()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData();
	m_bToCheckCovery2 = m_bToCheckCovery;
}


void CChecker5678Dlg::OnBnClickedButtonStart2()
{
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_hTestEvent, 10)) //���ֹͣ
	{
		AfxMessageBox(_T("����ʹ����ֹͣ"));
		return;
	}
	if (2 == theApp.m_Admin)
		return;
	//LOG4CPLUS_DEBUG(m_myLoger->logger, "����===================");
	//�򿪹�Դ
	MyWriteBit(OUT_LIGHT_CAM, IO_OUT_ON);
	
	SetParDlg dlg;
	dlg.m_pCam = m_pCam;
	dlg.m_pMainDlg = this;
	//��ֵ
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
		//�������
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
		m_ProcCheck1_Book.SavePara(PATH_PARA_LOGO); //˵����
		//m_ProcCheck2_CardTitle.SavePara(PATH_PARA_CARDTITLE); //��Ƭ����
		m_ProcCheck3_CardTri.SavePara(PATH_PARA_CARDTRI); //��Ƭ����
		m_ProcCheck4_Dry.SavePara(PATH_PARA_DRY); //�����
		m_ProcCheck5_Covery.SavePara(PATH_PARA_COVER); //����Ĥ
		m_ProcCheck6_Code.SavePara(PATH_PARA_CODE);
		m_ProcCheck7_Stop.SavePara(PATH_PARA_STOP);

		//LOG4CPLUS_DEBUG(m_myLoger->logger, "���� �˳�==============");
		CRect rect;
		m_show.GetClientRect(&rect);
		//m_mydll.SetHalconDispWnd(m_show.GetSafeHwnd(), rect);
		ImProcChecker::SetDispWnd((Hlong)m_show.GetSafeHwnd(), rect);

		//LOG4CPLUS_DEBUG(m_myLoger->logger, "���¼��ز���===========");
		m_ProcCheck1_Book.LoadPara(PATH_PARA_LOGO); //˵����
		//m_ProcCheck2_CardTitle.LoadPara(PATH_PARA_CARDTITLE); //��Ƭ����
		m_ProcCheck3_CardTri.LoadPara(PATH_PARA_CARDTRI); //��Ƭ����
		m_ProcCheck4_Dry.LoadPara(PATH_PARA_DRY); //�����
		m_ProcCheck5_Covery.LoadPara(PATH_PARA_COVER); //����Ĥ
		m_ProcCheck6_Code.LoadPara(PATH_PARA_CODE);
		m_ProcCheck7_Stop.LoadPara(PATH_PARA_STOP);

		//����ģ��
		//m_ProcCheck1_Book.LoadModel(PATH_MODEL1); //˵����
		//m_ProcCheck1_Book.LoadDeformModel(PATH_MODEL_LOGO_DFM);
		////m_ProcCheck2_CardTitle.LoadModel(PATH_MODEL2); //��Ƭ����
		//m_ProcCheck3_CardTri.LoadModel(PATH_MODEL3); //��Ƭ����
		//m_ProcCheck7_Stop.LoadModel(PATH_MODEL7);
		if (!m_ProcCheck1_Book.LoadModel(PATH_MODEL1)) //˵����
		{
			AfxMessageBox(_T("����˵����LOGOģ��ʧ�ܣ�����logo.shm�ļ���"));
		}
		//m_ProcCheck2_CardTitle.LoadModel(PATH_MODEL2); //��Ƭ����
		if (!m_ProcCheck3_CardTri.LoadModel(PATH_MODEL3)) //��Ƭ����
		{
			AfxMessageBox(_T("���ؿ�Ƭͼ��ģ��ʧ�ܣ�����cardtri.shm�ļ���"));
		}
		if (!m_ProcCheck7_Stop.LoadModel(PATH_MODEL7))
		{
			AfxMessageBox(_T("����ģ��ͼ��ģ��ʧ�ܣ�����stop.shm�ļ���"));
		}
		if (!m_ProcCheck1_Book.LoadDeformModel(PATH_MODEL_LOGO_DFM))//����logo�Ŀ��α�ģ��
		{
			AfxMessageBox(_T("����˵����LOGOģ��ʧ�ܣ�����logo.dfm�ļ���"));
		}
		//LOG4CPLUS_DEBUG(m_myLoger->logger, "���¼��ز��� ���======");
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
	g_criSectionIO.Lock();  //��
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
	g_criSectionIO.Lock();  //��
	U16 out_val = 0;
	short rtdat = NIO_GetDIBit(m_handle, bitno, &out_val);
	if (RTN_CMD_SUCCESS == rtdat)
	{
		if (out_val == 0)
		{
			g_criSectionIO.Unlock();  //��
			return IO_VALID; //��Ч��ƽ
		}
		else
		{
			g_criSectionIO.Unlock();
			return IO_INVALID;   //��Ч��ƽ
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

	//ʹ�øߴ�IO
	m_handle = NULL;
	unsigned short device_num = 0;
	TDevInfo devinfo_list[16];
	short rtdat = NIO_Search(&device_num, &devinfo_list[0]);
	if (RTN_CMD_SUCCESS == rtdat)//* ɨ��ϵͳ�е�SC�豸
	{
		if (device_num > 0)
		{
			char str_id_name[256];
			sprintf_s(str_id_name, "%s", devinfo_list[0].idStr);
			rtdat = NIO_OpenByID(str_id_name, &m_handle);
			if (RTN_CMD_SUCCESS == rtdat)//* ����SC�豸���ƴ��豸
			{
				if (m_handle != NULL)
				{
					return TRUE;
				}
				else
				{
					AfxMessageBox(_T("��ʼ�����ƿ�ʧ�ܣ�"));
					return FALSE;
				}
			}
			else if (RTN_CMD_ERROR == rtdat)
			{
				AfxMessageBox(_T("��ʼ�����ƿ�ʧ�ܣ�"));
				return FALSE;
			}
			else
			{
				AfxMessageBox(_T("��ʼ�����ƿ�ʧ�ܣ�"));
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
	//�ص�
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
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_hTestEvent, 10)) //���ֹͣ
	{
		AfxMessageBox(_T("����ʹ����ֹͣ"));
		return;
	}
	if (2 == theApp.m_Admin)
		return;
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	theApp.menageuser();
	//
	theApp.login();
	if (2 == theApp.m_Admin)
	{
		//�����߽�������
		GetDlgItem(IDC_BUTTON_START2)->EnableWindow(FALSE);
		//�����߽���
		//GetDlgItem(IDC_BUTTON_START3)->EnableWindow(FALSE);
	}
	else
	{
		//�����߽�������
		GetDlgItem(IDC_BUTTON_START2)->EnableWindow(TRUE);
		//�����߽�������
		//GetDlgItem(IDC_BUTTON_START3)->EnableWindow(TRUE);
	}
}


void CChecker5678Dlg::OnBnClickedButton2()
{
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_hTestEvent, 10)) //���ֹͣ
	{
		AfxMessageBox(_T("����ʹ����ֹͣ"));
		return;
	}
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	MyWriteBit(OUT_LIGHT_CAM, IO_OUT_OFF);
}


void CChecker5678Dlg::OnBnClickedButton3()
{
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_hTestEvent, 10)) //���ֹͣ
	{
		AfxMessageBox(_T("����ʹ����ֹͣ"));
		return;
	}
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_hTestEvent, 10)) //���ֹͣ
	{
		AfxMessageBox(_T("����ʹ����ֹͣ"));
		return;
	}
	//���ļ��м����ļ�
	HObject img;
	m_ProcMeasureUSB.GrabImageFromFile(0, img);
}


void CChecker5678Dlg::OnBnClickedButtonDrawarea()
{
	if (!m_ProcMeasureUSB.Show())
	{
		AfxMessageBox(_T("��ʾʧ��"));
		return;
	}
	//��ѡ�������
	if (IDOK != AfxMessageBox(_T("���ѡ���Ŀ�����������Ҽ�������ѡ��"), MB_OKCANCEL))
	{
		return;
	}
	m_brect = false;
	DetectArea rect;
	DetectArea rect_refer;
	if (!m_ProcMeasureUSB.DrawRect(rect))
	{
	}

	//��ѡ�ο�����
	if (IDOK != AfxMessageBox(_T("���ѡһ���ο�����"), MB_OKCANCEL))
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
	//����
	if (m_ProcMeasureUSB.SavePara(PATH_PARA_MEASURE_USB))
		AfxMessageBox(_T("����ɹ�"));
	else
		AfxMessageBox(_T("����ʧ��"));

	m_ProcMeasureUSB.LoadPara(PATH_PARA_MEASURE_USB);
}


void CChecker5678Dlg::OnBnClickedButtonDrawarea2()
{
	if (!m_ProcMeasureCircle.Show())
	{
		AfxMessageBox(_T("��ʾʧ��"));
		return;
	}
	//��ѡ�������
	if (IDOK != AfxMessageBox(_T("���ѡ���Ŀ�����������Ҽ�������ѡ��"), MB_OKCANCEL))
	{
		return;
	}
	m_brect = false;
	DetectArea rect;
	DetectArea rect_refer;
	if (!m_ProcMeasureCircle.DrawRect(rect))
	{
	}

	//��ѡ�ο�����
	if (IDOK != AfxMessageBox(_T("���ѡһ���ο�����"), MB_OKCANCEL))
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
	//����
	if (m_ProcMeasureCircle.SavePara(PATH_PARA_CIRCLE))
		AfxMessageBox(_T("����ɹ�"));
	else
		AfxMessageBox(_T("����ʧ��"));

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
			ShowMsg(_T("����ɼ�ʧ��."));
			AfxMessageBox(_T("����ɼ�ʧ��."));
			return;
		}
	}

	if (m_ProcMeasureCircle.MeasureCircle() != 0)
	{
		MeasureResult res;
		m_ProcMeasureCircle.GetReult(res);
		CString str;
		str.Format(_T("ʶ��ʧ�ܣ� �����룺 %d"), res.errorCode);
		ShowMsg(str);
	}
	else
	{
		MeasureResult res;
		m_ProcMeasureCircle.GetReult(res);
		CString str;
		str.Format(_T("length�� %.4f"), res.length);
		ShowMsg(str);
		str.Format(_T("cent_x�� %.4f"), res.cent_x);
		ShowMsg(str);
		str.Format(_T("cent_y�� %.4f"), res.cent_y);
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
		// TODO: �ڴ���ӿؼ�֪ͨ����������
		if (m_ProcMeasureUSB.SavePara(PATH_PARA_MEASURE_USB))
			AfxMessageBox(_T("����ɹ�"));
		else
			AfxMessageBox(_T("����ʧ��"));
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
		// TODO: �ڴ���ӿؼ�֪ͨ����������
		if (m_ProcMeasureCircle.SavePara(PATH_PARA_MEASURE_USB))
			AfxMessageBox(_T("����ɹ�"));
		else
			AfxMessageBox(_T("����ʧ��"));
	}
}
