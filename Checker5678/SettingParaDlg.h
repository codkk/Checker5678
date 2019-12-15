#pragma once


// SettingParaDlg 对话框

class SettingParaDlg : public CDialogEx
{
	DECLARE_DYNAMIC(SettingParaDlg)

public:
	SettingParaDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~SettingParaDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG3 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	double para1;
	double para2;
	double para3;
	double para4;
	double para5;
	double para6;
	double para7;
};
