#pragma once
#include "afxwin.h"


// SetParDlg 对话框
class CChecker5678Dlg;
class SetParDlg : public CDialog
{
	DECLARE_DYNAMIC(SetParDlg)

public:
	SetParDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~SetParDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	ImProcChecker m_ProcCheck;
	CChecker5678Dlg * m_pMainDlg;
	HAND m_handleIO; //io控制卡

	virtual BOOL OnInitDialog();
	CStatic m_show;
	bool m_brect;
	DetectArea m_rect;
	myCameraClass* m_pCam;	//相机
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
	// 最小条码长度
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
	void MakeShapeModel(char* path); //创建轮廓匹配
	void MakeDeformModel(char* path); //创建可变形轮廓匹配
									  // 触发延时
	double m_fDelayTime;
	afx_msg void OnBnClickedButtonDraw2();
};
