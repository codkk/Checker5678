
//***************************************************
//date: 2018.07.30
//function: �������ȶԻ���
//author: wuhanquan
//***************************************************

#pragma once

 class AFX_CLASS_EXPORT CProgressCtrlThread : public CWinThread
{
public:
	CProgressCtrlThread();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
	DECLARE_DYNCREATE(CProgressCtrlThread)
protected:
	void OnThreadMessages(WPARAM wParam, LPARAM lParam);
};

