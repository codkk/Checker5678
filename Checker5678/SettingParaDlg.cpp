// SettingParaDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Checker5678.h"
#include "SettingParaDlg.h"
#include "afxdialogex.h"


// SettingParaDlg �Ի���

IMPLEMENT_DYNAMIC(SettingParaDlg, CDialogEx)

SettingParaDlg::SettingParaDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG3, pParent)
	, para1(0)
	, para2(0)
	, para3(0)
	, para4(0)
{

}

SettingParaDlg::~SettingParaDlg()
{
}

void SettingParaDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT2, para1);
	DDX_Text(pDX, IDC_EDIT3, para2);
	DDX_Text(pDX, IDC_EDIT17, para3);
	DDX_Text(pDX, IDC_EDIT18, para4);
	DDX_Text(pDX, IDC_EDIT19, para5);
	DDX_Text(pDX, IDC_EDIT20, para6);
	DDX_Text(pDX, IDC_EDIT21, para7);
}


BEGIN_MESSAGE_MAP(SettingParaDlg, CDialogEx)
END_MESSAGE_MAP()


// SettingParaDlg ��Ϣ�������
