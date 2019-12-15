// SetParDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Checker5678.h"
#include "SetParDlg.h"
#include "afxdialogex.h"
#include "Checker5678Dlg.h"

// SetParDlg �Ի���

IMPLEMENT_DYNAMIC(SetParDlg, CDialog)

SetParDlg::SetParDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG1, pParent)
	, m_OutIdex(0)
	, m_iMinLength(0)
	, m_lfMinScore_book(0)
	, m_lfMinScore_card(0)
	, m_iMinGray(0)
	, m_iMaxGray(0)
	, m_iClosingSize(0)
	, m_iMinArea(0)
	, m_iMaxArea(0)
	, m_lfMinWeight(0)
	, m_lfMaxWeight(0)
	, m_lfExpo(0)
	, m_lfTime(0)
	, m_lfMinRect(0)
	, m_lfMinGray_Cover(0)
	, m_fDelayTime(800)
{
	m_brect = false;
	m_pCam = NULL;
	m_handleIO = NULL;
	m_pMainDlg = NULL;
}

SetParDlg::~SetParDlg()
{
}

void SetParDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_SHOW2, m_show);
	DDX_Text(pDX, IDC_EDIT1, m_OutIdex);
	DDX_Text(pDX, IDC_EDIT2, m_iMinLength);
	DDX_Text(pDX, IDC_EDIT3, m_lfMinScore_book);
	DDX_Text(pDX, IDC_EDIT4, m_lfMinScore_card);
	DDX_Text(pDX, IDC_EDIT5, m_iMinGray);
	DDX_Text(pDX, IDC_EDIT6, m_iMaxGray);
	DDX_Text(pDX, IDC_EDIT7, m_iClosingSize);
	DDX_Text(pDX, IDC_EDIT8, m_iMinArea);
	DDX_Text(pDX, IDC_EDIT9, m_iMaxArea);
	DDX_Text(pDX, IDC_EDIT10, m_lfMinWeight);
	DDX_Text(pDX, IDC_EDIT11, m_lfMaxWeight);
	DDX_Text(pDX, IDC_EDIT12, m_lfExpo);
	DDX_Text(pDX, IDC_EDIT13, m_lfTime);
	DDX_Text(pDX, IDC_EDIT14, m_lfMinRect);
	DDX_Text(pDX, IDC_EDIT15, m_lfMinGray_Cover);
	DDX_Text(pDX, IDC_EDIT16, m_fDelayTime);
}


BEGIN_MESSAGE_MAP(SetParDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_DRAW, &SetParDlg::OnBnClickedButtonDraw)
	ON_BN_CLICKED(IDC_BUTTON_SAVEAS_BARCODE, &SetParDlg::OnBnClickedButtonSaveasBarcode)
	ON_BN_CLICKED(IDC_BUTTON_SAVEAS_BOOK, &SetParDlg::OnBnClickedButtonSaveasBook)
	ON_BN_CLICKED(IDC_BUTTON_SAVEAS_CARD, &SetParDlg::OnBnClickedButtonSaveasCard)
	ON_BN_CLICKED(IDC_BUTTON_SAVEAS_COVERY, &SetParDlg::OnBnClickedButtonSaveasCovery)
	ON_BN_CLICKED(IDC_BUTTON_SAVEAS_COVERY2, &SetParDlg::OnBnClickedButtonSaveasCovery2)
	ON_BN_CLICKED(IDC_BUTTON1, &SetParDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &SetParDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &SetParDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &SetParDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &SetParDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &SetParDlg::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, &SetParDlg::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON8, &SetParDlg::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON9, &SetParDlg::OnBnClickedButton9)
	ON_BN_CLICKED(IDC_BUTTON10, &SetParDlg::OnBnClickedButton10)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_DRAW2, &SetParDlg::OnBnClickedButtonDraw2)
END_MESSAGE_MAP()


// SetParDlg ��Ϣ�������


BOOL SetParDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	CRect rect;
	m_show.GetClientRect(&rect);
	//m_mydll.SetHalconDispWnd(m_show.GetSafeHwnd(), rect);
	ImProcChecker::SetDispWnd((Hlong)m_show.GetSafeHwnd(), rect);
	ImProcChecker::SetCam(m_pCam);
	/*m_ProcCheck.GrabOneAndShow();*/
	SetTimer(1, 1500, NULL);
	//m_ProcCheck.SetDispWnd();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


void SetParDlg::OnBnClickedButtonDraw()
{
	GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
	m_brect = false;
	m_ProcCheck.ClearRect();
	m_ProcCheck.GrabOneAndShow();
	AfxMessageBox(_T("���ѡ������Ҽ�����"));
	if (m_ProcCheck.DrawRect(m_rect))
	{
		m_brect = true;
		GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);
		return;
	}
	m_brect = false;
	GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
	return;
}


void SetParDlg::OnBnClickedButtonSaveasBarcode()
{
	if (!m_brect) return;
	if (IDOK != AfxMessageBox(_T("�Ƿ񱣴�Ϊ ������ �������"), MB_OKCANCEL))
	{
		return;
	}
	//�ȼ���
	m_ProcCheck.LoadPara(PATH_PARA_CODE);
	//�޸�
	m_ProcCheck.m_Para.decArea = m_rect;
	//����
	if(m_ProcCheck.SavePara(PATH_PARA_CODE))
		AfxMessageBox(_T("����ɹ�"));
	else
		AfxMessageBox(_T("����ʧ��"));
}


void SetParDlg::OnBnClickedButtonSaveasBook()
{
	if (!m_brect) return;
	if (IDOK != AfxMessageBox(_T("�Ƿ񱣴�Ϊ ˵���� �������"), MB_OKCANCEL))
	{
		return;
	}
	//�ȼ���
	m_ProcCheck.LoadPara(PATH_PARA_LOGO);
	//�޸�
	m_ProcCheck.m_Para.decArea = m_rect;
	//����
	if (m_ProcCheck.SavePara(PATH_PARA_LOGO))
		AfxMessageBox(_T("����ɹ�"));
	else
		AfxMessageBox(_T("����ʧ��"));
}


void SetParDlg::OnBnClickedButtonSaveasCard()
{
	if (!m_brect) return;
	if (IDOK != AfxMessageBox(_T("�Ƿ񱣴�Ϊ ��Ƭ �������"), MB_OKCANCEL))
	{
		return;
	}
	//�ȼ���
	m_ProcCheck.LoadPara(PATH_PARA_CARDTRI);
	//�޸�
	m_ProcCheck.m_Para.decArea = m_rect;
	//����
	m_ProcCheck.SavePara(PATH_PARA_CARDTRI);

	//�ȼ���
	ImProcChecker m_ProcCheck2;
	m_ProcCheck2.LoadPara(PATH_PARA_CARDTITLE);
	//�޸�
	m_ProcCheck2.m_Para.decArea = m_rect;
	//����
	BOOL ret = m_ProcCheck2.SavePara(PATH_PARA_CARDTITLE);

	if (ret)
		AfxMessageBox(_T("����ɹ�"));
	else
		AfxMessageBox(_T("����ʧ��"));
}


void SetParDlg::OnBnClickedButtonSaveasCovery()
{
	if (!m_brect) return;
	if (IDOK != AfxMessageBox(_T("�Ƿ񱣴�Ϊ ����Ĥ �������"), MB_OKCANCEL))
	{
		return;
	}
	//�ȼ���
	m_ProcCheck.LoadPara(PATH_PARA_COVER);
	//�޸�
	m_ProcCheck.m_Para.decArea = m_rect;
	//����
	if (m_ProcCheck.SavePara(PATH_PARA_COVER))
		AfxMessageBox(_T("����ɹ�"));
	else
		AfxMessageBox(_T("����ʧ��"));
}


void SetParDlg::OnBnClickedButtonSaveasCovery2()
{
	if (!m_brect) return;
	if (IDOK != AfxMessageBox(_T("�Ƿ񱣴�Ϊ ����� �������"), MB_OKCANCEL))
	{
		return;
	}
	//�ȼ���
	m_ProcCheck.LoadPara(PATH_PARA_COVER);
	//�޸�
	m_ProcCheck.m_Para.decArea = m_rect;
	//����
	if (m_ProcCheck.SavePara(PATH_PARA_COVER))
		AfxMessageBox(_T("����ɹ�"));
	else
		AfxMessageBox(_T("����ʧ��"));
}


void SetParDlg::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData();
	OnOK();
}

//�����
void SetParDlg::OnBnClickedButton2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData();
	if ((m_OutIdex < 0) || (m_OutIdex > 31)) return;
	if (m_pMainDlg)
	{
		m_pMainDlg->MyWriteBit(m_OutIdex, IO_OUT_ON);
	}
}

//�ر����
void SetParDlg::OnBnClickedButton3()
{
	UpdateData();
	if ((m_OutIdex < 0) || (m_OutIdex > 31)) return;
	if (m_pMainDlg)
	{
		m_pMainDlg->MyWriteBit(m_OutIdex, IO_OUT_OFF);
	}
}


void SetParDlg::OnBnClickedButton4()
{
	OnCancel();
}


void SetParDlg::OnBnClickedButton5()
{
	if (m_pMainDlg)
	{
		m_pMainDlg->MyWriteBit(OUT_LIGHT_CAM, IO_OUT_ON);
	}
}


void SetParDlg::OnBnClickedButton6()
{
	if (m_pMainDlg)
	{
		m_pMainDlg->MyWriteBit(OUT_LIGHT_CAM, IO_OUT_OFF);
	}
}


void SetParDlg::OnBnClickedButton7()
{
	m_ProcCheck.GrabOneAndShow();
}

//����˵�����ϵ�����ģ�壬����logo
void SetParDlg::OnBnClickedButton8()
{
	if (!m_brect)
	{
		AfxMessageBox(_T("���ѡ����"));
		return;
	}
	if (IDOK != AfxMessageBox(_T("�Ƿ񴴽�Ϊ˵���������ģ�壿��������logo.dfm������ǰ���ݺ�"), MB_OKCANCEL))
	{
		return;
	}
	
	//MakeShapeModel(PATH_MODEL1);
	MakeDeformModel(PATH_MODEL_LOGO_DFM);
}


void SetParDlg::OnBnClickedButton9()
{
	if (!m_brect)
	{
		AfxMessageBox(_T("���ѡ����"));
		return;
	}
	if (IDOK != AfxMessageBox(_T("�Ƿ񴴽�Ϊ��Ƭ������ģ�壿��������cardtri.dfm������ǰ���ݺ�"), MB_OKCANCEL))
	{
		return;
	}

	//MakeShapeModel(PATH_MODEL3);
	MakeDeformModel(PATH_MODEL_TRI_DFM);
}


void SetParDlg::OnBnClickedButton10()
{
	if (!m_brect)
	{
		AfxMessageBox(_T("���ѡ����"));
		return;
	}
	if (IDOK != AfxMessageBox(_T("�Ƿ񴴽�Ϊ��Ӧ���������ģ�壿��������stop.shm������ǰ���ݺ�"), MB_OKCANCEL))
	{
		return;
	}
	
	MakeShapeModel(PATH_MODEL7);
}


void SetParDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if (1 == nIDEvent)
	{
		m_ProcCheck.GrabOneAndShow();
		KillTimer(1);
	}
	CDialog::OnTimer(nIDEvent);
}

void SetParDlg::MakeShapeModel(char* path)
{
	try
	{
		//��ͼ
		HObject roi;
		HalconCpp::GenRectangle1(&roi, m_rect.row1, m_rect.col1, m_rect.row2, m_rect.col2);
		//����ģ��
		if (m_ProcCheck.ho_Image.IsInitialized())
		{
			HObject img = m_ProcCheck.ho_Image.Clone();
			HObject imgreduce;
			ReduceDomain(img, roi, &imgreduce);
			HalconCpp::HTuple hv_ModelId;
			CreateScaledShapeModel(imgreduce, 4, HTuple(0).TupleRad(), HTuple(360).TupleRad(),
				HTuple(0.701).TupleRad(), 0.89, 1.12, "auto", (HTuple("point_reduction_high").Append("no_pregeneration")),
				"use_polarity", "auto", "auto", &hv_ModelId);

			//����ģ��
			WriteShapeModel(hv_ModelId, path/*PATH_MODEL1*/);
			ClearShapeModel(hv_ModelId);
			AfxMessageBox(_T("�����ɹ�"));
			m_brect = false;
		}
		else
		{
			AfxMessageBox(_T("����ʧ�ܣ�ͼ��Ϊ�գ�"));
			m_brect = false;
			return;
		}
	}
	catch (HalconCpp::HException & except)
	{
		//ShowException(except);
		AfxMessageBox(_T("����ʧ�ܣ���ѡ��������"));
		m_brect = false;
		return;
	}
}

void SetParDlg::MakeDeformModel(char * path)
{
	try
	{
		//��ͼ
		HObject roi;
		HalconCpp::GenRectangle1(&roi, m_rect.row1, m_rect.col1, m_rect.row2, m_rect.col2);
		//����ģ��
		if (m_ProcCheck.ho_Image.IsInitialized())
		{
			HObject img = m_ProcCheck.ho_Image.Clone();
			HObject imgreduce;
			ReduceDomain(img, roi, &imgreduce);
			//
			HTuple  hv_AngleExtent, hv_AngleExtentTracking;
			HTuple  hv_MinScore, hv_MinScoreTracking, hv_TrackingRadius;
			HTuple  hv_ScaleROffset, hv_ScaleCOffset, hv_Metric, hv_Area;
			HTuple  hv_Row, hv_Column, hv_ModelID, hv_ImageFiles, hv_Index;
			HTuple  hv_HomMat2D, hv_Score;
			hv_AngleExtent = 360;
			hv_MinScore = 0.4;
			hv_MinScoreTracking = 0.5;
			hv_TrackingRadius = 80;
			hv_ScaleROffset = 0.1;
			hv_ScaleCOffset = 0.0;
			hv_Metric = "use_polarity";
			AreaCenter(roi, &hv_Area, &hv_Row, &hv_Column);
			CreatePlanarUncalibDeformableModel(imgreduce, "auto", ((-hv_AngleExtent) / 2.0).TupleRad(),
				hv_AngleExtent.TupleRad(), "auto", 1, 1 + hv_ScaleROffset, "auto", 1, 1 + hv_ScaleCOffset,
				"auto", "none", hv_Metric, "auto", "auto", HTuple(), HTuple(), &hv_ModelID);
			SetDeformableModelOrigin(hv_ModelID, -hv_Row, -hv_Column);

			//����ģ��
			WriteDeformableModel(hv_ModelID, path/*PATH_MODEL1*/);
			ClearDeformableModel(hv_ModelID);
			AfxMessageBox(_T("�����ɹ�"));
			m_brect = false;
		}
		else
		{
			AfxMessageBox(_T("����ʧ�ܣ�ͼ��Ϊ�գ�"));
			m_brect = false;
			return;
		}
	}
	catch (HalconCpp::HException & except)
	{
		//ShowException(except);
		AfxMessageBox(_T("����ʧ�ܣ���ѡ��������"));
		m_brect = false;
		return;
	}
}


void SetParDlg::OnBnClickedButtonDraw2()
{
	OnBnClickedButtonDraw();
}
