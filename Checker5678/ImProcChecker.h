#pragma once
#include "stdafx.h"
# include "HalconCpp.h"
# include "HDevThread.h"
#include <string>
#include <vector>
using namespace std;
using namespace HalconCpp;

#define PATH_MODEL1 "Task\\logo.shm"
#define PATH_MODEL2 "Task\\cardtitle.shm"
#define PATH_MODEL3 "Task\\cardtri.shm"
#define PATH_MODEL7 "Task\\stop.shm"
#define PATH_MODEL_LOGO_DFM "Task\\logo.dfm"
#define PATH_MODEL_TRI_DFM "Task\\cardtri.dfm"


#define PATH_PARA_LOGO _T("Task\\logo.ini")
#define PATH_PARA_CARDTITLE _T("Task\\cardtitle.ini")
#define PATH_PARA_CARDTRI _T("Task\\cardtri.ini")
#define PATH_PARA_DRY _T("Task\\dry.ini")
#define PATH_PARA_COVER _T("Task\\cover.ini")
#define PATH_PARA_CODE _T("Task\\barcode.ini")
#define PATH_PARA_STOP _T("Task\\stop.ini")
#define PATH_PARA_MEASURE_USB _T("Task\\usb.ini")
#define PATH_PARA_CIRCLE _T("Task\\circle.ini")
class myCameraClass;
typedef struct xPara {
	double AngleStart;  //��ʼ�Ƕ�
	double AngleExtent; //�Ƕȷ�Χ
	double MinScore;	//��С���� ƥ��ȣ�0��1֮�䣩
	int    NumMatches;  //ƥ�������0�������ܶ��ƥ��)
	double MaxOverlap;  //����ص�
	string SubPixel;    //������
	int	   ModelLevels; //ģ���������
	int    NumLevels;   //��������
	double Greediness;  //̰���㷨
	double MinScale;
	double MaxScale;
	xPara() {
		AngleStart = 0;
		AngleExtent = 360;
		MinScore = 0.5;
		NumMatches = 1;
		MaxOverlap = 0.5;
		SubPixel = "least_squares";
		ModelLevels = 8;
		NumLevels = 1;
		Greediness = 0.75;
		MinScale = 0.6;
		MaxScale = 1.2;
	}
}XPARA;
//�������
typedef struct DetectArea {
	bool bAvalid;  //�Ƿ���Ч
	int row1;	   //���Ͻ�����
	int col1;
	int row2;	   //���½�����
	int col2;
	DetectArea() {
		bAvalid = false;
		row1 = -1;
		col1 = -1;
		row2 = -1;
		col2 = -1;
	}
}DetectArea;

//������ʽ
typedef struct CheckPara {
	//�������
	DetectArea decArea;
	DetectArea refArea;
	//ͨ�ò�������
	int para_num;
	//ͨ�ò���
	double paraN[20];
	//
	CheckPara() {
		para_num = 0;
		for (int i = 0; i < 20; i++)
		{
			paraN[i] = 0.0;
		}
	}


}CheckPara;
//����ṹ��
typedef struct CheckResult {
	string code;
	//����
	double x;
	double y;
	double angle;
	//����
	double score;
	//���
	double area;
	//�Ҷ�ֵ
	float gray;
	//�ж�
	BOOL isOK;
	CheckResult() {
		x = 0.0;
		y = 0.0;
		angle = 0.0;
		score = 0.0;
		area = 0.0;
		gray = 0.0;
		isOK = FALSE;
	}

}CheckResult;

struct MeasureResult {
	/*
		hv_isError, hv_ErrorCode, hv_length,
		hv_width, hv_angle, hv_center_x, hv_center_y,
		hv_dis_rr, hv_up_margin, hv_down_margin;
	*/
	bool isOK;
	int errorCode;
	double length;
	double width;
	double angle;
	double cent_x;
	double cent_y;
	double dis_rr;
	double up_margin;
	double dn_margin;
	MeasureResult() {
		isOK = false;
		errorCode = 0;
		length = 0.0;
		width = 0.0;
		angle = 0.0;
		cent_x = 0.0;
		cent_y = 0.0;
		dis_rr = 0.0;
		up_margin = 0.0;
		dn_margin = 0.0;
	}
};


class ImProcChecker
{
public:
	ImProcChecker();
	~ImProcChecker();
	//�����
	static void SetCam(myCameraClass* pCam);
	//������ʾ����
	static bool SetDispWnd(Hlong wnd, CRect showRect);
	//
	static bool DrawRect(DetectArea& showRect);
	static bool ClearRect();
	//���ü������
	void SetType(int type);
	//����ģ���ļ�
	BOOL LoadModel(char * pModelPath);
	//���ؿɱ���ģ��
	BOOL LoadDeformModel(char * pModelPath);
	//���ز����ļ�
	BOOL LoadPara(wchar_t * pParaPath);
	//��������ļ�
	BOOL SavePara(wchar_t * pParaPath);

	//��ȡͼ��
	BOOL GrabImageFromCam(HObject &Img);
	//���ļ��м���ͼ��
	BOOL GrabImageFromFile(char* path, HObject &Img);
	//���1 (����ƥ�䣩��Ҫ����ģ���ļ�
	BOOL CheckShape(HObject &Img);
	//���1.2 �����α������ƥ�䣩
	BOOL CheckDeformShape(HObject &Img);
	//���2 ��BLOB��������Ҫ���ز����ļ�
	BOOL CheckBlob(HObject &Img);
	//���3 ���Ҷȼ�⣩ ��Ҫ���ز����ļ�
	BOOL CheckGray(HObject &Img);
	//���4 �������룩
	BOOL CheckBarcode(HObject &Img);

	//�����׾� ��״1 USB��
	BOOL MeasureUSB();
	BOOL MeasureCircle();
	//��ý��
	BOOL GetReult(CheckResult &res);
	BOOL GetReult(MeasureResult &res);
	//��ȡͼ����ʾ
	void disp_message(HTuple hv_WindowHandle, HTuple hv_String, HTuple hv_CoordSystem, HTuple hv_Row, HTuple hv_Column, HTuple hv_Color, HTuple hv_Box);
	static void ShowException(HalconCpp::HException & except);
	static void ContoursAll();
	bool GrabOneAndShow();
	bool SaveImage(char* path);
	bool Show();
	int	 ConvertoHalcon(unsigned char * pData, MV_FRAME_OUT_INFO_EX stImageInfo, HalconCpp::HObject& image);
public:
	static myCameraClass * m_pCam;
	unsigned char *m_pImageBuffer;
	unsigned char *m_pDataSeparate;

	//������ʾ
	static Hlong	hl_wnd;//�ؼ����
	CRect			m_rect;//�ؼ�����
	static HTuple	WindowHandle;//���ھ��������������ʾ
	static int				m_TextRow;
	static int				m_TextCol;
	static HObject  g_hoContoursAll; //����
	//ģ��
	HTuple  m_hvModelID;       //ID
	HObject  m_hoModelContours; //����
	bool m_bLoadModel;
	HObject m_hoShape;
	//���α��ģ��
	HTuple  m_hvDeformModelID;			//ID
	HObject  m_hoDeformModelContours;	//����
	bool m_bLoadDeformModel;
	HObject m_hoDeformShape;
	//ͼ��
	static HObject  ho_Image;
	bool m_bRoi;

	//������
	int m_type;  //����
	CheckPara m_Para;
	//
	HObject m_ho_DetectArea;
	HObject m_ho_ReferArea;//�ο�ƽ������
	//һά��
	HTuple m_hvBarCodeHandle;
	static HObject g_hoBarCodeArea; //����������
	static HObject g_hoBlobArea;  //blob����
	static HObject g_hoShapeArea; //����ƥ������
	//�����
	CheckResult m_Result;
	MeasureResult m_Result_measure;
};

