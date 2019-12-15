
//***************************************************
//date: 2018.07.30
//function: 弹出进度对话框
//author: wuhanquan
//***************************************************

#pragma once

 class AFX_CLASS_EXPORT CProgressCtrlThread : public CWinThread
{
public:
	CProgressCtrlThread();

// 重写
public:
	virtual BOOL InitInstance();

// 实现

	DECLARE_MESSAGE_MAP()
	DECLARE_DYNCREATE(CProgressCtrlThread)
protected:
	void OnThreadMessages(WPARAM wParam, LPARAM lParam);
};

