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
	double AngleStart;  //起始角度
	double AngleExtent; //角度范围
	double MinScore;	//最小分数 匹配度（0到1之间）
	int    NumMatches;  //匹配个数（0代表尽可能多的匹配)
	double MaxOverlap;  //最大重叠
	string SubPixel;    //亚像素
	int	   ModelLevels; //模板金字塔级
	int    NumLevels;   //金字塔级
	double Greediness;  //贪婪算法
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
//检测区域
typedef struct DetectArea {
	bool bAvalid;  //是否有效
	int row1;	   //左上角坐标
	int col1;
	int row2;	   //右下角坐标
	int col2;
	DetectArea() {
		bAvalid = false;
		row1 = -1;
		col1 = -1;
		row2 = -1;
		col2 = -1;
	}
}DetectArea;

//参数格式
typedef struct CheckPara {
	//检测区域
	DetectArea decArea;
	DetectArea refArea;
	//通用参数个数
	int para_num;
	//通用参数
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
//结果结构体
typedef struct CheckResult {
	string code;
	//坐标
	double x;
	double y;
	double angle;
	//分数
	double score;
	//面积
	double area;
	//灰度值
	float gray;
	//判定
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
	//绑定相机
	static void SetCam(myCameraClass* pCam);
	//设置显示窗口
	static bool SetDispWnd(Hlong wnd, CRect showRect);
	//
	static bool DrawRect(DetectArea& showRect);
	static bool ClearRect();
	//设置检测类型
	void SetType(int type);
	//加载模板文件
	BOOL LoadModel(char * pModelPath);
	//加载可变形模板
	BOOL LoadDeformModel(char * pModelPath);
	//加载参数文件
	BOOL LoadPara(wchar_t * pParaPath);
	//保存参数文件
	BOOL SavePara(wchar_t * pParaPath);

	//获取图像
	BOOL GrabImageFromCam(HObject &Img);
	//从文件中加载图像
	BOOL GrabImageFromFile(char* path, HObject &Img);
	//检查1 (轮廓匹配）需要加载模板文件
	BOOL CheckShape(HObject &Img);
	//检测1.2 （可形变的轮廓匹配）
	BOOL CheckDeformShape(HObject &Img);
	//检查2 （BLOB分析）需要加载参数文件
	BOOL CheckBlob(HObject &Img);
	//检查3 （灰度检测） 需要加载参数文件
	BOOL CheckGray(HObject &Img);
	//检查4 （条形码）
	BOOL CheckBarcode(HObject &Img);

	//测量孔径 形状1 USB口
	BOOL MeasureUSB();
	BOOL MeasureCircle();
	//获得结果
	BOOL GetReult(CheckResult &res);
	BOOL GetReult(MeasureResult &res);
	//获取图像并显示
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

	//界面显示
	static Hlong	hl_wnd;//控件句柄
	CRect			m_rect;//控件矩形
	static HTuple	WindowHandle;//窗口句柄，这里用于显示
	static int				m_TextRow;
	static int				m_TextCol;
	static HObject  g_hoContoursAll; //保存
	//模板
	HTuple  m_hvModelID;       //ID
	HObject  m_hoModelContours; //轮廓
	bool m_bLoadModel;
	HObject m_hoShape;
	//可形变的模板
	HTuple  m_hvDeformModelID;			//ID
	HObject  m_hoDeformModelContours;	//轮廓
	bool m_bLoadDeformModel;
	HObject m_hoDeformShape;
	//图像
	static HObject  ho_Image;
	bool m_bRoi;

	//检测参数
	int m_type;  //类型
	CheckPara m_Para;
	//
	HObject m_ho_DetectArea;
	HObject m_ho_ReferArea;//参考平面区域
	//一维码
	HTuple m_hvBarCodeHandle;
	static HObject g_hoBarCodeArea; //条形码区域
	static HObject g_hoBlobArea;  //blob区域
	static HObject g_hoShapeArea; //轮廓匹配区域
	//检测结果
	CheckResult m_Result;
	MeasureResult m_Result_measure;
};

