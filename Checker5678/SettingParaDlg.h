#pragma once


// SettingParaDlg �Ի���

class SettingParaDlg : public CDialogEx
{
	DECLARE_DYNAMIC(SettingParaDlg)

public:
	SettingParaDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~SettingParaDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG3 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

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
