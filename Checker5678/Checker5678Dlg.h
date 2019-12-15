
// Checker5678Dlg.h : 头文件
//

#pragma once
#include "afxwin.h"


//测试结果结构体
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
// CChecker5678Dlg 对话框
class CChecker5678Dlg : public CDialogEx
{
// 构造
public:
	CChecker5678Dlg(CWnd* pParent = NULL);	// 标准构造函数
	~CChecker5678Dlg();
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHECKER5678_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	//界面
	myCameraClass* m_pCam;	//相机
	static int g_num;
	int m_iIndex;		 //相机序号
	double       m_fExporTime;
	CStatic m_show;      //相机显示
	bool m_bToStart;     //有权限开始测试的标志
	bool m_brect;

	HBRUSH m_brPASS;
	HBRUSH m_brFAIL;
	// 检查配件的信息
	CString m_strMsgAccessary;
	// 检查保护膜的信息
	CString m_strMsgCovery;
	// 检查条形码的信息
	CString m_strMsgBarCode;
	//控制
	bool m_bButtonTrig; //是否按钮触发开始识别
	bool m_bCamConneted; //
	bool m_bexit;
	HANDLE	m_hTestEvent;
	bool m_isPutOn; //是否来料
	bool m_bCheckBarcode; //每一轮的结果OK,NG
	bool m_bCheckAccessary;
	bool m_bCheckCovery;
	
	bool m_bCheckBook;
	bool m_bCheckCard;
	bool m_bCheckDry;

	bool m_bCheckBarcodeOnce; //总的结果OK,NG，只要一次OK， 这个就是
	bool m_bCheckAccessaryOnce;
	bool m_bCheckCoveryOnce;
	//识别的开关
	BOOL m_bToCheckBarcode;
	BOOL m_toCheckAcc;
	BOOL m_bToCheckCovery;
	bool m_bToCheckBarcode2;
	bool m_toCheckAcc2;
	bool m_bToCheckCovery2;
	
	//算法
	ImProcChecker m_ProcCheck1_Book; //检查说明书logo
	//ImProcChecker m_ProcCheck2_CardTitle; //检查卡片标题
	ImProcChecker m_ProcCheck3_CardTri; //检查卡片三角
	ImProcChecker m_ProcCheck4_Dry; //检查干燥剂
	ImProcChecker m_ProcCheck5_Covery; //检查保护膜
	ImProcChecker m_ProcCheck6_Code; //检测条形码
	ImProcChecker m_ProcCheck7_Stop; //空位标志，表示没有料盒在上面
	ImProcChecker m_ProcMeasureUSB; //
	ImProcChecker m_ProcMeasureCircle; 
	//扫码
	int m_MinLength;

	//称重
	CSerialPort m_SerialPort;
	int com_weight;
	int com_boad;
	double m_MaxWeight; //上限
	double m_MinWeight; //下限
	bool m_bCheckWeight;
	double m_fWeight;
	bool m_bWeight;

	//输入输出
	HAND m_handle;  //高川IO控制卡句柄
	bool m_cardconnect; //连接成功标志
	bool m_bSucker; //是否吸住

	//数据库
	CString m_strConnect;   //连接数据库串
	CString m_strTablename; //表名 
	bool m_bConnectDB;

	TestRes m_testres; //数据结果
	//日志
	//MyLogger		* m_myLoger;			//日志类

	//吸盘
	double m_fTime;  //吸盘持续时间
	double m_fDelayTime; //传感器触发时间延时
	//统计
	int m_iTotal;
	int m_iOkCount;
	int m_iNgCount;
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnBnClickedButtonStart();
	int Mainread();
	int Mainread2(); //使用按钮触发
	int MainProcess(); //
	int Weight();
	int Sucker();
	BOOL ReadBarCode(); //读取二维码
	BOOL CheckAccessary(); //检查配件
	BOOL CheckCovery();//检测是否有保护膜
	BOOL CheckStop(); //检测停止标志，如果为真，表示没有料盒放上来。
	BOOL LoadPara(wchar_t * pParaPath);   //
	BOOL SavePara(wchar_t * pParaPath);
	BOOL LoadCount(wchar_t * pParaPath);   //
	BOOL SaveCount(wchar_t * pParaPath);
	bool SaveOnlineData(CString strFilePath, TestRes nRes); //保存数据到本地
	bool SaveOnlineDataSQL(CString strFilePath, TestRes nRes, CString &error); //保存数据到远程SQL
	afx_msg void OnBnClickedButtonTest();
	afx_msg void OnClose();
	afx_msg LRESULT ShowResultBarcode(WPARAM, LPARAM);//自定义消息处理，显示结果
	afx_msg LRESULT ShowResultAccess(WPARAM, LPARAM);//自定义消息处理，显示结果
	afx_msg LRESULT ShowResultCovery(WPARAM, LPARAM);//自定义消息处理，显示结果
	afx_msg LRESULT ShowResultBook(WPARAM, LPARAM);//自定义消息处理，显示结果
	afx_msg LRESULT ShowResultCard(WPARAM, LPARAM);//自定义消息处理，显示结果
	afx_msg LRESULT ShowResultDry(WPARAM, LPARAM);//自定义消息处理，显示结果
	afx_msg LRESULT ShowResultHaevy(WPARAM, LPARAM);//自定义消息处理，显示结果
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	void CStaticShow(int ID, CString str, int size);
	afx_msg LRESULT OnReceiveStr(WPARAM str, LPARAM commInfo);
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedCheck2();
	afx_msg void OnBnClickedCheck3();
	afx_msg void OnBnClickedButtonStart2();
	//函数：板卡输入输出
	//返回值： 0表示成功， -1表示失败
	short MyWriteBit(int bitno, int on_off);
	short MyReadBit(int bitno);
	BOOL InitMotion();
	BOOL InitIO();
	BOOL showContour();
	afx_msg void OnBnClickedButtonStart3();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	void MenageSave();  //管理是否保存
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
