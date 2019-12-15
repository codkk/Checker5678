
// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // �� Windows ͷ���ų�����ʹ�õ�����
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // ĳЩ CString ���캯��������ʽ��

// �ر� MFC ��ĳЩ�����������ɷ��ĺ��Եľ�����Ϣ������
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC ��������ͱ�׼���
#include <afxext.h>         // MFC ��չ


#include <afxdisp.h>        // MFC �Զ�����



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC �� Internet Explorer 4 �����ؼ���֧��
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC �� Windows �����ؼ���֧��
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // �������Ϳؼ����� MFC ֧��



#include "myCameraClass.h"
#include "ImProcChecker.h"
#include "SerialPort.h"
#include "sc_io.h"
#ifdef _DEBUG
#pragma comment(lib,"scio64.lib")
#else
#pragma comment(lib,"scio64.lib")
#endif


#import "C:\Program Files\Common Files\System\ado\msado15.dll" no_namespace rename("EOF", "adoEOF") rename("BOF", "adoBOF")


//3. IO״̬����
#define IO_VALID 0   //��Ч��ƽ
#define IO_INVALID 1 //��Ч��ƽ

#define IO_OUT_ON   0 //���on
#define IO_OUT_OFF  1 //���off

//4. ���빦�ܶ���
#define IN_CHECKER_BT1 0//���԰�ť1
#define IN_CHECKER_BT2 1//���԰�ť1
#define IN_SENS_SUKER 2 //��������
#define IN_SENS_WEIGHT 3 //����
#define IN_BOTTON_WEIGHT 4 //����λ��ť
#define IN_BUTTON_CCD 5 //ccd��λ��ť
#define IN_SENS_SCAN 6 //ɨ��ǹ

//5. ������ܶ���
#define OUT_LIGHT_CAM 6  //�����Դ
#define OUT_SUKER 5  //����
#define OUT_LIGHT_WEIGHT_OK 3 //����ok��
#define OUT_LIGHT_WEIGHT_NG 4 //����ok��
#define OUT_LIGHT_BARCODE_OK 0//������ 
#define OUT_LIGHT_ACC_OK 3//�����ȫ���
#define OUT_LIGHT_COVERY_OK 7//����Ĥ���
#define OUT_LIGHT_RED 5
#define OUT_LIGHT_GREEN 6
#define OUT_LIGHT_BLUE 7

//6. ģ��·����
#define PATH_MODEL_LOGO "Task\\logo.shm"
#define PATH_MODEL_TRI "Task\\cardtri.shm"
#define PATH_MODEL_STOP "Task\\stop.shm"

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


