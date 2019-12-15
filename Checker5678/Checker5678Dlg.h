
// Checker5678Dlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"


//���Խ���ṹ��
typedef struct St_TestRes {
	CString nSerialNumber;
	CString nTestTime;
	CString nTestResult;
	CString nTestType;
	CString nPartNumber;
	CString nLine;
	CString nIP_Address;
	CString nMachine;
	CString nOperator;
	CString OnLine;
	std::vector<CString> vTestResult;
}TestRes;
//class MyLogger;
// CChecker5678Dlg �Ի���
class CChecker5678Dlg : public CDialogEx
{
// ����
public:
	CChecker5678Dlg(CWnd* pParent = NULL);	// ��׼���캯��
	~CChecker5678Dlg();
// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHECKER5678_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	//����
	myCameraClass* m_pCam;	//���
	static int g_num;
	int m_iIndex;		 //������
	double       m_fExporTime;
	CStatic m_show;      //�����ʾ
	bool m_bToStart;     //��Ȩ�޿�ʼ���Եı�־
	bool m_brect;

	HBRUSH m_brPASS;
	HBRUSH m_brFAIL;
	// ����������Ϣ
	CString m_strMsgAccessary;
	// ��鱣��Ĥ����Ϣ
	CString m_strMsgCovery;
	// ������������Ϣ
	CString m_strMsgBarCode;
	//����
	bool m_bButtonTrig; //�Ƿ�ť������ʼʶ��
	bool m_bCamConneted; //
	bool m_bexit;
	HANDLE	m_hTestEvent;
	bool m_isPutOn; //�Ƿ�����
	bool m_bCheckBarcode; //ÿһ�ֵĽ��OK,NG
	bool m_bCheckAccessary;
	bool m_bCheckCovery;
	
	bool m_bCheckBook;
	bool m_bCheckCard;
	bool m_bCheckDry;

	bool m_bCheckBarcodeOnce; //�ܵĽ��OK,NG��ֻҪһ��OK�� �������
	bool m_bCheckAccessaryOnce;
	bool m_bCheckCoveryOnce;
	//ʶ��Ŀ���
	BOOL m_bToCheckBarcode;
	BOOL m_toCheckAcc;
	BOOL m_bToCheckCovery;
	bool m_bToCheckBarcode2;
	bool m_toCheckAcc2;
	bool m_bToCheckCovery2;
	
	//�㷨
	ImProcChecker m_ProcCheck1_Book; //���˵����logo
	//ImProcChecker m_ProcCheck2_CardTitle; //��鿨Ƭ����
	ImProcChecker m_ProcCheck3_CardTri; //��鿨Ƭ����
	ImProcChecker m_ProcCheck4_Dry; //�������
	ImProcChecker m_ProcCheck5_Covery; //��鱣��Ĥ
	ImProcChecker m_ProcCheck6_Code; //���������
	ImProcChecker m_ProcCheck7_Stop; //��λ��־����ʾû���Ϻ�������
	ImProcChecker m_ProcMeasureUSB; //
	ImProcChecker m_ProcMeasureCircle; 
	//ɨ��
	int m_MinLength;

	//����
	CSerialPort m_SerialPort;
	int com_weight;
	int com_boad;
	double m_MaxWeight; //����
	double m_MinWeight; //����
	bool m_bCheckWeight;
	double m_fWeight;
	bool m_bWeight;

	//�������
	HAND m_handle;  //�ߴ�IO���ƿ����
	bool m_cardconnect; //���ӳɹ���־
	bool m_bSucker; //�Ƿ���ס

	//���ݿ�
	CString m_strConnect;   //�������ݿ⴮
	CString m_strTablename; //���� 
	bool m_bConnectDB;

	TestRes m_testres; //���ݽ��
	//��־
	//MyLogger		* m_myLoger;			//��־��

	//����
	double m_fTime;  //���̳���ʱ��
	double m_fDelayTime; //����������ʱ����ʱ
	//ͳ��
	int m_iTotal;
	int m_iOkCount;
	int m_iNgCount;
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnBnClickedButtonStart();
	int Mainread();
	int Mainread2(); //ʹ�ð�ť����
	int MainProcess(); //
	int Weight();
	int Sucker();
	BOOL ReadBarCode(); //��ȡ��ά��
	BOOL CheckAccessary(); //������
	BOOL CheckCovery();//����Ƿ��б���Ĥ
	BOOL CheckStop(); //���ֹͣ��־�����Ϊ�棬��ʾû���Ϻз�������
	BOOL LoadPara(wchar_t * pParaPath);   //
	BOOL SavePara(wchar_t * pParaPath);
	BOOL LoadCount(wchar_t * pParaPath);   //
	BOOL SaveCount(wchar_t * pParaPath);
	bool SaveOnlineData(CString strFilePath, TestRes nRes); //�������ݵ�����
	bool SaveOnlineDataSQL(CString strFilePath, TestRes nRes, CString &error); //�������ݵ�Զ��SQL
	afx_msg void OnBnClickedButtonTest();
	afx_msg void OnClose();
	afx_msg LRESULT ShowResultBarcode(WPARAM, LPARAM);//�Զ�����Ϣ������ʾ���
	afx_msg LRESULT ShowResultAccess(WPARAM, LPARAM);//�Զ�����Ϣ������ʾ���
	afx_msg LRESULT ShowResultCovery(WPARAM, LPARAM);//�Զ�����Ϣ������ʾ���
	afx_msg LRESULT ShowResultBook(WPARAM, LPARAM);//�Զ�����Ϣ������ʾ���
	afx_msg LRESULT ShowResultCard(WPARAM, LPARAM);//�Զ�����Ϣ������ʾ���
	afx_msg LRESULT ShowResultDry(WPARAM, LPARAM);//�Զ�����Ϣ������ʾ���
	afx_msg LRESULT ShowResultHaevy(WPARAM, LPARAM);//�Զ�����Ϣ������ʾ���
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	void CStaticShow(int ID, CString str, int size);
	afx_msg LRESULT OnReceiveStr(WPARAM str, LPARAM commInfo);
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedCheck2();
	afx_msg void OnBnClickedCheck3();
	afx_msg void OnBnClickedButtonStart2();
	//�������忨�������
	//����ֵ�� 0��ʾ�ɹ��� -1��ʾʧ��
	short MyWriteBit(int bitno, int on_off);
	short MyReadBit(int bitno);
	BOOL InitMotion();
	BOOL InitIO();
	BOOL showContour();
	afx_msg void OnBnClickedButtonStart3();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	void MenageSave();  //�����Ƿ񱣴�
	CListBox m_listMsg;
	bool m_btoShowMsg;
	void ShowMsg(LPCTSTR msg);
	
	CString m_strMsgBook;
	CString m_strMsgCard;
	CString m_strMsgDry;
	afx_msg void OnBnClickedButtonTest2();
	afx_msg void OnBnClickedButtonDrawarea();
	BOOL m_bCurrent;
	CButton m_btn_draw1;
	afx_msg void OnBnClickedButtonDrawarea2();
	afx_msg void OnBnClickedButtonStart4();
	afx_msg void OnBnClickedButtonTest3();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton6();
};
