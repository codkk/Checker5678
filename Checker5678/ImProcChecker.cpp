#include "stdafx.h"
#include "ImProcChecker.h"
# include "ConvertPixel.h"
#define IMG_W 4024
#define IMG_H 3036

Hlong   ImProcChecker::hl_wnd = 0;
HTuple	ImProcChecker::WindowHandle = 0;
myCameraClass* ImProcChecker::m_pCam = NULL;
int ImProcChecker::m_TextRow = 10;
int ImProcChecker::m_TextCol = 10;
HObject ImProcChecker::ho_Image;
HObject ImProcChecker::g_hoContoursAll;

HObject ImProcChecker::g_hoBarCodeArea;
HObject ImProcChecker::g_hoBlobArea;  //blob区域
HObject ImProcChecker::g_hoShapeArea; //轮廓匹配区域

CCriticalSection g_criSection;  //锁

/*

*/
// Short Description: Creates an arrow shaped XLD contour. 
void gen_arrow_contour_xld(HObject *ho_Arrow, HTuple hv_Row1, HTuple hv_Column1,
	HTuple hv_Row2, HTuple hv_Column2, HTuple hv_HeadLength, HTuple hv_HeadWidth);
// Local procedures 
void DrawIndica(HTuple hv_x1, HTuple hv_y1, HTuple hv_x2, HTuple hv_y2, HTuple hv_phi,
	HTuple hv_margin, HTuple hv_value, HTuple hv_windowhandle, HTuple hv_dir);
void DoubleCircle(HObject ho_imagesrc, HObject ho_regionsrc, HObject ho_regionref,
	HTuple hv_deviation_value1, HTuple hv_deviation_value2, HTuple hv_factor, HTuple hv_half_margin,
	HTuple hv_WindowHandle, HTuple *hv_isError, HTuple *hv_ErrorCode, HTuple *hv_length,
	HTuple *hv_width, HTuple *hv_angle, HTuple *hv_center_x, HTuple *hv_center_y,
	HTuple *hv_dis_rr, HTuple *hv_up_margin, HTuple *hv_down_margin);
void SingleCircle(HObject ho_imagesrc, HObject ho_regionsrc, HObject ho_regionref,
	HTuple hv_deviation_value1, HTuple hv_deviation_value2, HTuple hv_factor, HTuple hv_WindowHandle,
	HTuple *hv_radius, HTuple *hv_center_x, HTuple *hv_center_y, HTuple *hv_isError,
	HTuple *hv_ErrorCode);
void DoubleCircle2(HObject ho_imagesrc, HObject ho_regionsrc, HObject ho_regionref,
	HTuple hv_deviation_value1, HTuple hv_deviation_value2, HTuple hv_deviation_value3,
	HTuple hv_factor, HTuple hv_half_margin, HTuple hv_WindowHandle, HTuple hv_thr,
	HTuple hv_sigma, HTuple *hv_isError, HTuple *hv_ErrorCode, HTuple *hv_length,
	HTuple *hv_width, HTuple *hv_angle, HTuple *hv_center_x, HTuple *hv_center_y,
	HTuple *hv_dis_rr, HTuple *hv_up_margin, HTuple *hv_down_margin);
//////////////////////////////////////////////////////////
// Short Description: This procedure writes a text message. 
void disp_message(HTuple hv_WindowHandle, HTuple hv_String, HTuple hv_CoordSystem,
	HTuple hv_Row, HTuple hv_Column, HTuple hv_Color, HTuple hv_Box)
{

	// Local iconic variables

	// Local control variables
	HTuple  hv_Red, hv_Green, hv_Blue, hv_Row1Part;
	HTuple  hv_Column1Part, hv_Row2Part, hv_Column2Part, hv_RowWin;
	HTuple  hv_ColumnWin, hv_WidthWin, hv_HeightWin, hv_MaxAscent;
	HTuple  hv_MaxDescent, hv_MaxWidth, hv_MaxHeight, hv_R1;
	HTuple  hv_C1, hv_FactorRow, hv_FactorColumn, hv_UseShadow;
	HTuple  hv_ShadowColor, hv_Exception, hv_Width, hv_Index;
	HTuple  hv_Ascent, hv_Descent, hv_W, hv_H, hv_FrameHeight;
	HTuple  hv_FrameWidth, hv_R2, hv_C2, hv_DrawMode, hv_CurrentColor;

	//This procedure displays text in a graphics window.
	//
	//Input parameters:
	//WindowHandle: The WindowHandle of the graphics window, where
	//   the message should be displayed
	//String: A tuple of strings containing the text message to be displayed
	//CoordSystem: If set to 'window', the text position is given
	//   with respect to the window coordinate system.
	//   If set to 'image', image coordinates are used.
	//   (This may be useful in zoomed images.)
	//Row: The row coordinate of the desired text position
	//   If set to -1, a default value of 12 is used.
	//Column: The column coordinate of the desired text position
	//   If set to -1, a default value of 12 is used.
	//Color: defines the color of the text as string.
	//   If set to [], '' or 'auto' the currently set color is used.
	//   If a tuple of strings is passed, the colors are used cyclically
	//   for each new textline.
	//Box: If Box[0] is set to 'true', the text is written within an orange box.
	//     If set to' false', no box is displayed.
	//     If set to a color string (e.g. 'white', '#FF00CC', etc.),
	//       the text is written in a box of that color.
	//     An optional second value for Box (Box[1]) controls if a shadow is displayed:
	//       'true' -> display a shadow in a default color
	//       'false' -> display no shadow (same as if no second value is given)
	//       otherwise -> use given string as color string for the shadow color
	//
	//Prepare window
	GetRgb(hv_WindowHandle, &hv_Red, &hv_Green, &hv_Blue);
	GetPart(hv_WindowHandle, &hv_Row1Part, &hv_Column1Part, &hv_Row2Part, &hv_Column2Part);
	GetWindowExtents(hv_WindowHandle, &hv_RowWin, &hv_ColumnWin, &hv_WidthWin, &hv_HeightWin);
	SetPart(hv_WindowHandle, 0, 0, hv_HeightWin - 1, hv_WidthWin - 1);
	//
	//default settings
	if (0 != (hv_Row == -1))
	{
		hv_Row = 12;
	}
	if (0 != (hv_Column == -1))
	{
		hv_Column = 12;
	}
	if (0 != (hv_Color == HTuple()))
	{
		hv_Color = "";
	}
	//
	hv_String = (("" + hv_String) + "").TupleSplit("\n");
	//
	//Estimate extentions of text depending on font size.
	GetFontExtents(hv_WindowHandle, &hv_MaxAscent, &hv_MaxDescent, &hv_MaxWidth, &hv_MaxHeight);
	if (0 != (hv_CoordSystem == HTuple("window")))
	{
		hv_R1 = hv_Row;
		hv_C1 = hv_Column;
	}
	else
	{
		//Transform image to window coordinates
		hv_FactorRow = (1.*hv_HeightWin) / ((hv_Row2Part - hv_Row1Part) + 1);
		hv_FactorColumn = (1.*hv_WidthWin) / ((hv_Column2Part - hv_Column1Part) + 1);
		hv_R1 = ((hv_Row - hv_Row1Part) + 0.5)*hv_FactorRow;
		hv_C1 = ((hv_Column - hv_Column1Part) + 0.5)*hv_FactorColumn;
	}
	//
	//Display text box depending on text size
	hv_UseShadow = 1;
	hv_ShadowColor = "gray";
	if (0 != (HTuple(hv_Box[0]) == HTuple("true")))
	{
		hv_Box[0] = "#fce9d4";
		hv_ShadowColor = "#f28d26";
	}
	if (0 != ((hv_Box.TupleLength())>1))
	{
		if (0 != (HTuple(hv_Box[1]) == HTuple("true")))
		{
			//Use default ShadowColor set above
		}
		else if (0 != (HTuple(hv_Box[1]) == HTuple("false")))
		{
			hv_UseShadow = 0;
		}
		else
		{
			hv_ShadowColor = ((const HTuple&)hv_Box)[1];
			//Valid color?
			try
			{
				SetColor(hv_WindowHandle, HTuple(hv_Box[1]));
			}
			// catch (Exception) 
			catch (HalconCpp::HException &HDevExpDefaultException)
			{
				HDevExpDefaultException.ToHTuple(&hv_Exception);
				hv_Exception = "Wrong value of control parameter Box[1] (must be a 'true', 'false', or a valid color string)";
				throw HalconCpp::HException(hv_Exception);
			}
		}
	}
	if (0 != (HTuple(hv_Box[0]) != HTuple("false")))
	{
		//Valid color?
		try
		{
			SetColor(hv_WindowHandle, HTuple(hv_Box[0]));
		}
		// catch (Exception) 
		catch (HalconCpp::HException &HDevExpDefaultException)
		{
			HDevExpDefaultException.ToHTuple(&hv_Exception);
			hv_Exception = "Wrong value of control parameter Box[0] (must be a 'true', 'false', or a valid color string)";
			throw HalconCpp::HException(hv_Exception);
		}
		//Calculate box extents
		hv_String = (" " + hv_String) + " ";
		hv_Width = HTuple();
		{
			HTuple end_val93 = (hv_String.TupleLength()) - 1;
			HTuple step_val93 = 1;
			for (hv_Index = 0; hv_Index.Continue(end_val93, step_val93); hv_Index += step_val93)
			{
				GetStringExtents(hv_WindowHandle, HTuple(hv_String[hv_Index]), &hv_Ascent,
					&hv_Descent, &hv_W, &hv_H);
				hv_Width = hv_Width.TupleConcat(hv_W);
			}
		}
		hv_FrameHeight = hv_MaxHeight*(hv_String.TupleLength());
		hv_FrameWidth = (HTuple(0).TupleConcat(hv_Width)).TupleMax();
		hv_R2 = hv_R1 + hv_FrameHeight;
		hv_C2 = hv_C1 + hv_FrameWidth;
		//Display rectangles
		GetDraw(hv_WindowHandle, &hv_DrawMode);
		SetDraw(hv_WindowHandle, "fill");
		//Set shadow color
		SetColor(hv_WindowHandle, hv_ShadowColor);
		if (0 != hv_UseShadow)
		{
			DispRectangle1(hv_WindowHandle, hv_R1 + 1, hv_C1 + 1, hv_R2 + 1, hv_C2 + 1);
		}
		//Set box color
		SetColor(hv_WindowHandle, HTuple(hv_Box[0]));
		DispRectangle1(hv_WindowHandle, hv_R1, hv_C1, hv_R2, hv_C2);
		SetDraw(hv_WindowHandle, hv_DrawMode);
	}
	//Write text.
	{
		HTuple end_val115 = (hv_String.TupleLength()) - 1;
		HTuple step_val115 = 1;
		for (hv_Index = 0; hv_Index.Continue(end_val115, step_val115); hv_Index += step_val115)
		{
			hv_CurrentColor = ((const HTuple&)hv_Color)[hv_Index % (hv_Color.TupleLength())];
			if (0 != (HTuple(hv_CurrentColor != HTuple("")).TupleAnd(hv_CurrentColor != HTuple("auto"))))
			{
				SetColor(hv_WindowHandle, hv_CurrentColor);
			}
			else
			{
				SetRgb(hv_WindowHandle, hv_Red, hv_Green, hv_Blue);
			}
			hv_Row = hv_R1 + (hv_MaxHeight*hv_Index);
			SetTposition(hv_WindowHandle, hv_Row, hv_C1);
			WriteString(hv_WindowHandle, HTuple(hv_String[hv_Index]));
		}
	}
	//Reset changed window settings
	SetRgb(hv_WindowHandle, hv_Red, hv_Green, hv_Blue);
	SetPart(hv_WindowHandle, hv_Row1Part, hv_Column1Part, hv_Row2Part, hv_Column2Part);
	return;
}

// Short Description: Creates an arrow shaped XLD contour. 
void gen_arrow_contour_xld(HObject *ho_Arrow, HTuple hv_Row1, HTuple hv_Column1,
	HTuple hv_Row2, HTuple hv_Column2, HTuple hv_HeadLength, HTuple hv_HeadWidth)
{

	// Local iconic variables
	HObject  ho_TempArrow;

	// Local control variables
	HTuple  hv_Length, hv_ZeroLengthIndices, hv_DR;
	HTuple  hv_DC, hv_HalfHeadWidth, hv_RowP1, hv_ColP1, hv_RowP2;
	HTuple  hv_ColP2, hv_Index;

	//This procedure generates arrow shaped XLD contours,
	//pointing from (Row1, Column1) to (Row2, Column2).
	//If starting and end point are identical, a contour consisting
	//of a single point is returned.
	//
	//input parameteres:
	//Row1, Column1: Coordinates of the arrows' starting points
	//Row2, Column2: Coordinates of the arrows' end points
	//HeadLength, HeadWidth: Size of the arrow heads in pixels
	//
	//output parameter:
	//Arrow: The resulting XLD contour
	//
	//The input tuples Row1, Column1, Row2, and Column2 have to be of
	//the same length.
	//HeadLength and HeadWidth either have to be of the same length as
	//Row1, Column1, Row2, and Column2 or have to be a single element.
	//If one of the above restrictions is violated, an error will occur.
	//
	//
	//Init
	GenEmptyObj(&(*ho_Arrow));
	//
	//Calculate the arrow length
	DistancePp(hv_Row1, hv_Column1, hv_Row2, hv_Column2, &hv_Length);
	//
	//Mark arrows with identical start and end point
	//(set Length to -1 to avoid division-by-zero exception)
	hv_ZeroLengthIndices = hv_Length.TupleFind(0);
	if (0 != (hv_ZeroLengthIndices != -1))
	{
		hv_Length[hv_ZeroLengthIndices] = -1;
	}
	//
	//Calculate auxiliary variables.
	hv_DR = (1.0*(hv_Row2 - hv_Row1)) / hv_Length;
	hv_DC = (1.0*(hv_Column2 - hv_Column1)) / hv_Length;
	hv_HalfHeadWidth = hv_HeadWidth / 2.0;
	//
	//Calculate end points of the arrow head.
	hv_RowP1 = (hv_Row1 + ((hv_Length - hv_HeadLength)*hv_DR)) + (hv_HalfHeadWidth*hv_DC);
	hv_ColP1 = (hv_Column1 + ((hv_Length - hv_HeadLength)*hv_DC)) - (hv_HalfHeadWidth*hv_DR);
	hv_RowP2 = (hv_Row1 + ((hv_Length - hv_HeadLength)*hv_DR)) - (hv_HalfHeadWidth*hv_DC);
	hv_ColP2 = (hv_Column1 + ((hv_Length - hv_HeadLength)*hv_DC)) + (hv_HalfHeadWidth*hv_DR);
	//
	//Finally create output XLD contour for each input point pair
	{
		HTuple end_val45 = (hv_Length.TupleLength()) - 1;
		HTuple step_val45 = 1;
		for (hv_Index = 0; hv_Index.Continue(end_val45, step_val45); hv_Index += step_val45)
		{
			if (0 != (HTuple(hv_Length[hv_Index]) == -1))
			{
				//Create_ single points for arrows with identical start and end point
				GenContourPolygonXld(&ho_TempArrow, HTuple(hv_Row1[hv_Index]), HTuple(hv_Column1[hv_Index]));
			}
			else
			{
				//Create arrow contour
				GenContourPolygonXld(&ho_TempArrow, ((((HTuple(hv_Row1[hv_Index]).TupleConcat(HTuple(hv_Row2[hv_Index]))).TupleConcat(HTuple(hv_RowP1[hv_Index]))).TupleConcat(HTuple(hv_Row2[hv_Index]))).TupleConcat(HTuple(hv_RowP2[hv_Index]))).TupleConcat(HTuple(hv_Row2[hv_Index])),
					((((HTuple(hv_Column1[hv_Index]).TupleConcat(HTuple(hv_Column2[hv_Index]))).TupleConcat(HTuple(hv_ColP1[hv_Index]))).TupleConcat(HTuple(hv_Column2[hv_Index]))).TupleConcat(HTuple(hv_ColP2[hv_Index]))).TupleConcat(HTuple(hv_Column2[hv_Index])));
			}
			ConcatObj((*ho_Arrow), ho_TempArrow, &(*ho_Arrow));
		}
	}
	return;
}

// Local procedures 
void DrawIndica(HTuple hv_x1, HTuple hv_y1, HTuple hv_x2, HTuple hv_y2, HTuple hv_phi,
	HTuple hv_margin, HTuple hv_value, HTuple hv_windowhandle, HTuple hv_dir)
{

	// Local iconic variables
	HObject  ho_Cross1, ho_Cross2, ho_Cross3, ho_Cross4;
	HObject  ho_RegionLines, ho_RegionLines1, ho_RegionLines0;
	HObject  ho_Arrow, ho_Arrow2;

	// Local control variables
	HTuple  hv_HomMat2DIdentity, hv_p, hv_HomMat2DRotate2;
	HTuple  hv_Qx1, hv_Qy1, hv_Qx2, hv_Qy2, hv_g;

	//if (HDevWindowStack::IsOpen())
	SetColor(hv_windowhandle, "green");
	HomMat2dIdentity(&hv_HomMat2DIdentity);
	hv_p = (HTuple(90).TupleRad()) + hv_phi;
	if (0 != (hv_dir == 0))
	{
		HomMat2dRotate(hv_HomMat2DIdentity, -hv_p, hv_x1, hv_y1, &hv_HomMat2DRotate2);
		AffineTransPoint2d(hv_HomMat2DRotate2, hv_x1 + hv_margin, hv_y1, &hv_Qx1, &hv_Qy1);
		GenCrossContourXld(&ho_Cross1, hv_y1, hv_x1, 36, hv_phi);
		//if (HDevWindowStack::IsOpen())
		DispObj(ho_Cross1, hv_windowhandle);

		HomMat2dRotate(hv_HomMat2DIdentity, (-(HTuple(90).TupleRad())) - hv_phi, hv_x2,
			hv_y2, &hv_HomMat2DRotate2);
		AffineTransPoint2d(hv_HomMat2DRotate2, hv_x2 + hv_margin, hv_y2, &hv_Qx2, &hv_Qy2);
		GenCrossContourXld(&ho_Cross2, hv_y2, hv_x2, 36, hv_phi);
		//if (HDevWindowStack::IsOpen())
		DispObj(ho_Cross2, hv_windowhandle);
	}
	else
	{
		HomMat2dRotate(hv_HomMat2DIdentity, -hv_p, hv_x1, hv_y1, &hv_HomMat2DRotate2);
		AffineTransPoint2d(hv_HomMat2DRotate2, hv_x1, hv_y1 + hv_margin, &hv_Qx1, &hv_Qy1);
		GenCrossContourXld(&ho_Cross3, hv_y1, hv_x1, 36, hv_phi);
		//if (HDevWindowStack::IsOpen())
		DispObj(ho_Cross3, hv_windowhandle);

		HomMat2dRotate(hv_HomMat2DIdentity, (-(HTuple(90).TupleRad())) - hv_phi, hv_x2,
			hv_y2, &hv_HomMat2DRotate2);
		AffineTransPoint2d(hv_HomMat2DRotate2, hv_x2, hv_y2 + hv_margin, &hv_Qx2, &hv_Qy2);
		GenCrossContourXld(&ho_Cross4, hv_y2, hv_x2, 36, hv_phi);
		//if (HDevWindowStack::IsOpen())
		DispObj(ho_Cross4, hv_windowhandle);
	}


	GenRegionLine(&ho_RegionLines, hv_Qy2, hv_Qx2, hv_y2, hv_x2);
	GenRegionLine(&ho_RegionLines1, hv_Qy1, hv_Qx1, hv_y1, hv_x1);
	GenRegionLine(&ho_RegionLines0, hv_y2, hv_x2, hv_y1, hv_x1);
	gen_arrow_contour_xld(&ho_Arrow, hv_Qy2, hv_Qx2, hv_Qy1, hv_Qx1, 25, 25);
	gen_arrow_contour_xld(&ho_Arrow2, hv_Qy1, hv_Qx1, hv_Qy2, hv_Qx2, 25, 25);

	DispObj(ho_Arrow, hv_windowhandle);

	DispObj(ho_Arrow2, hv_windowhandle);

	DispObj(ho_RegionLines, hv_windowhandle);

	DispObj(ho_RegionLines1, hv_windowhandle);

	DispObj(ho_RegionLines0, hv_windowhandle);
	SetTposition(hv_windowhandle, (hv_Qy1 + hv_Qy2) / 2.0, (hv_Qx1 + hv_Qx2) / 2.0);
	hv_g = hv_value.TupleString(".4f");
	WriteString(hv_windowhandle, hv_g);
	return;
}

void DoubleCircle(HObject ho_imagesrc, HObject ho_regionsrc, HObject ho_regionref,
	HTuple hv_deviation_value1, HTuple hv_deviation_value2, HTuple hv_factor, HTuple hv_half_margin,
	HTuple hv_WindowHandle, HTuple *hv_isError, HTuple *hv_ErrorCode, HTuple *hv_length,
	HTuple *hv_width, HTuple *hv_angle, HTuple *hv_center_x, HTuple *hv_center_y,
	HTuple *hv_dis_rr, HTuple *hv_up_margin, HTuple *hv_down_margin)
{

	// Local iconic variables
	HObject  ho_ImageReduced, ho_Union, ho_RegionClosing;
	HObject  ho_RegionFillUp, ho_RegionErosion, ho_RegionDilation;
	HObject  ho_ConnectedRegions, ho_SelectedRegions, ho_ObjectSelected1;
	HObject  ho_RegionTrans, ho_RegionTrans1, ho_ImageReduced1;
	HObject  ho_Cross, ho_Cross1, ho_Cross2, ho_RegionLines;
	HObject  ho_Contours, ho_RectangleLeft, ho_Rectangle, ho_ClippedContours;
	HObject  ho_ContCircle, ho_Cross3, ho_Rectangle1, ho_ClippedContours1;
	HObject  ho_ContCircle1, ho_Cross9, ho_CrossUp, ho_ROI_2;
	HObject  ho_ImageReduced2, ho_Regions2, ho_RegionFillUp1;
	HObject  ho_RegionErosion1, ho_RegionDilation1, ho_ConnectedRegions1;
	HObject  ho_SelectedRegions1, ho_ObjectSelected, ho_RegionTrans2;
	HObject  ho_Cross5, ho_Cross4, ho_Cross6, ho_Cross7, ho_Cross8;

	// Local control variables
	HTuple  hv_Value, hv_gray_ref, hv_AreaRegion;
	HTuple  hv_Row5, hv_Column5, hv_Number1, hv_Row, hv_Column;
	HTuple  hv_Phi, hv_Length1, hv_Length2, hv_HomMat2DIdentity;
	HTuple  hv_HomMat2DRotate, hv_HomMat2DRotate2, hv_Qx, hv_Qy;
	HTuple  hv_Qx2, hv_Qy2, hv_Row11, hv_Column11, hv_Row2;
	HTuple  hv_Column2, hv_Row1, hv_Column1, hv_Radius, hv_StartPhi;
	HTuple  hv_EndPhi, hv_PointOrder, hv_Row12, hv_Column12;
	HTuple  hv_Row21, hv_Column21, hv_Radius2, hv_cen_x, hv_cen_y;
	HTuple  hv_HomMat0, hv_p, hv_HomMat0_R, hv_upx1, hv_upy1;
	HTuple  hv_dnx2, hv_dny2, hv_Area_big, hv_Row6, hv_Column6;
	HTuple  hv_Number2, hv_Row3, hv_Column3, hv_Phi1, hv_Length11;
	HTuple  hv_Length21, hv_HomMat2DIdentity3, hv_Qx11, hv_Qy11;
	HTuple  hv_Qx21, hv_Qy21, hv_Qx31, hv_Qy31, hv_Qx41, hv_Qy41;
	HTuple  hv_Row4, hv_Column4, hv_IsOverlapping, hv_Distance;
	HTuple  hv_Distance2, hv_Dis_pp, hv_Exception, hv_ErrorMsg;
	//HTuple  hv_dn_margin;

	(*hv_isError) = 0;
	(*hv_ErrorCode) = 0;


	try
	{
		DispObj(ho_imagesrc, hv_WindowHandle);
		//求参考平面灰度值
		GrayFeatures(ho_regionref, ho_imagesrc, "mean", &hv_Value);
		hv_gray_ref = hv_Value;
		//求出检测的区域的面积
		AreaCenter(ho_regionsrc, &hv_AreaRegion, &hv_Row5, &hv_Column5);
		//抠出目标
		//deviation_value1 := 50
		ReduceDomain(ho_imagesrc, ho_regionsrc, &ho_ImageReduced);

		Threshold(ho_ImageReduced, &ho_Union, HTuple(0).TupleConcat(hv_gray_ref + hv_deviation_value1),
			(hv_gray_ref - hv_deviation_value1).TupleConcat(255));


		Union1(ho_Union, &ho_Union);
		//预处理
		ClosingCircle(ho_Union, &ho_RegionClosing, 5.5);
		FillUp(ho_RegionClosing, &ho_RegionFillUp);
		ErosionCircle(ho_RegionFillUp, &ho_RegionErosion, 5.5);
		DilationCircle(ho_RegionErosion, &ho_RegionDilation, 5.5);
		HalconCpp::Connection(ho_RegionDilation, &ho_ConnectedRegions);
		SelectShape(ho_ConnectedRegions, &ho_SelectedRegions, "area", "and", hv_AreaRegion*(1 / 3.0),
			hv_AreaRegion);
		CountObj(ho_SelectedRegions, &hv_Number1);
		if (0 != (hv_Number1<1))
		{
			(*hv_isError) = 1;
			(*hv_ErrorCode) = 1;
			//返回1
			return;
		}
		SelectObj(ho_SelectedRegions, &ho_ObjectSelected1, 1);
		//拟合成矩形 此时Length2接近圆的直径
		ShapeTrans(ho_ObjectSelected1, &ho_RegionTrans, "convex");
		ShapeTrans(ho_RegionTrans, &ho_RegionTrans1, "rectangle2");
		ReduceDomain(ho_ImageReduced, ho_RegionTrans1, &ho_ImageReduced1);
		SmallestRectangle2(ho_RegionTrans1, &hv_Row, &hv_Column, &hv_Phi, &hv_Length1,
			&hv_Length2);
		GenCrossContourXld(&ho_Cross, hv_Row, hv_Column, 6, hv_Phi);
		//求两个最远的端点坐标
		HomMat2dIdentity(&hv_HomMat2DIdentity);
		HomMat2dRotate(hv_HomMat2DIdentity, -hv_Phi, hv_Column, hv_Row, &hv_HomMat2DRotate);
		HomMat2dRotate(hv_HomMat2DIdentity, hv_Phi, hv_Column, hv_Row, &hv_HomMat2DRotate2);
		AffineTransPoint2d(hv_HomMat2DRotate, hv_Column + hv_Length1, hv_Row, &hv_Qx, &hv_Qy);
		AffineTransPoint2d(hv_HomMat2DRotate, hv_Column - hv_Length1, hv_Row, &hv_Qx2,
			&hv_Qy2);
		//最长距离的连线
		GenCrossContourXld(&ho_Cross1, hv_Qy2, hv_Qx2, 10, hv_Phi);
		GenCrossContourXld(&ho_Cross2, hv_Qy, hv_Qx, 10, hv_Phi);
		GenRegionLine(&ho_RegionLines, hv_Qy2, hv_Qx2, hv_Qy, hv_Qx);
		
		//DispObj(ho_RegionLines, hv_WindowHandle);
		//left左圆
		GenContourRegionXld(ho_RegionTrans, &ho_Contours, "border");
		GenRectangle2(&ho_RectangleLeft, hv_Qy2, hv_Qx2, hv_Phi, hv_Length2 + 20, hv_Length2 + 20);

		SmallestRectangle1(ho_RectangleLeft, &hv_Row11, &hv_Column11, &hv_Row2, &hv_Column2);
		GenRectangle1(&ho_Rectangle, hv_Row11, hv_Column11, hv_Row2, hv_Column2);
		ClipContoursXld(ho_Contours, &ho_ClippedContours, hv_Row11, hv_Column11, hv_Row2,
			hv_Column2);
		FitCircleContourXld(ho_ClippedContours, "geotukey", -1, 2, 0, 10, 1, &hv_Row1,
			&hv_Column1, &hv_Radius, &hv_StartPhi, &hv_EndPhi, &hv_PointOrder);
		if (0 != ((hv_Row1.TupleLength()) != 1))
		{
			(*hv_isError) = 1;
			(*hv_ErrorCode) = 2;
			//返回1
			return;
		}

		GenCircleContourXld(&ho_ContCircle, hv_Row1, hv_Column1, hv_Radius, 0, 6.28318,
			"positive", 1);
		GenCrossContourXld(&ho_Cross3, hv_Row1, hv_Column1, 16, hv_Phi);
		DispObj(ho_Cross3, hv_WindowHandle);
		DispObj(ho_ContCircle, hv_WindowHandle);
		//right右圆
		GenRectangle2(&ho_RectangleLeft, hv_Qy, hv_Qx, hv_Phi, hv_Length2 + 20, hv_Length2 + 20);
		SmallestRectangle1(ho_RectangleLeft, &hv_Row12, &hv_Column12, &hv_Row21, &hv_Column21);
		GenRectangle1(&ho_Rectangle1, hv_Row12, hv_Column12, hv_Row21, hv_Column21);
		ClipContoursXld(ho_Contours, &ho_ClippedContours1, hv_Row12, hv_Column12, hv_Row21,
			hv_Column21);
		FitCircleContourXld(ho_ClippedContours1, "geotukey", -1, 2, 0, 10, 1, &hv_Row2,
			&hv_Column2, &hv_Radius2, &hv_StartPhi, &hv_EndPhi, &hv_PointOrder);
		if (0 != ((hv_Row2.TupleLength()) != 1))
		{
			(*hv_isError) = 1;
			(*hv_ErrorCode) = 3;
			//返回3
			return;
		}
		GenCircleContourXld(&ho_ContCircle1, hv_Row2, hv_Column2, hv_Radius2, 0, 6.28318,
			"positive", 1);
		GenCrossContourXld(&ho_Cross9, hv_Row2, hv_Column2, 16, hv_Phi);
		DispObj(ho_ContCircle1, hv_WindowHandle);
		DispObj(ho_Cross9, hv_WindowHandle);
		//result
		//两个圆心连线的中心center point
		hv_cen_x = (hv_Column1 + hv_Column2) / 2.0;
		hv_cen_y = (hv_Row2 + hv_Row1) / 2.0;
		//
		HomMat2dIdentity(&hv_HomMat0);
		hv_p = (HTuple(90).TupleRad()) + hv_Phi;
		HomMat2dRotate(hv_HomMat0, -hv_p, hv_Column2, hv_Row2, &hv_HomMat0_R);
		AffineTransPoint2d(hv_HomMat0_R, hv_Column2 + hv_Radius2, hv_Row2, &hv_upx1, &hv_upy1);
		GenCrossContourXld(&ho_CrossUp, hv_upy1, hv_upx1, 36, hv_Phi);

		AffineTransPoint2d(hv_HomMat0_R, hv_Column2 - hv_Radius2, hv_Row2, &hv_dnx2, &hv_dny2);
		GenCrossContourXld(&ho_CrossUp, hv_dny2, hv_dnx2, 36, hv_Phi);
		//求边缘到中轴线的距离
		GenRectangle2(&ho_ROI_2, hv_Row, hv_Column, hv_Phi, hv_Length1 + 40, hv_Length2 + hv_half_margin);
		ReduceDomain(ho_imagesrc, ho_ROI_2, &ho_ImageReduced2);
		//deviation_value2 := 10

		Threshold(ho_ImageReduced2, &ho_Regions2, hv_gray_ref - hv_deviation_value2, 255);
		//计算检测框的面积
		AreaCenter(ho_ROI_2, &hv_Area_big, &hv_Row6, &hv_Column6);
		FillUp(ho_Regions2, &ho_RegionFillUp1);
		ErosionRectangle1(ho_RegionFillUp1, &ho_RegionErosion1, 11, 11);
		DilationRectangle1(ho_RegionErosion1, &ho_RegionDilation1, 11, 11);

		HalconCpp::Connection(ho_RegionDilation1, &ho_ConnectedRegions1);
		SelectShape(ho_ConnectedRegions1, &ho_SelectedRegions1, "area", "and", hv_Area_big*(1 / 5.0),
			hv_Area_big);
		CountObj(ho_SelectedRegions1, &hv_Number2);
		if (0 != (hv_Number2<1))
		{
			(*hv_isError) = 1;
			(*hv_ErrorCode) = 3;
			//返回1
			return;
		}
		SelectObj(ho_SelectedRegions1, &ho_ObjectSelected, 1);
		ShapeTrans(ho_ObjectSelected, &ho_RegionTrans2, "rectangle2");
		SmallestRectangle2(ho_RegionTrans2, &hv_Row3, &hv_Column3, &hv_Phi1, &hv_Length11,
			&hv_Length21);
		//gen_cross_contour_xld (Cross5, Row3, Column3, 6, Phi1)

		HomMat2dIdentity(&hv_HomMat2DIdentity3);
		HomMat2dRotate(hv_HomMat2DIdentity3, -hv_Phi1, hv_Row3, hv_Column3, &hv_HomMat2DRotate2);
		AffineTransPoint2d(hv_HomMat2DRotate2, hv_Column3 + hv_Length11, hv_Row3 + hv_Length21,
			&hv_Qx11, &hv_Qy11);

		GenCrossContourXld(&ho_Cross4, hv_Qy11, hv_Qx11, 6, hv_Phi1);
		AffineTransPoint2d(hv_HomMat2DRotate2, hv_Column3 - hv_Length11, hv_Row3 + hv_Length21,
			&hv_Qx21, &hv_Qy21);
		GenCrossContourXld(&ho_Cross5, hv_Qy21, hv_Qx21, 6, hv_Phi1);
		AffineTransPoint2d(hv_HomMat2DRotate2, hv_Column3 - hv_Length11, hv_Row3 - hv_Length21,
			&hv_Qx31, &hv_Qy31);
		GenCrossContourXld(&ho_Cross6, hv_Qy31, hv_Qx31, 6, hv_Phi1);
		AffineTransPoint2d(hv_HomMat2DRotate2, hv_Column3 + hv_Length11, hv_Row3 - hv_Length21,
			&hv_Qx41, &hv_Qy41);
		GenCrossContourXld(&ho_Cross7, hv_Qy41, hv_Qx41, 6, hv_Phi1);
		//交点
		IntersectionLines(hv_Qy41, hv_Qx41, hv_Qy11, hv_Qx11, hv_Qy2, hv_Qx2, hv_Qy,
			hv_Qx, &hv_Row4, &hv_Column4, &hv_IsOverlapping);
		GenCrossContourXld(&ho_Cross8, hv_Row4, hv_Column4, 6, hv_Phi1);
		//中心线到目标线的长度
		DistancePp(hv_Row4, hv_Column4, hv_Qy41, hv_Qx41, &hv_Distance);
		DistancePp(hv_Row4, hv_Column4, hv_Qy11, hv_Qx11, &hv_Distance2);
		//求两个圆心之间的距离
		DistancePp(hv_Row2, hv_Column2, hv_Row1, hv_Column1, &hv_Dis_pp);
	}
	// catch (Exception) 
	catch (HalconCpp::HException &HDevExpDefaultException)
	{
		HDevExpDefaultException.ToHTuple(&hv_Exception);
		HalconCpp::HException::GetExceptionData(hv_Exception, "error_msg", &hv_ErrorMsg);
		disp_message(hv_WindowHandle, "Error message: " + hv_ErrorMsg, "window", 12, 12,
			"red", "true");
		(*hv_isError) = 1;
		(*hv_ErrorCode) = 4;
		//返回1
		return;
	}
	//返回值
	(*hv_dis_rr) = hv_Dis_pp*hv_factor;
	//长度
	(*hv_length) = ((hv_Dis_pp + hv_Radius2) + hv_Radius)*hv_factor;
	//宽度
	(*hv_width) = (hv_Radius2 + hv_Radius)*hv_factor;
	//角度方向（弧度制）
	(*hv_angle) = hv_Phi1*hv_factor;
	//中心点 x坐标
	(*hv_center_x) = hv_cen_x*hv_factor;
	(*hv_center_y) = hv_cen_y*hv_factor;
	//上边缘距离中轴线
	(*hv_up_margin) = hv_Distance*hv_factor;
	(*hv_down_margin) = hv_Distance2*hv_factor;

	//两个圆心之间的距离
	DrawIndica(hv_Column1, hv_Row1, hv_Column2, hv_Row2, hv_Phi1, hv_Radius * 3, (*hv_dis_rr),
		hv_WindowHandle, 0);
	DrawIndica(hv_Qx, hv_Qy, hv_Qx2, hv_Qy2, hv_Phi1, hv_Radius * 4, (*hv_length), hv_WindowHandle,
		0);
	DrawIndica(hv_upx1, hv_upy1, hv_dnx2, hv_dny2, hv_Phi, hv_Radius + 100, (*hv_width),
		hv_WindowHandle, 1);
	DrawIndica(hv_Column4, hv_Row4, hv_Qx41, hv_Qy41, hv_Phi1, hv_Radius * 2, (*hv_up_margin),
		hv_WindowHandle, 1);
	DrawIndica(hv_Column4, hv_Row4, hv_Qx11, hv_Qy11, hv_Phi1, hv_Radius * 2, (*hv_down_margin),
		hv_WindowHandle, 1);
	return;
}
void SingleCircle(HObject ho_imagesrc, HObject ho_regionsrc, HObject ho_regionref,
	HTuple hv_deviation_value1, HTuple hv_deviation_value2, HTuple hv_factor, HTuple hv_WindowHandle,
	HTuple *hv_length, HTuple *hv_center_x, HTuple *hv_center_y, HTuple *hv_isError,
	HTuple *hv_ErrorCode)
{

	// Local iconic variables
	HObject  ho_ImageReduced, ho_Union, ho_RegionClosing;
	HObject  ho_RegionFillUp, ho_RegionErosion, ho_RegionDilation;
	HObject  ho_ConnectedRegions, ho_SelectedRegions, ho_ObjectSelected1;
	HObject  ho_Contours, ho_ContCircle;

	// Local control variables
	HTuple  hv_Value, hv_gray_ref, hv_AreaRegion;
	HTuple  hv_Row5, hv_Column5, hv_Number1, hv_Row1, hv_Column1;
	HTuple  hv_Radius, hv_StartPhi, hv_EndPhi, hv_PointOrder;
	HTuple  hv_x1, hv_y1, hv_x2, hv_y2, hv_Length, hv_Exception;
	HTuple  hv_ErrorMsg;

	(*hv_isError) = 0;
	(*hv_ErrorCode) = 0;

	try
	{
		DispObj(ho_imagesrc, hv_WindowHandle);
		//求参考平面灰度值
		GrayFeatures(ho_regionref, ho_imagesrc, "mean", &hv_Value);
		hv_gray_ref = hv_Value;
		//求出检测的区域的面积
		AreaCenter(ho_regionsrc, &hv_AreaRegion, &hv_Row5, &hv_Column5);
		//抠出目标
		//deviation_value1 := 50
		ReduceDomain(ho_imagesrc, ho_regionsrc, &ho_ImageReduced);

		Threshold(ho_ImageReduced, &ho_Union, HTuple(0).TupleConcat(hv_gray_ref + hv_deviation_value1),
			(hv_gray_ref - hv_deviation_value1).TupleConcat(255));
		Union1(ho_Union, &ho_Union);
		//预处理
		ClosingCircle(ho_Union, &ho_RegionClosing, 5.5);
		FillUp(ho_RegionClosing, &ho_RegionFillUp);
		ErosionCircle(ho_RegionFillUp, &ho_RegionErosion, 5.5);
		DilationCircle(ho_RegionErosion, &ho_RegionDilation, 5.5);
		HalconCpp::Connection(ho_RegionDilation, &ho_ConnectedRegions);
		SelectShape(ho_ConnectedRegions, &ho_SelectedRegions, "roundness", "and", 0.8092,
			2);
		CountObj(ho_SelectedRegions, &hv_Number1);
		if (0 != (hv_Number1<1))
		{
			(*hv_isError) = 1;
			(*hv_ErrorCode) = 1;
			//返回1
			return;
		}
		SelectObj(ho_SelectedRegions, &ho_ObjectSelected1, 1);
		GenContourRegionXld(ho_ObjectSelected1, &ho_Contours, "border");
		FitCircleContourXld(ho_Contours, "geotukey", -1, 2, 0, 10, 1, &hv_Row1, &hv_Column1,
			&hv_Radius, &hv_StartPhi, &hv_EndPhi, &hv_PointOrder);
		if (0 != ((hv_Row1.TupleLength()) != 1))
		{
			(*hv_isError) = 1;
			(*hv_ErrorCode) = 2;
			//返回1
			return;
		}
		GenCircleContourXld(&ho_ContCircle, hv_Row1, hv_Column1, hv_Radius, 0, 6.28318,
			"positive", 1);
		DispObj(ho_ContCircle, hv_WindowHandle);
		hv_x1 = hv_Column1;
		hv_y1 = hv_Row1 - hv_Radius;
		hv_x2 = hv_Column1;
		hv_y2 = hv_Row1 + hv_Radius;
		hv_Length = (hv_Radius * 2)*hv_factor;
		(*hv_length) = hv_Length;
		(*hv_center_x) = hv_Column1;
		(*hv_center_y) = hv_Row1;
		DrawIndica(hv_x1, hv_y1, hv_x2, hv_y2, 0, hv_Radius*2.5, hv_Length, hv_WindowHandle,
			1);
	}
	// catch (Exception) 
	catch (HalconCpp::HException &HDevExpDefaultException)
	{
		HDevExpDefaultException.ToHTuple(&hv_Exception);
		HalconCpp::HException::GetExceptionData(hv_Exception, "error_msg", &hv_ErrorMsg);
		disp_message(hv_WindowHandle, "Error message: " + hv_ErrorMsg, "window", 12, 12,
			"red", "true");
		(*hv_isError) = 1;
		(*hv_ErrorCode) = 4;
		//返回1
		return;
	}
	return;
}

void DoubleCircle2(HObject ho_imagesrc, HObject ho_regionsrc, HObject ho_regionref,
	HTuple hv_deviation_value1, HTuple hv_deviation_value2, HTuple hv_deviation_value3,
	HTuple hv_factor, HTuple hv_half_margin, HTuple hv_WindowHandle, HTuple hv_thr,
	HTuple hv_sigma, HTuple *hv_isError, HTuple *hv_ErrorCode, HTuple *hv_length,
	HTuple *hv_width, HTuple *hv_angle, HTuple *hv_center_x, HTuple *hv_center_y,
	HTuple *hv_dis_rr, HTuple *hv_up_margin, HTuple *hv_down_margin)
{

	// Local iconic variables
	HObject  ho_ImageReduced, ho_Union, ho_RegionClosing;
	HObject  ho_RegionFillUp, ho_RegionErosion, ho_RegionDilation;
	HObject  ho_ConnectedRegions, ho_SelectedRegions, ho_ObjectSelected1;
	HObject  ho_RegionTrans, ho_RegionTrans1, ho_ImageReduced1;
	HObject  ho_Cross, ho_Cross1, ho_Cross2, ho_RegionLines;
	HObject  ho_Contours, ho_RectangleLeft, ho_Rectangle, ho_ClippedContours;
	HObject  ho_ContCircle, ho_Cross3, ho_Rectangle1, ho_ClippedContours1;
	HObject  ho_ContCircle1, ho_Cross9, ho_CrossUp, ho_Cross4;
	HObject  ho_Cross5, ho_Cross6, ho_Cross7, ho_RegionLines1;
	HObject  ho_Cross10, ho_Cross8;

	// Local control variables
	HTuple  hv_Value, hv_gray_ref, hv_AreaRegion;
	HTuple  hv_Row5, hv_Column5, hv_th1, hv_Number1, hv_Row;
	HTuple  hv_Column, hv_Phi, hv_Length1, hv_Length2, hv_HomMat2DIdentity;
	HTuple  hv_HomMat2DRotate, hv_HomMat2DRotate2, hv_Qx, hv_Qy;
	HTuple  hv_Qx2, hv_Qy2, hv_Row11, hv_Column11, hv_Row2;
	HTuple  hv_Column2, hv_Row1, hv_Column1, hv_Radius, hv_StartPhi;
	HTuple  hv_EndPhi, hv_PointOrder, hv_Row12, hv_Column12;
	HTuple  hv_Row21, hv_Column21, hv_Radius2, hv_cen_x, hv_cen_y;
	HTuple  hv_HomMat0, hv_p, hv_HomMat0_R, hv_upx1, hv_upy1;
	HTuple  hv_dnx2, hv_dny2, hv_Phi1, hv_Row3, hv_Column3;
	HTuple  hv_Length11, hv_Length21, hv_HomMat2DIdentity3;
	HTuple  hv_Qx11, hv_Qy11, hv_Qx21, hv_Qy21, hv_Qx31, hv_Qy31;
	HTuple  hv_Qx41, hv_Qy41, hv_AmplitudeThreshold, hv_RoiWidthLen2;
	HTuple  hv_Sigma, hv_LineRowStart_Measure_01_0, hv_LineColumnStart_Measure_01_0;
	HTuple  hv_LineRowEnd_Measure_01_0, hv_LineColumnEnd_Measure_01_0;
	HTuple  hv_TmpCtrl_Row, hv_TmpCtrl_Column, hv_TmpCtrl_Dr;
	HTuple  hv_TmpCtrl_Dc, hv_TmpCtrl_Phi, hv_TmpCtrl_Len1;
	HTuple  hv_TmpCtrl_Len2, hv_MsrHandle_Measure_01_0, hv_Row_Measure_01_0;
	HTuple  hv_Column_Measure_01_0, hv_Amplitude_Measure_01_0;
	HTuple  hv_Distance_Measure_01_0, hv_up_x, hv_up_y, hv_dn_x;
	HTuple  hv_dn_y, hv_Row4, hv_Column4, hv_IsOverlapping;
	HTuple  hv_Distance, hv_Distance2, hv_Dis_pp, hv_Exception;
	HTuple  hv_ErrorMsg;

	(*hv_isError) = 0;
	(*hv_ErrorCode) = 0;


	try
	{
		//求参考平面灰度值
		GrayFeatures(ho_regionref, ho_imagesrc, "mean", &hv_Value);
		hv_gray_ref = hv_Value;
		//求出检测的区域的面积
		AreaCenter(ho_regionsrc, &hv_AreaRegion, &hv_Row5, &hv_Column5);
		//抠出目标
		//deviation_value1 := 50
		ReduceDomain(ho_imagesrc, ho_regionsrc, &ho_ImageReduced);
		//threshold (ImageReduced, Union, [0,242], [99,255])
		hv_th1 = 0;
		Threshold(ho_ImageReduced, &ho_Union, HTuple(0).TupleConcat(hv_gray_ref + hv_deviation_value2),
			(hv_gray_ref - hv_deviation_value1).TupleConcat(255));

		//union1 (Union, Union)
		//预处理
		ClosingCircle(ho_Union, &ho_RegionClosing, 5.5);
		FillUp(ho_RegionClosing, &ho_RegionFillUp);
		ErosionCircle(ho_RegionFillUp, &ho_RegionErosion, 5.5);
		DilationCircle(ho_RegionErosion, &ho_RegionDilation, 5.5);
		HalconCpp::Connection(ho_RegionDilation, &ho_ConnectedRegions);
		SelectShape(ho_ConnectedRegions, &ho_SelectedRegions, "area", "and", hv_AreaRegion*(1 / 3.0),
			hv_AreaRegion);
		CountObj(ho_SelectedRegions, &hv_Number1);
		if (0 != (hv_Number1<1))
		{
			(*hv_isError) = 1;
			(*hv_ErrorCode) = 1;
			//返回1
			return;
		}
		SelectObj(ho_SelectedRegions, &ho_ObjectSelected1, 1);
		//拟合成矩形 此时Length2接近圆的直径
		ShapeTrans(ho_ObjectSelected1, &ho_RegionTrans, "convex");
		ShapeTrans(ho_RegionTrans, &ho_RegionTrans1, "rectangle2");
		ReduceDomain(ho_ImageReduced, ho_RegionTrans1, &ho_ImageReduced1);
		SmallestRectangle2(ho_RegionTrans1, &hv_Row, &hv_Column, &hv_Phi, &hv_Length1,
			&hv_Length2);
		GenCrossContourXld(&ho_Cross, hv_Row, hv_Column, 6, hv_Phi);
		//求两个最远的端点坐标
		HomMat2dIdentity(&hv_HomMat2DIdentity);
		HomMat2dRotate(hv_HomMat2DIdentity, -hv_Phi, hv_Column, hv_Row, &hv_HomMat2DRotate);
		HomMat2dRotate(hv_HomMat2DIdentity, hv_Phi, hv_Column, hv_Row, &hv_HomMat2DRotate2);
		AffineTransPoint2d(hv_HomMat2DRotate, hv_Column + hv_Length1, hv_Row, &hv_Qx, &hv_Qy);
		AffineTransPoint2d(hv_HomMat2DRotate, hv_Column - hv_Length1, hv_Row, &hv_Qx2,
			&hv_Qy2);
		//最长距离的连线
		GenCrossContourXld(&ho_Cross1, hv_Qy2, hv_Qx2, 10, hv_Phi);
		GenCrossContourXld(&ho_Cross2, hv_Qy, hv_Qx, 10, hv_Phi);
		GenRegionLine(&ho_RegionLines, hv_Qy2, hv_Qx2, hv_Qy, hv_Qx);
		DispObj(ho_RegionLines, hv_WindowHandle);
		//left左圆
		GenContourRegionXld(ho_RegionTrans, &ho_Contours, "border");
		GenRectangle2(&ho_RectangleLeft, hv_Qy2, hv_Qx2, hv_Phi, hv_Length2 + 20, hv_Length2 + 20);

		SmallestRectangle1(ho_RectangleLeft, &hv_Row11, &hv_Column11, &hv_Row2, &hv_Column2);
		GenRectangle1(&ho_Rectangle, hv_Row11, hv_Column11, hv_Row2, hv_Column2);
		ClipContoursXld(ho_Contours, &ho_ClippedContours, hv_Row11, hv_Column11, hv_Row2,
			hv_Column2);
		FitCircleContourXld(ho_ClippedContours, "geotukey", -1, 2, 0, 10, 1, &hv_Row1,
			&hv_Column1, &hv_Radius, &hv_StartPhi, &hv_EndPhi, &hv_PointOrder);
		if (0 != ((hv_Row1.TupleLength()) != 1))
		{
			(*hv_isError) = 1;
			(*hv_ErrorCode) = 2;
			//返回1
			return;
		}

		GenCircleContourXld(&ho_ContCircle, hv_Row1, hv_Column1, hv_Radius, 0, 6.28318,
			"positive", 1);
		GenCrossContourXld(&ho_Cross3, hv_Row1, hv_Column1, 16, hv_Phi);
		DispObj(ho_Cross3, hv_WindowHandle);
		DispObj(ho_ContCircle, hv_WindowHandle);
		//right右圆
		GenRectangle2(&ho_RectangleLeft, hv_Qy, hv_Qx, hv_Phi, hv_Length2 + 20, hv_Length2 + 20);
		SmallestRectangle1(ho_RectangleLeft, &hv_Row12, &hv_Column12, &hv_Row21, &hv_Column21);
		GenRectangle1(&ho_Rectangle1, hv_Row12, hv_Column12, hv_Row21, hv_Column21);
		ClipContoursXld(ho_Contours, &ho_ClippedContours1, hv_Row12, hv_Column12, hv_Row21,
			hv_Column21);
		FitCircleContourXld(ho_ClippedContours1, "geotukey", -1, 2, 0, 10, 1, &hv_Row2,
			&hv_Column2, &hv_Radius2, &hv_StartPhi, &hv_EndPhi, &hv_PointOrder);
		if (0 != ((hv_Row2.TupleLength()) != 1))
		{
			(*hv_isError) = 1;
			(*hv_ErrorCode) = 3;
			//返回3
			return;
		}
		GenCircleContourXld(&ho_ContCircle1, hv_Row2, hv_Column2, hv_Radius2, 0, 6.28318,
			"positive", 1);
		GenCrossContourXld(&ho_Cross9, hv_Row2, hv_Column2, 16, hv_Phi);
		DispObj(ho_ContCircle1, hv_WindowHandle);
		DispObj(ho_Cross9, hv_WindowHandle);
		//result
		//两个圆心连线的中心center point
		hv_cen_x = (hv_Column1 + hv_Column2) / 2.0;
		hv_cen_y = (hv_Row2 + hv_Row1) / 2.0;
		//
		HomMat2dIdentity(&hv_HomMat0);
		hv_p = (HTuple(90).TupleRad()) + hv_Phi;
		HomMat2dRotate(hv_HomMat0, -hv_p, hv_Column2, hv_Row2, &hv_HomMat0_R);
		AffineTransPoint2d(hv_HomMat0_R, hv_Column2 + hv_Radius2, hv_Row2, &hv_upx1, &hv_upy1);
		GenCrossContourXld(&ho_CrossUp, hv_upy1, hv_upx1, 36, hv_Phi);

		AffineTransPoint2d(hv_HomMat0_R, hv_Column2 - hv_Radius2, hv_Row2, &hv_dnx2, &hv_dny2);
		GenCrossContourXld(&ho_CrossUp, hv_dny2, hv_dnx2, 36, hv_Phi);
		//
		//
		//
		hv_Phi1 = hv_Phi;
		hv_Row3 = hv_Row;
		hv_Column3 = hv_Column;
		hv_Length11 = hv_Length1 + 40;
		hv_Length21 = hv_Length2 + hv_half_margin;
		HomMat2dIdentity(&hv_HomMat2DIdentity3);
		HomMat2dRotate(hv_HomMat2DIdentity3, -hv_Phi1, hv_Row3, hv_Column3, &hv_HomMat2DRotate2);
		AffineTransPoint2d(hv_HomMat2DRotate2, hv_Column3 + hv_Length11, hv_Row3 + hv_Length21,
			&hv_Qx11, &hv_Qy11);
		GenCrossContourXld(&ho_Cross4, hv_Qy11, hv_Qx11, 6, hv_Phi1);
		AffineTransPoint2d(hv_HomMat2DRotate2, hv_Column3 - hv_Length11, hv_Row3 + hv_Length21,
			&hv_Qx21, &hv_Qy21);
		GenCrossContourXld(&ho_Cross5, hv_Qy21, hv_Qx21, 6, hv_Phi1);
		AffineTransPoint2d(hv_HomMat2DRotate2, hv_Column3 - hv_Length11, hv_Row3 - hv_Length21,
			&hv_Qx31, &hv_Qy31);
		GenCrossContourXld(&ho_Cross6, hv_Qy31, hv_Qx31, 6, hv_Phi1);
		AffineTransPoint2d(hv_HomMat2DRotate2, hv_Column3 + hv_Length11, hv_Row3 - hv_Length21,
			&hv_Qx41, &hv_Qy41);
		GenCrossContourXld(&ho_Cross7, hv_Qy41, hv_Qx41, 6, hv_Phi1);
		//求出边缘点
		//Measure 01: Code generated by Measure 01
		//Measure 01: Prepare measurement

		//AmplitudeThreshold := 24
		hv_AmplitudeThreshold = hv_thr;
		hv_RoiWidthLen2 = 60.5;
		//Sigma := 4
		hv_Sigma = hv_sigma;
		SetSystem("int_zooming", "true");
		//Measure 01: Coordinates for line Measure 01 [0]
		hv_LineRowStart_Measure_01_0 = hv_Qy41;
		hv_LineColumnStart_Measure_01_0 = hv_Qx41;
		hv_LineRowEnd_Measure_01_0 = hv_Qy11;
		hv_LineColumnEnd_Measure_01_0 = hv_Qx11;
		//Measure 01: Convert coordinates to rectangle2 type
		hv_TmpCtrl_Row = 0.5*(hv_LineRowStart_Measure_01_0 + hv_LineRowEnd_Measure_01_0);
		hv_TmpCtrl_Column = 0.5*(hv_LineColumnStart_Measure_01_0 + hv_LineColumnEnd_Measure_01_0);
		hv_TmpCtrl_Dr = hv_LineRowStart_Measure_01_0 - hv_LineRowEnd_Measure_01_0;
		hv_TmpCtrl_Dc = hv_LineColumnEnd_Measure_01_0 - hv_LineColumnStart_Measure_01_0;
		hv_TmpCtrl_Phi = hv_TmpCtrl_Dr.TupleAtan2(hv_TmpCtrl_Dc);
		hv_TmpCtrl_Len1 = 0.5*(((hv_TmpCtrl_Dr*hv_TmpCtrl_Dr) + (hv_TmpCtrl_Dc*hv_TmpCtrl_Dc)).TupleSqrt());
		hv_TmpCtrl_Len2 = hv_RoiWidthLen2;
		//Measure 01: Create measure for line Measure 01 [0]
		//Measure 01: Attention: This assumes all images have the same size!
		GenMeasureRectangle2(hv_TmpCtrl_Row, hv_TmpCtrl_Column, hv_TmpCtrl_Phi, hv_TmpCtrl_Len1,
			hv_TmpCtrl_Len2, 4024, 3036, "nearest_neighbor", &hv_MsrHandle_Measure_01_0);
		GenRegionLine(&ho_RegionLines1, hv_LineRowStart_Measure_01_0, hv_LineColumnStart_Measure_01_0,
			hv_LineRowEnd_Measure_01_0, hv_LineColumnEnd_Measure_01_0);
		//Measure 01: ***************************************************************
		//Measure 01: * The code which follows is to be executed once / measurement *
		//Measure 01: ***************************************************************
		//Measure 01: *************
		//Measure 01: * ATTENTION *
		//Measure 01: *************
		//Measure 01: The image from the graphics window is not available. At this point,
		//Measure 01: it is necessary to ensure an image is stored in the variable 'Image'
		//Measure 01: Execute measurements
		MeasurePos(ho_imagesrc, hv_MsrHandle_Measure_01_0, hv_Sigma, hv_AmplitudeThreshold,
			"all", "all", &hv_Row_Measure_01_0, &hv_Column_Measure_01_0, &hv_Amplitude_Measure_01_0,
			&hv_Distance_Measure_01_0);
		//Measure 01: Do something with the results
		GenCrossContourXld(&ho_Cross10, hv_Row_Measure_01_0, hv_Column_Measure_01_0,
			6, hv_Phi);
		//Measure 01: Clear measure when done
		CloseMeasure(hv_MsrHandle_Measure_01_0);
		//****************************************************************************
		if (0 != ((hv_Row_Measure_01_0.TupleLength()) == 4))
		{
			hv_up_x = ((const HTuple&)hv_Column_Measure_01_0)[1];
			hv_up_y = ((const HTuple&)hv_Row_Measure_01_0)[1];
			hv_dn_x = ((const HTuple&)hv_Column_Measure_01_0)[2];
			hv_dn_y = ((const HTuple&)hv_Row_Measure_01_0)[2];
		}
		else
		{
			(*hv_isError) = 1;
			(*hv_ErrorCode) = 6;
			//返回3
			return;
		}
		//交点
		IntersectionLines(hv_up_y, hv_up_x, hv_dn_y, hv_dn_x, hv_Qy2, hv_Qx2, hv_Qy,
			hv_Qx, &hv_Row4, &hv_Column4, &hv_IsOverlapping);
		GenCrossContourXld(&ho_Cross8, hv_Row4, hv_Column4, 6, hv_Phi1);
		//中心线到目标线的长度
		DistancePp(hv_Row4, hv_Column4, hv_up_y, hv_up_x, &hv_Distance);
		DistancePp(hv_Row4, hv_Column4, hv_dn_y, hv_dn_x, &hv_Distance2);
		//求两个圆心之间的距离
		DistancePp(hv_Row2, hv_Column2, hv_Row1, hv_Column1, &hv_Dis_pp);
	}
	// catch (Exception) 
	catch (HalconCpp::HException &HDevExpDefaultException)
	{
		HDevExpDefaultException.ToHTuple(&hv_Exception);
		HalconCpp::HException::GetExceptionData(hv_Exception, "error_msg", &hv_ErrorMsg);
		disp_message(hv_WindowHandle, "Error message: " + hv_ErrorMsg, "window", 12, 12,
			"red", "true");
		(*hv_isError) = 1;
		(*hv_ErrorCode) = 4;
		//返回1
		return;
	}
	//返回值
	(*hv_dis_rr) = hv_Dis_pp*hv_factor;
	//长度
	(*hv_length) = ((hv_Dis_pp + hv_Radius2) + hv_Radius)*hv_factor;
	//宽度
	(*hv_width) = (hv_Radius2 + hv_Radius)*hv_factor;
	//角度方向（弧度制）
	(*hv_angle) = hv_Phi1;
	//中心点 x坐标
	(*hv_center_x) = hv_cen_x*hv_factor;
	(*hv_center_y) = hv_cen_y*hv_factor;
	//上边缘距离中轴线
	(*hv_up_margin) = hv_Distance*hv_factor;
	(*hv_down_margin) = hv_Distance2*hv_factor;

	//两个圆心之间的距离
	DrawIndica(hv_Column1, hv_Row1, hv_Column2, hv_Row2, hv_Phi1, hv_Radius * 3, (*hv_dis_rr),
		hv_WindowHandle, 0);
	DrawIndica(hv_Qx, hv_Qy, hv_Qx2, hv_Qy2, hv_Phi1, hv_Radius * 4, (*hv_length), hv_WindowHandle,
		0);
	DrawIndica(hv_upx1, hv_upy1, hv_dnx2, hv_dny2, hv_Phi, hv_Radius + 100, (*hv_width),
		hv_WindowHandle, 1);
	DrawIndica(hv_Column4, hv_Row4, hv_up_x, hv_up_y, hv_Phi1, hv_Radius * 2, (*hv_up_margin),
		hv_WindowHandle, 1);
	DrawIndica(hv_Column4, hv_Row4, hv_dn_x, hv_dn_y, hv_Phi1, hv_Radius * 2, (*hv_down_margin),
		hv_WindowHandle, 1);
	return;
}
////////////////////////////////////////////
//异常处理函数
void MyExcepHandle(const HException &except)
{
	throw except;
}
BOOL ImProcChecker::GetReult(CheckResult &res)
{
	res = m_Result;
	return TRUE;
}
BOOL ImProcChecker::GetReult(MeasureResult & res)
{
	res = m_Result_measure;
	return TRUE;
}
void ImProcChecker::disp_message(HTuple hv_WindowHandle, HTuple hv_String, HTuple hv_CoordSystem,
	HTuple hv_Row, HTuple hv_Column, HTuple hv_Color, HTuple hv_Box)
{

	// Local iconic variables

	// Local control variables
	HTuple  hv_GenParamName, hv_GenParamValue;

	//This procedure displays text in a graphics window.
	//
	//Input parameters:
	//WindowHandle: The WindowHandle of the graphics window, where
	//   the message should be displayed
	//String: A tuple of strings containing the text message to be displayed
	//CoordSystem: If set to 'window', the text position is given
	//   with respect to the window coordinate system.
	//   If set to 'image', image coordinates are used.
	//   (This may be useful in zoomed images.)
	//Row: The row coordinate of the desired text position
	//   A tuple of values is allowed to display text at different
	//   positions.
	//Column: The column coordinate of the desired text position
	//   A tuple of values is allowed to display text at different
	//   positions.
	//Color: defines the color of the text as string.
	//   If set to [], '' or 'auto' the currently set color is used.
	//   If a tuple of strings is passed, the colors are used cyclically...
	//   - if |Row| == |Column| == 1: for each new textline
	//   = else for each text position.
	//Box: If Box[0] is set to 'true', the text is written within an orange box.
	//     If set to' false', no box is displayed.
	//     If set to a color string (e.g. 'white', '#FF00CC', etc.),
	//       the text is written in a box of that color.
	//     An optional second value for Box (Box[1]) controls if a shadow is displayed:
	//       'true' -> display a shadow in a default color
	//       'false' -> display no shadow
	//       otherwise -> use given string as color string for the shadow color
	//
	//It is possible to display multiple text strings in a single call.
	//In this case, some restrictions apply:
	//- Multiple text positions can be defined by specifying a tuple
	//  with multiple Row and/or Column coordinates, i.e.:
	//  - |Row| == n, |Column| == n
	//  - |Row| == n, |Column| == 1
	//  - |Row| == 1, |Column| == n
	//- If |Row| == |Column| == 1,
	//  each element of String is display in a new textline.
	//- If multiple positions or specified, the number of Strings
	//  must match the number of positions, i.e.:
	//  - Either |String| == n (each string is displayed at the
	//                          corresponding position),
	//  - or     |String| == 1 (The string is displayed n times).
	//
	//
	//Convert the parameters for disp_text.
	if (0 != (HTuple(hv_Row == HTuple()).TupleOr(hv_Column == HTuple())))
	{
		return;
	}
	if (0 != (hv_Row == -1))
	{
		hv_Row = 12;
	}
	if (0 != (hv_Column == -1))
	{
		hv_Column = 12;
	}
	//
	//Convert the parameter Box to generic parameters.
	hv_GenParamName = HTuple();
	hv_GenParamValue = HTuple();
	if (0 != ((hv_Box.TupleLength())>0))
	{
		if (0 != (HTuple(hv_Box[0]) == HTuple("false")))
		{
			//Display no box
			hv_GenParamName = hv_GenParamName.TupleConcat("box");
			hv_GenParamValue = hv_GenParamValue.TupleConcat("false");
		}
		else if (0 != (HTuple(hv_Box[0]) != HTuple("true")))
		{
			//Set a color other than the default.
			hv_GenParamName = hv_GenParamName.TupleConcat("box_color");
			hv_GenParamValue = hv_GenParamValue.TupleConcat(HTuple(hv_Box[0]));
		}
	}
	if (0 != ((hv_Box.TupleLength())>1))
	{
		if (0 != (HTuple(hv_Box[1]) == HTuple("false")))
		{
			//Display no shadow.
			hv_GenParamName = hv_GenParamName.TupleConcat("shadow");
			hv_GenParamValue = hv_GenParamValue.TupleConcat("false");
		}
		else if (0 != (HTuple(hv_Box[1]) != HTuple("true")))
		{
			//Set a shadow color other than the default.
			hv_GenParamName = hv_GenParamName.TupleConcat("shadow_color");
			hv_GenParamValue = hv_GenParamValue.TupleConcat(HTuple(hv_Box[1]));
		}
	}
	//Restore default CoordSystem behavior.
	if (0 != (hv_CoordSystem != HTuple("window")))
	{
		hv_CoordSystem = "image";
	}
	//
	if (0 != (hv_Color == HTuple("")))
	{
		//disp_text does not accept an empty string for Color.
		hv_Color = HTuple();
	}
	//
	//DispText(hv_WindowHandle, hv_String, hv_CoordSystem, hv_Row, hv_Column, hv_Color,
	//	hv_GenParamName, hv_GenParamValue);
	return;
}

void ImProcChecker::ShowException(HalconCpp::HException & except)
{
	CString str(except.ErrorMessage());
	str.Replace(_T("HALCON"), _T(""));
	MessageBoxW((HWND)hl_wnd, str, _T("Error"), MB_OK);
}

void ImProcChecker::ContoursAll()
{
	if (ImProcChecker::WindowHandle.Length()&& ImProcChecker::g_hoContoursAll.IsInitialized())
	{
		HalconCpp::SetColor(WindowHandle, "green");
		DispObj(ImProcChecker::g_hoContoursAll, ImProcChecker::WindowHandle);
	}
		
}


ImProcChecker::ImProcChecker()
{
	
	m_pImageBuffer = NULL;
	m_pDataSeparate = NULL;
	m_bRoi = false;
	m_bLoadModel = false;
	m_bLoadDeformModel = false;
	if (!ImProcChecker::g_hoContoursAll.IsInitialized())
		GenEmptyObj(&ImProcChecker::g_hoContoursAll);
}


ImProcChecker::~ImProcChecker()
{
	if (m_pImageBuffer)
	{
		free(m_pImageBuffer);
		m_pImageBuffer = NULL;
	}
	if (m_pDataSeparate)
	{
		free(m_pDataSeparate);
		m_pDataSeparate = NULL;
	}

	//如果MODELID存在,则释放
	int len = m_hvModelID.TupleLength();
	if (len)
	{
		int idModel = m_hvModelID[0].I();
		if (idModel >-1)
			HalconCpp::ClearShapeModel(m_hvModelID);
	}

	len = m_hvDeformModelID.TupleLength();
	if (len)
	{
		int idModel = m_hvDeformModelID[0].I();
		if (idModel >-1)
			HalconCpp::ClearDeformableModel(m_hvDeformModelID);
	}

	len = m_hvBarCodeHandle.TupleLength();
	if (len)
	{
		int idModel = m_hvBarCodeHandle[0].I();
		if (idModel >-1)
			HalconCpp::ClearBarCodeModel(m_hvBarCodeHandle);
	}
}

void ImProcChecker::SetCam(myCameraClass * pCam)
{
	m_pCam = pCam;
}

bool ImProcChecker::SetDispWnd(Hlong wnd, CRect showRect)
{
	try
	{
		hl_wnd = wnd;
		//m_rect = showRect;
		if (WindowHandle.Length())
		{
			CloseWindow(WindowHandle);
		}
		OpenWindow(showRect.top, showRect.left, showRect.Width(), showRect.Height(), wnd, "", "", &WindowHandle);
		HalconCpp::SetSystem("tsp_width", IMG_W);
		HalconCpp::SetSystem("tsp_height", IMG_H);
		HalconCpp::SetPart(WindowHandle, 0, 0, IMG_H - 1, IMG_W - 1);
		HalconCpp::SetColor(WindowHandle, "green");
		HalconCpp::SetDraw(WindowHandle, "margin");
		//HalconCpp::SetLineWidth(WindowHandle, 1);
		//HObject img;
		//HalconCpp::ReadImage(&img, "default.bmp");
		//DispObj(img, WindowHandle);
	}
	catch (HalconCpp::HException & except)
	{
		ShowException(except);
		return false;
	}
	return true;
}

bool ImProcChecker::DrawRect(DetectArea & showRect)
{
	if (!WindowHandle.Length()) return false;

	HalconCpp::HTuple Row1, Col1, Row2, Col2;
	SetColor(WindowHandle, "green");
	SetDraw(WindowHandle, "margin");
	SetTposition(WindowHandle, HTuple(m_TextRow), HTuple(m_TextCol));
	WriteString(WindowHandle, "Please Draw A Model Detect Area");

	HalconCpp::DrawRectangle1(WindowHandle, &Row1, &Col1, &Row2, &Col2);
	DetectArea areaTmp;
	if (!Row1.Length())
	{
		SetTposition(WindowHandle, HTuple(m_TextRow + 200), HTuple(m_TextCol));
		WriteString(WindowHandle, "Model Detect Area Too Small");
		return false;
	}
	areaTmp.row1 = (int)Row1.D();
	areaTmp.col1 = (int)Col1.D();
	areaTmp.row2 = (int)Row2.D();
	areaTmp.col2 = (int)Col2.D();
	int w = areaTmp.col2 - areaTmp.col1;
	int h = areaTmp.row2 - areaTmp.row1;
	if (w < 10 || h < 10) //框选范围太小
	{
		SetTposition(WindowHandle, HTuple(m_TextRow + 200), HTuple(m_TextCol));
		WriteString(WindowHandle, "Model Detect Area Too Small");
		return false;
	}

	HObject m_ho_ModelDetectArea;
	GenRectangle1(&m_ho_ModelDetectArea, Row1, Col1, Row2, Col2);
	DispObj(m_ho_ModelDetectArea, WindowHandle);
	areaTmp.bAvalid = true;

	//
	showRect = areaTmp;
	return true;
}

bool ImProcChecker::ClearRect()
{
	if (!WindowHandle.Length()) return false;
	ClearWindow(WindowHandle);
	return true;
}

void ImProcChecker::SetType(int type)
{
	m_type = type;
	if (6 == type)
	{
		CreateBarCodeModel("element_size_min", 1.5, &m_hvBarCodeHandle);
	}
}

BOOL ImProcChecker::LoadModel(char * pModelPath)
{
	try
	{
		int len = m_hvModelID.Length();
		if (len)
		{
			int idModel = m_hvModelID[0].I();
			if (idModel > -1)
				HalconCpp::ClearShapeModel(m_hvModelID);
		}
		//Matching 02: Read the shape model from file
		ReadShapeModel(pModelPath, &m_hvModelID);
		HalconCpp::GetShapeModelContours(&m_hoModelContours, m_hvModelID, 1);
		m_bLoadModel = true;
	}
	catch (HalconCpp::HException & except)
	{
		m_bLoadModel = false;
		ShowException(except);
		return FALSE;
	}
	return TRUE;
}

BOOL ImProcChecker::LoadDeformModel(char * pModelPath)
{
	try
	{
		int len = m_hvDeformModelID.Length();
		if (len)
		{
			int idModel = m_hvDeformModelID[0].I();
			if (idModel > -1)
				HalconCpp::ClearDeformableModel(m_hvDeformModelID);
		}
		//Matching 02: Read the shape model from file
		ReadDeformableModel(pModelPath, &m_hvDeformModelID);
		HalconCpp::GetDeformableModelContours(&m_hoDeformModelContours, m_hvDeformModelID, 1);
		m_bLoadDeformModel = true;
	}
	catch (HalconCpp::HException & except)
	{
		m_bLoadDeformModel = false;
		ShowException(except);
		return FALSE;
	}
	return TRUE;
}

BOOL ImProcChecker::LoadPara(wchar_t * pParaPath)
{
	CString strPath;
	strPath.Format(_T("%s"), pParaPath);

	CFileFind find;
	int result = find.FindFile(strPath);
	if (result == 0)
	{
		//WritePrivateProfileString(_T("相机参数"), _T("exp"), _T("50000"), strPath);

		WritePrivateProfileString(_T("检测区域"), _T("top"), _T("0"), strPath);
		WritePrivateProfileString(_T("检测区域"), _T("left"), _T("0"), strPath);
		WritePrivateProfileString(_T("检测区域"), _T("bottom"), _T("0"), strPath);
		WritePrivateProfileString(_T("检测区域"), _T("right"), _T("0"), strPath);

		WritePrivateProfileString(_T("参考区域"), _T("top"), _T("0"), strPath);
		WritePrivateProfileString(_T("参考区域"), _T("left"), _T("0"), strPath);
		WritePrivateProfileString(_T("参考区域"), _T("bottom"), _T("0"), strPath);
		WritePrivateProfileString(_T("参考区域"), _T("right"), _T("0"), strPath);

		WritePrivateProfileString(_T("通用参数"), _T("num"), _T("4"), strPath);
		WritePrivateProfileString(_T("通用参数"), _T("para1"), _T("0"), strPath);
		WritePrivateProfileString(_T("通用参数"), _T("para2"), _T("0"), strPath);
		WritePrivateProfileString(_T("通用参数"), _T("para3"), _T("0"), strPath);
		WritePrivateProfileString(_T("通用参数"), _T("para4"), _T("0"), strPath);
	}
	//加载检测区域
	CString SName;
	DWORD i = GetPrivateProfileString(_T("检测区域"), _T("top"), _T("0"), SName.GetBuffer(128), 128, strPath);
	SName.ReleaseBuffer();
	m_Para.decArea.row1 = _ttoi(SName);
	i = GetPrivateProfileString(_T("检测区域"), _T("left"), _T("0"), SName.GetBuffer(128), 128, strPath);
	SName.ReleaseBuffer();
	m_Para.decArea.col1 = _ttoi(SName);
	i = GetPrivateProfileString(_T("检测区域"), _T("bottom"), _T("2048"), SName.GetBuffer(128), 128, strPath);
	SName.ReleaseBuffer();
	m_Para.decArea.row2 = _ttoi(SName);
	i = GetPrivateProfileString(_T("检测区域"), _T("right"), _T("3072"), SName.GetBuffer(128), 128, strPath);
	SName.ReleaseBuffer();
	m_Para.decArea.col2 = _ttoi(SName);
	
	//生成检测区域
	int w = m_Para.decArea.col2 - m_Para.decArea.col1;
	int h = m_Para.decArea.row2 - m_Para.decArea.row1;
	if (w > 10 && h > 10)
		GenRectangle1(&m_ho_DetectArea, m_Para.decArea.row1, m_Para.decArea.col1, m_Para.decArea.row2, m_Para.decArea.col2);
	else
	{
		if(m_ho_DetectArea.IsInitialized())
			HalconCpp::ClearObj(m_ho_DetectArea);
	}

	//加载参考区域
	i = GetPrivateProfileString(_T("参考区域"), _T("top"), _T("0"), SName.GetBuffer(128), 128, strPath);
	SName.ReleaseBuffer();
	m_Para.refArea.row1 = _ttoi(SName);
	i = GetPrivateProfileString(_T("参考区域"), _T("left"), _T("0"), SName.GetBuffer(128), 128, strPath);
	SName.ReleaseBuffer();
	m_Para.refArea.col1 = _ttoi(SName);
	i = GetPrivateProfileString(_T("参考区域"), _T("bottom"), _T("2048"), SName.GetBuffer(128), 128, strPath);
	SName.ReleaseBuffer();
	m_Para.refArea.row2 = _ttoi(SName);
	i = GetPrivateProfileString(_T("参考区域"), _T("right"), _T("3072"), SName.GetBuffer(128), 128, strPath);
	SName.ReleaseBuffer();
	m_Para.refArea.col2 = _ttoi(SName);

	//生成检测区域
	 w = m_Para.refArea.col2 - m_Para.refArea.col1;
	 h = m_Para.refArea.row2 - m_Para.refArea.row1;
	if (w > 10 && h > 10)
		GenRectangle1(&m_ho_ReferArea, m_Para.refArea.row1, m_Para.refArea.col1, m_Para.refArea.row2, m_Para.refArea.col2);
	else
	{
		if (m_ho_ReferArea.IsInitialized())
			HalconCpp::ClearObj(m_ho_ReferArea);
	}

	//加载通用参数
	i = GetPrivateProfileString(_T("通用参数"), _T("num"), _T("0"), SName.GetBuffer(128), 128, strPath);
	SName.ReleaseBuffer();
	m_Para.para_num = _ttoi(SName);
	if (m_Para.para_num > 20) return FALSE;

	for (int m = 0; m < m_Para.para_num; m++)
	{
		CString str;
		str.Format(_T("para%d"), m + 1);
		i = GetPrivateProfileString(_T("通用参数"), str, _T("0"), SName.GetBuffer(128), 128, strPath);
		SName.ReleaseBuffer();
		m_Para.paraN[m] = _ttof(SName);
	}

	return TRUE;
}

BOOL ImProcChecker::SavePara(wchar_t * pParaPath)
{
	CString strPath;
	strPath.Format(_T("%s"), pParaPath);
	
	CString strValue;
	//
	strValue.Format(_T("%d"), m_Para.decArea.row1);
	WritePrivateProfileString(_T("检测区域"), _T("top"), strValue, strPath);
	strValue.Format(_T("%d"), m_Para.decArea.col1);
	WritePrivateProfileString(_T("检测区域"), _T("left"), strValue, strPath);
	strValue.Format(_T("%d"), m_Para.decArea.row2);
	WritePrivateProfileString(_T("检测区域"), _T("bottom"), strValue, strPath);
	strValue.Format(_T("%d"), m_Para.decArea.col2);
	WritePrivateProfileString(_T("检测区域"), _T("right"), strValue, strPath);

	//
	strValue.Format(_T("%d"), m_Para.refArea.row1);
	WritePrivateProfileString(_T("参考区域"), _T("top"), strValue, strPath);
	strValue.Format(_T("%d"), m_Para.refArea.col1);
	WritePrivateProfileString(_T("参考区域"), _T("left"), strValue, strPath);
	strValue.Format(_T("%d"), m_Para.refArea.row2);
	WritePrivateProfileString(_T("参考区域"), _T("bottom"), strValue, strPath);
	strValue.Format(_T("%d"), m_Para.refArea.col2);
	WritePrivateProfileString(_T("参考区域"), _T("right"), strValue, strPath);

	strValue.Format(_T("%d"), m_Para.para_num);
	WritePrivateProfileString(_T("通用参数"), _T("num"), strValue, strPath);
	for (int m = 0; m < m_Para.para_num; m++)
	{
		CString str;
		str.Format(_T("para%d"), m + 1);
		strValue.Format(_T("%f"), m_Para.paraN[m]);
		WritePrivateProfileString(_T("通用参数"), str, strValue, strPath);
	}
	return TRUE;
}

BOOL ImProcChecker::GrabImageFromCam(HObject & Img)
{
	//1.采图
	unsigned char * pData = NULL;
	MV_FRAME_OUT_INFO_EX stImageInfo = { 0 };

	g_criSection.Lock();
	m_pCam->GrabOneImage(&pData, stImageInfo);
	g_criSection.Unlock();
	if (!pData)
	{
		if (WindowHandle.Length())
		{
			SetTposition(WindowHandle, HTuple(m_TextRow), HTuple(m_TextCol));
			SetColor(WindowHandle, "red");
			WriteString(WindowHandle, "Camera Error");
		}
		return FALSE;
	}
	ho_Image.Clear();
	if (-1 == ConvertoHalcon(pData, stImageInfo, ho_Image))
	{
		return FALSE;
	}

	if (!WindowHandle.Length())
		return FALSE;

	if (!ho_Image.IsInitialized()) return FALSE;
	//HalconCpp::WriteImage(ho_Image, "bmp", 0, "test");
	DispObj(ho_Image, WindowHandle);

	//截取roi
	//if (m_bRoi && m_ho_DetectArea.IsInitialized())
	//{
	//	ReduceDomain(ho_Image, m_ho_DetectArea, &Img);
	//	HalconCpp::WriteImage(ho_Image, "bmp", 0, "test");
	//}
	//else
		Img = ho_Image.Clone();
	return TRUE;
}

BOOL ImProcChecker::GrabImageFromFile(char * path, HObject & Img)
{
	//打开文件选择窗口
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL/*_T("位图文件(*.BMP)|*.BMP|jpg文件(*.jpg)|*.jpg||")*/);
	if (IDOK == dlg.DoModal())
	{
		CString imgpath;
		imgpath.Format(_T("%s"), dlg.GetPathName());

		int len = WideCharToMultiByte(CP_ACP, 0, imgpath, -1, NULL, 0, NULL, NULL);//宽字符转
		char* strTemp = new char[len + 1];
		WideCharToMultiByte(CP_ACP, 0, imgpath, -1, strTemp, len, NULL, NULL);

		try
		{
			HalconCpp::ReadImage(&Img, strTemp);
			DispObj(Img, WindowHandle);
			ho_Image = Img;
			if (strTemp) delete[]strTemp;
			strTemp = NULL;
		}
		catch (HalconCpp::HException & except)
		{
			if (strTemp) delete[]strTemp;
			strTemp = NULL;
			ShowException(except);
			return false;
		}
		return true;
	}
	return false;
}

BOOL ImProcChecker::CheckShape(HObject &Img)
{
	//使用形状匹配
	m_Result.isOK = false;

	//2.识别
	HTuple hv_Row, hv_Column, hv_Angle, hv_Score, hv_Scale;
	xPara findPara;
	findPara.MinScore = m_Para.paraN[0];
	try
	{
	
		HObject ho_Img = Img;
		//截取感兴趣区域
		if (m_ho_DetectArea.IsInitialized())
		{
			ReduceDomain(Img, m_ho_DetectArea, &ho_Img);
		}

		//HalconCpp::FindShapeModel(Img, m_hvModelID,
		//HTuple(findPara.AngleStart).TupleRad(),
		//HTuple(findPara.AngleExtent).TupleRad(),
		//findPara.MinScore,
		//findPara.NumMatches,
		//findPara.MaxOverlap,
		//findPara.SubPixel.c_str()/*SubPixel*/,
		//(HTuple(findPara.ModelLevels).Append(findPara.NumLevels)),
		//findPara.Greediness,
		//&hv_Row, &hv_Column, &hv_Angle, &hv_Score);

		HalconCpp::FindScaledShapeModel(ho_Img, m_hvModelID,
			HTuple(findPara.AngleStart).TupleRad(),
			HTuple(findPara.AngleExtent).TupleRad(),
			findPara.MinScale,
			findPara.MaxScale,
			findPara.MinScore,
			findPara.NumMatches,
			findPara.MaxOverlap,
			findPara.SubPixel.c_str()/*SubPixel*/,
			(HTuple(findPara.ModelLevels).Append(findPara.NumLevels)),
			findPara.Greediness,
			&hv_Row, &hv_Column, &hv_Angle,&hv_Scale, &hv_Score);
	}
	catch (HalconCpp::HException & except)
	{
		if (WindowHandle.Length())
		{
			SetTposition(WindowHandle, HTuple(m_TextRow), HTuple(m_TextCol));
			SetColor(WindowHandle, "red");
			WriteString(WindowHandle, "Find Error:算法错误，请检查图像");
		}

		ShowException(except);
		return FALSE;
	}
	
	//3.判定
	//显示结果并返回
	int iNum = hv_Score.TupleLength();
	if (iNum == 0)
	{
		SetTposition(WindowHandle, HTuple(m_TextRow), HTuple(m_TextCol));
		//SetColor(WindowHandle, "red");
		//WriteString(WindowHandle, "Detect Failed:找不到目标，请检查图像或光源");
		return FALSE;
	}
	for (int i = 0; i < iNum; i++)
	{
		m_Result.x = hv_Column[i].D();
		m_Result.y = hv_Row[i].D();
		m_Result.angle = hv_Angle[i].D();
		m_Result.score = hv_Score[i].D();
		m_Result.isOK = true;
	}

	//显示
	try
	{
		int len = (hv_Score.TupleLength());
		int idx = 1;
		if (WindowHandle.Length())
			SetColor(WindowHandle, "green");
		HTuple hv_HomMat2D;
		HObject ho_TransContours;
		for (idx = 0; idx < len; idx++)
		{
			HomMat2dIdentity(&hv_HomMat2D);
			HalconCpp::HomMat2dRotate(hv_HomMat2D, HTuple(hv_Angle[idx]), 0, 0, &hv_HomMat2D);
			HalconCpp::HomMat2dTranslate(hv_HomMat2D, HTuple(hv_Row[idx]), HTuple(hv_Column[idx]),
				&hv_HomMat2D);
			HalconCpp::AffineTransContourXld(m_hoModelContours, &ho_TransContours, hv_HomMat2D);
			HObject ho_ModelRegion_Trans;
			//HalconCpp::AffineTransContourXld(m_ho_ModelRegion, &ho_ModelRegion_Trans, hv_HomMat2D);
			DispObj(ho_TransContours, WindowHandle);
			m_hoShape = ho_TransContours;
			//if (ImProcChecker::g_hoContoursAll.IsInitialized())
			//{
			//	HalconCpp::HTuple num_obj;
			//	HalconCpp::CountObj(ImProcChecker::g_hoContoursAll, &num_obj);
			//	if (num_obj < 100)
			//	{
			//		HalconCpp::ConcatObj(ImProcChecker::g_hoContoursAll, ho_TransContours, &ImProcChecker::g_hoContoursAll);
			//	}
			//}
		}
	}
	catch (HalconCpp::HException & except)
	{
		ShowException(except);
		SetColor(WindowHandle, "red");
		SetTposition(WindowHandle, HTuple(m_TextRow), HTuple(m_TextCol));
		WriteString(WindowHandle, "变换显示错误，请检查图像");
		return FALSE;
	}

	return TRUE;
}

BOOL ImProcChecker::CheckDeformShape(HObject & Img)
{
	//使用可形变形状匹配
	m_Result.isOK = false;

	//2.识别
	HTuple  hv_ScaleROffset, hv_ScaleCOffset,hv_Area;
	HTuple  hv_HomMat2D, hv_MinScore;
	HTuple hv_Row, hv_Column, hv_Angle, hv_Score, hv_Scale;
	xPara findPara;
	findPara.MinScore = m_Para.paraN[0];

	hv_ScaleROffset = 0.1;
	hv_ScaleCOffset = 0.0;
	try
	{
		HObject ho_Img = Img;
		//截取感兴趣区域
		if (m_ho_DetectArea.IsInitialized())
		{
			ReduceDomain(Img, m_ho_DetectArea, &ho_Img);
		}

		FindPlanarUncalibDeformableModel(ho_Img, m_hvDeformModelID, HTuple(-0).TupleRad(),
			HTuple(360).TupleRad(), 1, 1 + hv_ScaleROffset, 1, 1 + hv_ScaleCOffset, findPara.MinScore,
			1, 1, 0, 0.9, HTuple(), HTuple(), &hv_HomMat2D, &hv_Score);
	}
	catch (HalconCpp::HException & except)
	{
		if (WindowHandle.Length())
		{
			SetTposition(WindowHandle, HTuple(m_TextRow), HTuple(m_TextCol));
			SetColor(WindowHandle, "red");
			WriteString(WindowHandle, "Find Error:算法错误，请检查图像");
		}

		ShowException(except);
		return FALSE;
	}

	//3.判定
	//显示结果并返回
	int iNum = hv_Score.TupleLength();
	if (iNum == 0)
	{
		SetTposition(WindowHandle, HTuple(m_TextRow), HTuple(m_TextCol));
		//SetColor(WindowHandle, "red");
		//WriteString(WindowHandle, "Detect Failed:找不到目标，请检查图像或光源");
		return FALSE;
	}
	for (int i = 0; i < iNum; i++)
	{
		m_Result.isOK = true;
	}

	//显示
	try
	{
		if (WindowHandle.Length())
			SetColor(WindowHandle, "green");
		HObject ho_TransContours;

		ProjectiveTransContourXld(m_hoDeformModelContours, &ho_TransContours, hv_HomMat2D);
		DispObj(ho_TransContours, WindowHandle);
		m_hoDeformShape = ho_TransContours;
	}
	catch (HalconCpp::HException & except)
	{
		ShowException(except);
		SetColor(WindowHandle, "red");
		SetTposition(WindowHandle, HTuple(m_TextRow), HTuple(m_TextCol));
		WriteString(WindowHandle, "变换显示错误，请检查图像");
		return FALSE;
	}

	return TRUE;
}

BOOL ImProcChecker::CheckBlob(HObject &Img)
{
	//2.识别
	try
	{
		HObject ho_Regions, ho_RegionFillUp, ho_RegionClosing, ho_ConnectedRegions, ho_SelectedRegions, ho_ObjectSelected;
		HObject ho_RegionDilation;
		HTuple hv_Number, hv_Area, hv_Row, hv_Column;
		int  thr1 = (int)m_Para.paraN[0];
		int  thr2 = (int)m_Para.paraN[1];
		int  closingsize = (int)m_Para.paraN[2];
		int  minArea = (int)m_Para.paraN[3];
		int  maxArea = (int)m_Para.paraN[4];
		int  minRect = (int)m_Para.paraN[5];
		
		//去掉二维码区域
		HTuple h, w;
		HObject grayroi, ImageROI;
		GetImageSize(Img, &w, &h);
		GenRectangle1(&grayroi, 0, 0, h - 1, w - 1);
		if (g_hoBarCodeArea.IsInitialized())
		{
			HObject dilationbarcode;
			HalconCpp::DilationCircle(g_hoBarCodeArea, &dilationbarcode, 80);
			HalconCpp::Difference(grayroi, dilationbarcode/*g_hoBarCodeArea*/, &grayroi);
		}

		HObject ho_Img = Img;
		//截取感兴趣区域
		if (m_ho_DetectArea.IsInitialized())
		{
			ReduceDomain(Img, m_ho_DetectArea, &ho_Img);
		}

		ReduceDomain(ho_Img, grayroi, &ImageROI);
		Threshold(ImageROI, &ho_Regions, thr1, thr2);
		FillUp(ho_Regions, &ho_RegionFillUp);
		ClosingRectangle1(ho_RegionFillUp, &ho_RegionClosing, closingsize, closingsize);
		ErosionCircle(ho_RegionClosing, &ho_RegionDilation, 20);
		HalconCpp::Connection(ho_RegionDilation, &ho_ConnectedRegions);
		//SelectShape(ho_ConnectedRegions, &ho_SelectedRegions, "area", "and", minArea, 1e+006);
		SelectShape(ho_ConnectedRegions, &ho_SelectedRegions, (HTuple("area").Append("rectangularity").Append("convexity")),
			"and", (HTuple(minArea).Append(minRect).Append(0.7)), (HTuple(maxArea).Append(1).Append(1)));
		CountObj(ho_SelectedRegions, &hv_Number);
		if (0 != (hv_Number<1))
		{
			return FALSE;
		}

		//显示
		SelectObj(ho_SelectedRegions, &ho_ObjectSelected, 1);
		AreaCenter(ho_SelectedRegions, &hv_Area, &hv_Row, &hv_Column);
		m_Result.isOK = true;
		m_Result.area = hv_Area.D();
		m_Result.x = hv_Column.D();
		m_Result.y = hv_Row.D();
		DispObj(ho_ObjectSelected, WindowHandle);
		g_hoBlobArea = ho_ObjectSelected;
		//if (ImProcChecker::g_hoContoursAll.IsInitialized())
		//	HalconCpp::ConcatObj(ImProcChecker::g_hoContoursAll, ho_ObjectSelected, &ImProcChecker::g_hoContoursAll);
	}
	catch (HalconCpp::HException & except)
	{
		ShowException(except);
		SetColor(WindowHandle, "red");
		SetTposition(WindowHandle, HTuple(m_TextRow), HTuple(m_TextCol));
		WriteString(WindowHandle, "算法错误，请检查图像");
		return FALSE;
	}

	return TRUE;
}

BOOL ImProcChecker::CheckGray(HObject &Img)
{	
	m_Result.isOK = false;
	HObject grayroi; //计算灰度的区域
	try
	{
		HTuple w, h;
		GetImageSize(Img, &h, &w);
		GenRectangle1(&grayroi, 0, 0, h-1, w - 1);
		//if (m_hoBarCodeArea.IsInitialized())
		//{
		//	HalconCpp::Difference(grayroi, m_hoBarCodeArea, &grayroi);
		//}
		//2. 计算灰度
		HTuple hv_Value;
		double minGray = m_Para.paraN[0];
		GrayFeatures(grayroi, Img, "mean", &hv_Value);
		m_Result.gray = hv_Value.D();
	
		SetTposition(WindowHandle, HTuple(m_TextRow), HTuple(m_TextCol));

		//3. 判断
		if (hv_Value < minGray)
		{
			SetColor(WindowHandle, "red");
			WriteString(WindowHandle, hv_Value.ToString());
			return FALSE;
		}
		SetColor(WindowHandle, "green");
		WriteString(WindowHandle, hv_Value.ToString());
		m_Result.isOK = true;
	}
	catch (HalconCpp::HException & except)
	{
		ShowException(except);
		SetColor(WindowHandle, "red");
		SetTposition(WindowHandle, HTuple(m_TextRow), HTuple(m_TextCol));
		WriteString(WindowHandle, "算法错误，请检查图像");
		return FALSE;
	}
	return TRUE;
}

BOOL ImProcChecker::CheckBarcode(HObject &Img)
{
	m_Result.isOK = false;

	//2. 识别一维码
	try
	{
		HObject ho_Img = Img;
		//截取感兴趣区域
		if (m_ho_DetectArea.IsInitialized())
		{
			ReduceDomain(Img, m_ho_DetectArea, &ho_Img);
		}
		HObject ho_SymbolRegions;
		HTuple hv_DecodedDataStrings, hv_Number;
		int minlength = (int)m_Para.paraN[0];

		FindBarCode(ho_Img, &ho_SymbolRegions, m_hvBarCodeHandle, "auto"/*"EAN-13"*/, &hv_DecodedDataStrings);
	
		CountObj(ho_SymbolRegions, &hv_Number);
		if (0 != (hv_Number<1))
		{
			return FALSE;
		}
		DispObj(ho_SymbolRegions, WindowHandle);
		g_hoBarCodeArea = ho_SymbolRegions;
		//if(ImProcChecker::g_hoContoursAll.IsInitialized())
		//	ImProcChecker::g_hoContoursAll.Clear();
		//GenEmptyObj(&ImProcChecker::g_hoContoursAll);
		//HalconCpp::ConcatObj(ImProcChecker::g_hoContoursAll, ho_SymbolRegions, &ImProcChecker::g_hoContoursAll);
		//
		
		std::string code = hv_DecodedDataStrings.S();
		if (code.length() > minlength)
		{
			m_Result.isOK = true;
			m_Result.code = code;
			return TRUE;
		}
		else
			return FALSE;
	}
	catch (HalconCpp::HException & except)
	{
		ShowException(except);
		SetColor(WindowHandle, "red");
		SetTposition(WindowHandle, HTuple(m_TextRow), HTuple(m_TextCol));
		WriteString(WindowHandle, "找不到条形码");
		return FALSE;
	}

	return TRUE;
}

BOOL ImProcChecker::MeasureUSB()
{
	if (!ho_Image.IsInitialized())
	{
		AfxMessageBox(_T("请先加载图像"));
		return 0;
	}
	//
	HTuple hv_deviation_value1, hv_deviation_value2, hv_deviation_value3, hv_factor, hv_half_margin,
		hv_isError, hv_ErrorCode, hv_length,
		hv_width, hv_angle, hv_center_x, hv_center_y,
		hv_dis_rr, hv_up_margin, hv_down_margin;
	//
	//孔的灰度相对于参考平面的偏移量
	//hv_deviation_value1 = 50;
	hv_deviation_value1 = (int)m_Para.paraN[0];
	//上下边缘灰度相对于参考平面的偏移量
	hv_deviation_value2 = (int)m_Para.paraN[1];//10;
	hv_deviation_value3 = (int)m_Para.paraN[2];//10;
	//像素与物理尺寸的比例， mm/pix
	hv_factor = m_Para.paraN[3];//0.00495589;
	//孔位距离上缘的框选距离
	hv_half_margin = (int)m_Para.paraN[4];// 230;
	HTuple  hv_thr, hv_sigma;
	hv_thr = m_Para.paraN[5];
	hv_sigma = m_Para.paraN[6];
	//
	//DoubleCircle(ho_Image, m_ho_DetectArea, m_ho_ReferArea,
	//hv_deviation_value1, hv_deviation_value2, hv_factor, hv_half_margin,
	//	WindowHandle, &hv_isError, &hv_ErrorCode, &hv_length,
	//	&hv_width, &hv_angle, &hv_center_x, &hv_center_y,
	//	&hv_dis_rr, &hv_up_margin, &hv_down_margin);
	DoubleCircle2(ho_Image, m_ho_DetectArea, m_ho_ReferArea, hv_deviation_value1, hv_deviation_value2,
		hv_deviation_value3, hv_factor, hv_half_margin, WindowHandle, hv_thr, hv_sigma, &hv_isError,
		&hv_ErrorCode, &hv_length, &hv_width, &hv_angle, &hv_center_x, &hv_center_y,
		&hv_dis_rr, &hv_up_margin, &hv_down_margin);
	if (hv_isError == 0)
	{
		m_Result_measure.isOK = true;
		m_Result_measure.length = hv_length.D();
		m_Result_measure.width = hv_width.D();
		m_Result_measure.angle = hv_angle.D();
		m_Result_measure.cent_x = hv_center_x.D();
		m_Result_measure.cent_y = hv_center_y.D();
		m_Result_measure.dis_rr = hv_dis_rr.D();
		m_Result_measure.up_margin = hv_up_margin.D();
		m_Result_measure.dn_margin = hv_down_margin.D();
		m_Result_measure.errorCode = 0;
		return 0;
	}
	else
	{
		m_Result_measure.isOK = true;
		m_Result_measure.errorCode = hv_ErrorCode;
		return hv_ErrorCode;
	}
	
}

BOOL ImProcChecker::MeasureCircle()
{
	if (!ho_Image.IsInitialized())
	{
		AfxMessageBox(_T("请先加载图像"));
		return 0;
	}
	//
	HTuple hv_deviation_value1, hv_deviation_value2, hv_factor, hv_half_margin,
		hv_isError, hv_ErrorCode, hv_center_x, hv_center_y, hv_Length;
	//	hv_width, hv_angle, hv_center_x, hv_center_y,
	//	hv_dis_rr, hv_up_margin, hv_down_margin;
	//
	//孔的灰度相对于参考平面的偏移量
	//hv_deviation_value1 = 50;
	hv_deviation_value1 = (int)m_Para.paraN[0];
	//上下边缘灰度相对于参考平面的偏移量
	hv_deviation_value2 = (int)m_Para.paraN[1];//10;
											   //像素与物理尺寸的比例， mm/pix
	hv_factor = m_Para.paraN[2];//0.00495589;
								//孔位距离上缘的框选距离
	hv_half_margin = (int)m_Para.paraN[3];// 230;
										  //
	//DoubleCircle(ho_Image, m_ho_DetectArea, m_ho_ReferArea,
	//	hv_deviation_value1, hv_deviation_value2, hv_factor, hv_half_margin,
	//	WindowHandle, &hv_isError, &hv_ErrorCode, &hv_length,
	//	&hv_width, &hv_angle, &hv_center_x, &hv_center_y,
	//	&hv_dis_rr, &hv_up_margin, &hv_down_margin);
	SingleCircle(ho_Image, m_ho_DetectArea, m_ho_ReferArea, hv_deviation_value1, hv_deviation_value2,
		hv_factor, WindowHandle, &hv_Length, &hv_center_x, &hv_center_y, &hv_isError, &hv_ErrorCode);
	if (hv_isError == 0)
	{
		m_Result_measure.isOK = true;
		m_Result_measure.length = hv_Length.D();
		m_Result_measure.cent_x = hv_center_x.D();
		m_Result_measure.cent_y = hv_center_y.D();
		m_Result_measure.errorCode = 0;
		return 0;
	}
	else
	{
		m_Result_measure.isOK = true;
		m_Result_measure.errorCode = hv_ErrorCode;
		return hv_ErrorCode;
	}
	return 0;
}

bool ImProcChecker::GrabOneAndShow()
{
	if (!m_pCam) return false;
	unsigned char * pData = NULL;
	MV_FRAME_OUT_INFO_EX stImageInfo = { 0 };
	g_criSection.Lock();
	m_pCam->GrabOneImage(&pData, stImageInfo);
	g_criSection.Unlock();
	if (pData)
	{
		try
		{
			HalconCpp::HObject Image;
			ConvertoHalcon(pData, stImageInfo, Image);
			DispObj(Image, WindowHandle);
			ho_Image = Image.Clone();
			//WriteImage(ho_Image, "bmp", 0, "CapOne");

		}
		catch (HalconCpp::HException & except)
		{
			SetTposition(WindowHandle, HTuple(m_TextRow), HTuple(m_TextCol));
			SetColor(WindowHandle, "red");
			CString str(except.ErrorMessage());
			str.Replace(_T("HALCON"), _T(""));
			//MessageBoxW((HWND)hl_wnd, str, _T("Error"), MB_OK);
			WriteString(WindowHandle, "GetImage Error:图像转换失败");
			return false;
		}

	}
	else
	{
		if (WindowHandle.Length())
		{
			SetTposition(WindowHandle, HTuple(m_TextRow), HTuple(m_TextCol));
			SetColor(WindowHandle, "red");
			WriteString(WindowHandle, "GetImage Nothing:获取不到图像，请检查相机");
			return false;
		}
	}
	return true;
}

bool ImProcChecker::SaveImage(char * path)
{
	if (ho_Image.IsInitialized())
	{
		WriteImage(ho_Image, "bmp", 0, "CapOne");
	}
	return true;
}

bool ImProcChecker::Show()
{
	if (WindowHandle.Length())
	{
		if (ho_Image.IsInitialized())
		{
			DispObj(ho_Image, WindowHandle);
			return true;
		}
	}
	return false;
}

int ImProcChecker::ConvertoHalcon(unsigned char * pData, MV_FRAME_OUT_INFO_EX stImageInfo, HalconCpp::HObject& image)
{
	if (NULL == m_pImageBuffer)//Allocates the cache needed to transform the image，Mono8 and RGB are no need to allocate memory
	{
		if (PixelType_Gvsp_RGB8_Packed != stImageInfo.enPixelType && PixelType_Gvsp_Mono8 != stImageInfo.enPixelType)
		{
			m_pImageBuffer = (unsigned char *)malloc(sizeof(unsigned char) * stImageInfo.nWidth * stImageInfo.nHeight * 3);
		}
	}

	int nRet = 0;
	unsigned char* pTemp = NULL;
	if (IsColorPixelFormat(stImageInfo.enPixelType))//Color
	{
		if (NULL == m_pDataSeparate)
		{
			m_pDataSeparate = (unsigned char *)malloc(sizeof(unsigned char) * stImageInfo.nHeight * stImageInfo.nWidth * 3);
		}
		if (PixelType_Gvsp_RGB8_Packed == stImageInfo.enPixelType)
		{
			pTemp = pData;
		}
		else
		{
			nRet = ConvertToRGB(m_pCam->m_pcMyCamera->m_hDevHandle, pData, stImageInfo.nHeight, stImageInfo.nWidth, stImageInfo.enPixelType, m_pImageBuffer);
			if (MV_OK != nRet)
			{
				return -1;
			}
			pTemp = m_pImageBuffer;
		}

		nRet = ConvertRGBToHalcon(&image, stImageInfo.nHeight, stImageInfo.nWidth, stImageInfo.enPixelType, pTemp, m_pDataSeparate);
		if (MV_OK != nRet)
		{
			return -1;
		}
	}
	else if (IsMonoPixelFormat(stImageInfo.enPixelType))//Mono
	{
		if (PixelType_Gvsp_Mono8 == stImageInfo.enPixelType)
		{
			pTemp = pData;
		}
		else
		{
			nRet = ConvertToMono8(m_pCam->m_pcMyCamera->m_hDevHandle, pData, m_pImageBuffer, stImageInfo.nHeight, stImageInfo.nWidth, stImageInfo.enPixelType);
			if (MV_OK != nRet)
			{
				return -1;
			}
			pTemp = m_pImageBuffer;
		}

		nRet = ConvertMono8ToHalcon(&image, stImageInfo.nHeight, stImageInfo.nWidth, stImageInfo.enPixelType, pTemp);
		if (MV_OK != nRet)
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
	//HalconDisplay(hWindow, Hobj, stImageInfo.nHeight, stImageInfo.nWidth);
	return nRet;
}
