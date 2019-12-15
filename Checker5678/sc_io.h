
#ifndef SC_IO_H__
#define SC_IO_H__

#include <wchar.h>

#if defined(_WIN32) || defined(_WIN64)
      #define NIO_API_EXPORT __declspec(dllexport)
      #define NIO_API_CALL __stdcall
#else
      #define NIO_API_EXPORT /**< API export macro */
      #define NIO_API_CALL /**< API call macro */
#endif

/**< API export and call macro*/
#define NIO_API_EXPORT_CALL NIO_API_EXPORT NIO_API_CALL

#define NIO

#ifdef __cplusplus
extern "C" {
#endif

/********************************宏定义****************************************/

#define MAX_SUPPORT_DEVICES 16


/* 指令返回值定义 */

#define RTN_CMD_SUCCESS          0        //指令执行成功
#define RTN_CMD_ERROR           -1        //指令执行失败
#define RTN_LIB_PARA_ERROR      -2        //指令参数错误
#define RTN_NO_SUPPORT          -3        //不支持
#define RTN_INVALID_HANDLE      -4        //无效句柄

/********************************类型定义**************************************/
typedef unsigned int U32;
typedef signed int S32;
typedef unsigned short U16;
typedef signed short S16;
typedef unsigned char U8;
typedef signed char S8;
typedef char B8;

//设备信息结构
typedef struct
{
	U16 address;              // 在上位机系统中分配的地址序号,如USB
	U8  idStr[16];            // 识别字符串
	U8  description[64];      // 描述符
	U16 ID;                   // 板上的ID(未用)
}TDevInfo;

typedef struct _CALENDAR_{
	U16 w_year;
	U8 month;
	U8 day;
	U8 week;
	U8 hour;
	U8 min;
	U8 sec;
}CALENDAR;

// 设备句柄定义
#if defined(_WIN64)
	typedef long long HAND;
	typedef long long * PHAND;
#elif defined(_WIN32)
	typedef unsigned long HAND;
	typedef unsigned long * PHAND;
#else
	#error "Just support 32bit or 64bit compiler"
#endif
/********************************函数声明**************************************/
/*******************************************************************************
* 函数名称: NIO_Search
* 功能说明: 板卡搜寻
* 入口参数: pDevNo:     用于接收搜寻到的设备的数目
* 出口参数: 指令执行状态
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_Search(U16 *pDevNo, TDevInfo *pInfoList);

/*******************************************************************************
* 函数名称: NIO_OpenByID
* 功能说明: 根据SIO 设备在系统中名称，打开相应的设备。
* 入口参数: idStr:      设备名称
            pDevHandle: 用于接收设备操作句柄
* 出口参数: 指令执行成功
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_OpenByID(
            const B8 *idStr, PHAND pDevHandle);

/*******************************************************************************
* 函数名称: NIO_Close
* 功能说明: 关闭打开的设备
* 入口参数: devHandle:  设备操作句柄
* 出口参数: 指令执行状态
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_Close(HAND devHandle);

/*******************************************************************************
* 函数名称: NIO_GetDO
* 功能说明: 获取设备点位输出状态
* 入口参数: devHandle:  设备操作句柄
            portStart:  起始端口号, 取值范围(0-1)
            portCount:  要获取的端口数量, 取值范围(1-2)
            value:      用于存储端口状态值缓冲(每个端口16个点位), 每个点位: 
                        1 代表触发; 0 代表未触发
* 出口参数: 指令执行状态
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_GetDO(
            HAND devHandle, U16 portStart, U16 portCount, U16 *value);

/*******************************************************************************
* 函数名称: NIO_SetDO
* 功能说明: 设置设备输出端口状态
* 入口参数: devHandle:  设备操作句柄
            portStart:  起始端口号, 取值范围(0-1)
            portCount:  要获取的端口数量, 取值范围(1-2)
            value:      要设置的端口状态值缓冲(每个端口16个点位), 每个点位: 
                        1 代表触发; 0 代表不触发
* 出口参数: 指令执行状态
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_SetDO(
            HAND devHandle, U16 portStart, U16 portCount, const U16 *value);

/*******************************************************************************
* 函数名称: NIO_GetDOBit
* 功能说明: 获取设备单个点位输出状态
* 入口参数: devHandle:  设备操作句柄
            channel:    点位位置, 取值范围: 0-31
            value:      用于存储获取的点位状态, 1 代表触发; 0 代表未触发
* 出口参数: 指令执行状态
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_GetDOBit(
            HAND devHandle, U16 channel, U16 *value);

/*******************************************************************************
* 函数名称: NIO_SetDOBit
* 功能说明: 设置设备单个点位输出状态
* 入口参数: devHandle:  设备操作句柄
            channel:    点位位置, 取值范围: 0-31
            value:      要设置的值, 1 代表触发; 0 代表不触发
* 出口参数: 指令执行状态
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_SetDOBit(
            HAND devHandle, U16 channel, U16 value);

/*******************************************************************************
* 函数名称: NIO_GetDI
* 功能说明: 获取设备输入状态，按端口获取
* 入口参数: devHandle:  设备操作句柄
            portStart:  起始端口号, 取值范围(0-1)
            portCount:  要获取的端口数量, 取值范围(1-2)
            value:      用于存储端口状态值缓冲(每个端口16个点位), 每个点位: 
                        1 代表有效（接通）; 0 代表无效（未接通）
* 出口参数: 指令执行状态
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_GetDI(
            HAND devHandle, U16 portStart, U16 portCount, U16 *value);

/*******************************************************************************
* 函数名称: NIO_GetDIBit
* 功能说明: 获取设备单个点位输入状态
* 入口参数: devHandle:  设备操作句柄
            channel:    点位位置, 取值范围: 0-31
            value:      用于存储获取的点位状态, 1 代表有效; 0 代表无效
* 出口参数: 指令执行状态
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_GetDIBit(
            HAND devHandle, U16 channel, U16 *value);

/*******************************************************************************
* 函数名称: NIO_GetTime
* 功能说明: 获取设备时期及时间
* 入口参数: devHandle:  设备操作句柄
            date:       用于存储日期时间的缓冲
* 出口参数: 指令执行状态
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_GetTime(
            HAND devHandle, CALENDAR *date);

/*******************************************************************************
* 函数名称: NIO_WriteID
* 功能说明: 修改SIO 设备在系统中名称
* 入口参数: devHandle:  操作句柄
            idStr:      新的设备名称, 最长16字节包括末尾'\0'字符
* 出口参数: 指令执行状态
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_WriteID(
            HAND devHandle, const B8 *idStr );

/*******************************************************************************
* 函数名称: NIO_ReadID
* 功能说明: 读取SIO 设备名称
* 入口参数: devHandle:  设备操作句柄
            productId:  用于存放设备名称缓冲区, 最长16字节
* 出口参数: 指令执行状态
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_ReadID(
            HAND devHandle, B8 *productId );

/*******************************************************************************
* 函数名称: NIO_WriteDevID
* 功能说明: 修改SIO 设备ID序号（用于Modbus通讯的设备ID）
* 入口参数: devHandle:  操作句柄
            idStr:      新的设备序号
* 出口参数: 指令执行状态
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_WriteDevID(
            HAND devHandle, const U8 devId );

/*******************************************************************************
* 函数名称: NIO_ReadDevID
* 功能说明: 读取SIO 设备ID序号（用于Modbus通讯的设备ID）
* 入口参数: devHandle:  设备操作句柄
            productId:  用于存放设备ID序号
* 出口参数: 指令执行状态
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_ReadDevID(
            HAND devHandle, U8 *devId );

/*******************************************************************************
* 函数名称: NIO_ReadProductID
* 功能说明: 获取产品唯一ID
* 入口参数: devHandle:  设备操作句柄
            productId:  用于存放设备ID的缓冲(12字节共96位)
* 出口参数: 指令执行状态
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_ReadProductID(
            HAND devHandle, U16 *productId);

/*******************************************************************************
* 函数名称: NIO_ReadUserData
* 功能说明: 获取用户数据
* 入口参数: devHandle:  设备操作句柄
            userData:   用于存放用户数据的缓冲(84字节)
* 出口参数: 指令执行状态
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_ReadUserData(
            HAND devHandle, B8 *userData);
/*******************************************************************************
* 函数名称: NIO_WriteUserData
* 功能说明: 存储用户数据
* 入口参数: devHandle:  设备操作句柄
            userData:   用于存放用户数据的缓冲(最多存储84字节, 可用于存储加密)
						注意：数据一次性读出和写入，超出部分将忽略
* 出口参数: 指令执行状态
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_WriteUserData(
            HAND devHandle, const B8 *userData);

/*******************************************************************************
* 函数名称: NIO_GetIOLogic
* 功能说明: 获取设备IO口逻辑
* 入口参数: devHandle:  设备操作句柄
value:      用于存储IO逻辑状态
* 出口参数: 指令执行状态
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_GetIOLogic(HAND devHandle, U16 *value);

/*******************************************************************************
* 函数名称: NIO_SetIOLogic
* 功能说明: 设置设备IO口逻辑
* 入口参数: devHandle:  设备操作句柄
value:      要设置的IO逻辑状态（1，正逻辑；0，负逻辑）
* 出口参数: 指令执行状态
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_SetIOLogic(HAND devHandle, U16 value);
/*******************************************************************************
* 函数名称: NIO_GetError
* 功能说明: SC设备操作过程中，最后出现的错误，以字符串的形式提供
* 入口参数: handle:     设备句柄
            str:        用于存储错误信息的缓冲
            len:        缓冲区大小
* 出口参数: 成功返回错误字符串，失败返回NULL
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_GetError(
            HAND devHandle, B8 *str, U16 len);


#ifdef __cplusplus
}
#endif

#endif

