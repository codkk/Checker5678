#pragma once
#include "afxwin.h"


// SetParDlg �Ի���
class CChecker5678Dlg;
class SetParDlg : public CDialog
{
	DECLARE_DYNAMIC(SetParDlg)

public:
	SetParDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~SetParDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	ImProcChecker m_ProcCheck;
	CChecker5678Dlg * m_pMainDlg;
	HAND m_handleIO; //io���ƿ�

	virtual BOOL OnInitDialog();
	CStatic m_show;
	bool m_brect;
	DetectArea m_rect;
	myCameraClass* m_pCam;	//���
	afx_msg void OnBnClickedButtonDraw();
	afx_msg void OnBnClickedButtonSaveasBarcode();
	afx_msg void OnBnClickedButtonSaveasBook();
	afx_msg void OnBnClickedButtonSaveasCard();
	afx_msg void OnBnClickedButtonSaveasCovery();
	afx_msg void OnBnClickedButtonSaveasCovery2();
	afx_msg void OnBnClickedButton1();
	int m_OutIdex;
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	// ��С���볤��
	int m_iMinLength;
	double m_lfMinScore_book;
	double m_lfMinScore_card;
	int m_iMinGray;
	int m_iMaxGray;
	int m_iClosingSize;
	int m_iMinArea;
	int m_iMaxArea;
	double m_lfMinWeight;
	double m_lfMaxWeight;
	double m_lfExpo;
	double m_lfTime;
	afx_msg void OnBnClickedButton4();
	double m_lfMinRect;
	double m_lfMinGray_Cover;
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton7();
	afx_msg void OnBnClickedButton8();
	afx_msg void OnBnClickedButton9();
	afx_msg void OnBnClickedButton10();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void MakeShapeModel(char* path); //��������ƥ��
	void MakeDeformModel(char* path); //�����ɱ�������ƥ��
									  // ������ʱ
	double m_fDelayTime;
	afx_msg void OnBnClickedButtonDraw2();
};
