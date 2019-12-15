
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

/********************************�궨��****************************************/

#define MAX_SUPPORT_DEVICES 16


/* ָ���ֵ���� */

#define RTN_CMD_SUCCESS          0        //ָ��ִ�гɹ�
#define RTN_CMD_ERROR           -1        //ָ��ִ��ʧ��
#define RTN_LIB_PARA_ERROR      -2        //ָ���������
#define RTN_NO_SUPPORT          -3        //��֧��
#define RTN_INVALID_HANDLE      -4        //��Ч���

/********************************���Ͷ���**************************************/
typedef unsigned int U32;
typedef signed int S32;
typedef unsigned short U16;
typedef signed short S16;
typedef unsigned char U8;
typedef signed char S8;
typedef char B8;

//�豸��Ϣ�ṹ
typedef struct
{
	U16 address;              // ����λ��ϵͳ�з���ĵ�ַ���,��USB
	U8  idStr[16];            // ʶ���ַ���
	U8  description[64];      // ������
	U16 ID;                   // ���ϵ�ID(δ��)
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

// �豸�������
#if defined(_WIN64)
	typedef long long HAND;
	typedef long long * PHAND;
#elif defined(_WIN32)
	typedef unsigned long HAND;
	typedef unsigned long * PHAND;
#else
	#error "Just support 32bit or 64bit compiler"
#endif
/********************************��������**************************************/
/*******************************************************************************
* ��������: NIO_Search
* ����˵��: �忨��Ѱ
* ��ڲ���: pDevNo:     ���ڽ�����Ѱ�����豸����Ŀ
* ���ڲ���: ָ��ִ��״̬
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_Search(U16 *pDevNo, TDevInfo *pInfoList);

/*******************************************************************************
* ��������: NIO_OpenByID
* ����˵��: ����SIO �豸��ϵͳ�����ƣ�����Ӧ���豸��
* ��ڲ���: idStr:      �豸����
            pDevHandle: ���ڽ����豸�������
* ���ڲ���: ָ��ִ�гɹ�
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_OpenByID(
            const B8 *idStr, PHAND pDevHandle);

/*******************************************************************************
* ��������: NIO_Close
* ����˵��: �رմ򿪵��豸
* ��ڲ���: devHandle:  �豸�������
* ���ڲ���: ָ��ִ��״̬
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_Close(HAND devHandle);

/*******************************************************************************
* ��������: NIO_GetDO
* ����˵��: ��ȡ�豸��λ���״̬
* ��ڲ���: devHandle:  �豸�������
            portStart:  ��ʼ�˿ں�, ȡֵ��Χ(0-1)
            portCount:  Ҫ��ȡ�Ķ˿�����, ȡֵ��Χ(1-2)
            value:      ���ڴ洢�˿�״ֵ̬����(ÿ���˿�16����λ), ÿ����λ: 
                        1 ������; 0 ����δ����
* ���ڲ���: ָ��ִ��״̬
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_GetDO(
            HAND devHandle, U16 portStart, U16 portCount, U16 *value);

/*******************************************************************************
* ��������: NIO_SetDO
* ����˵��: �����豸����˿�״̬
* ��ڲ���: devHandle:  �豸�������
            portStart:  ��ʼ�˿ں�, ȡֵ��Χ(0-1)
            portCount:  Ҫ��ȡ�Ķ˿�����, ȡֵ��Χ(1-2)
            value:      Ҫ���õĶ˿�״ֵ̬����(ÿ���˿�16����λ), ÿ����λ: 
                        1 ������; 0 ��������
* ���ڲ���: ָ��ִ��״̬
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_SetDO(
            HAND devHandle, U16 portStart, U16 portCount, const U16 *value);

/*******************************************************************************
* ��������: NIO_GetDOBit
* ����˵��: ��ȡ�豸������λ���״̬
* ��ڲ���: devHandle:  �豸�������
            channel:    ��λλ��, ȡֵ��Χ: 0-31
            value:      ���ڴ洢��ȡ�ĵ�λ״̬, 1 ������; 0 ����δ����
* ���ڲ���: ָ��ִ��״̬
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_GetDOBit(
            HAND devHandle, U16 channel, U16 *value);

/*******************************************************************************
* ��������: NIO_SetDOBit
* ����˵��: �����豸������λ���״̬
* ��ڲ���: devHandle:  �豸�������
            channel:    ��λλ��, ȡֵ��Χ: 0-31
            value:      Ҫ���õ�ֵ, 1 ������; 0 ��������
* ���ڲ���: ָ��ִ��״̬
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_SetDOBit(
            HAND devHandle, U16 channel, U16 value);

/*******************************************************************************
* ��������: NIO_GetDI
* ����˵��: ��ȡ�豸����״̬�����˿ڻ�ȡ
* ��ڲ���: devHandle:  �豸�������
            portStart:  ��ʼ�˿ں�, ȡֵ��Χ(0-1)
            portCount:  Ҫ��ȡ�Ķ˿�����, ȡֵ��Χ(1-2)
            value:      ���ڴ洢�˿�״ֵ̬����(ÿ���˿�16����λ), ÿ����λ: 
                        1 ������Ч����ͨ��; 0 ������Ч��δ��ͨ��
* ���ڲ���: ָ��ִ��״̬
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_GetDI(
            HAND devHandle, U16 portStart, U16 portCount, U16 *value);

/*******************************************************************************
* ��������: NIO_GetDIBit
* ����˵��: ��ȡ�豸������λ����״̬
* ��ڲ���: devHandle:  �豸�������
            channel:    ��λλ��, ȡֵ��Χ: 0-31
            value:      ���ڴ洢��ȡ�ĵ�λ״̬, 1 ������Ч; 0 ������Ч
* ���ڲ���: ָ��ִ��״̬
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_GetDIBit(
            HAND devHandle, U16 channel, U16 *value);

/*******************************************************************************
* ��������: NIO_GetTime
* ����˵��: ��ȡ�豸ʱ�ڼ�ʱ��
* ��ڲ���: devHandle:  �豸�������
            date:       ���ڴ洢����ʱ��Ļ���
* ���ڲ���: ָ��ִ��״̬
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_GetTime(
            HAND devHandle, CALENDAR *date);

/*******************************************************************************
* ��������: NIO_WriteID
* ����˵��: �޸�SIO �豸��ϵͳ������
* ��ڲ���: devHandle:  �������
            idStr:      �µ��豸����, �16�ֽڰ���ĩβ'\0'�ַ�
* ���ڲ���: ָ��ִ��״̬
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_WriteID(
            HAND devHandle, const B8 *idStr );

/*******************************************************************************
* ��������: NIO_ReadID
* ����˵��: ��ȡSIO �豸����
* ��ڲ���: devHandle:  �豸�������
            productId:  ���ڴ���豸���ƻ�����, �16�ֽ�
* ���ڲ���: ָ��ִ��״̬
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_ReadID(
            HAND devHandle, B8 *productId );

/*******************************************************************************
* ��������: NIO_WriteDevID
* ����˵��: �޸�SIO �豸ID��ţ�����ModbusͨѶ���豸ID��
* ��ڲ���: devHandle:  �������
            idStr:      �µ��豸���
* ���ڲ���: ָ��ִ��״̬
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_WriteDevID(
            HAND devHandle, const U8 devId );

/*******************************************************************************
* ��������: NIO_ReadDevID
* ����˵��: ��ȡSIO �豸ID��ţ�����ModbusͨѶ���豸ID��
* ��ڲ���: devHandle:  �豸�������
            productId:  ���ڴ���豸ID���
* ���ڲ���: ָ��ִ��״̬
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_ReadDevID(
            HAND devHandle, U8 *devId );

/*******************************************************************************
* ��������: NIO_ReadProductID
* ����˵��: ��ȡ��ƷΨһID
* ��ڲ���: devHandle:  �豸�������
            productId:  ���ڴ���豸ID�Ļ���(12�ֽڹ�96λ)
* ���ڲ���: ָ��ִ��״̬
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_ReadProductID(
            HAND devHandle, U16 *productId);

/*******************************************************************************
* ��������: NIO_ReadUserData
* ����˵��: ��ȡ�û�����
* ��ڲ���: devHandle:  �豸�������
            userData:   ���ڴ���û����ݵĻ���(84�ֽ�)
* ���ڲ���: ָ��ִ��״̬
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_ReadUserData(
            HAND devHandle, B8 *userData);
/*******************************************************************************
* ��������: NIO_WriteUserData
* ����˵��: �洢�û�����
* ��ڲ���: devHandle:  �豸�������
            userData:   ���ڴ���û����ݵĻ���(���洢84�ֽ�, �����ڴ洢����)
						ע�⣺����һ���Զ�����д�룬�������ֽ�����
* ���ڲ���: ָ��ִ��״̬
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_WriteUserData(
            HAND devHandle, const B8 *userData);

/*******************************************************************************
* ��������: NIO_GetIOLogic
* ����˵��: ��ȡ�豸IO���߼�
* ��ڲ���: devHandle:  �豸�������
value:      ���ڴ洢IO�߼�״̬
* ���ڲ���: ָ��ִ��״̬
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_GetIOLogic(HAND devHandle, U16 *value);

/*******************************************************************************
* ��������: NIO_SetIOLogic
* ����˵��: �����豸IO���߼�
* ��ڲ���: devHandle:  �豸�������
value:      Ҫ���õ�IO�߼�״̬��1�����߼���0�����߼���
* ���ڲ���: ָ��ִ��״̬
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_SetIOLogic(HAND devHandle, U16 value);
/*******************************************************************************
* ��������: NIO_GetError
* ����˵��: SC�豸���������У������ֵĴ������ַ�������ʽ�ṩ
* ��ڲ���: handle:     �豸���
            str:        ���ڴ洢������Ϣ�Ļ���
            len:        ��������С
* ���ڲ���: �ɹ����ش����ַ�����ʧ�ܷ���NULL
*******************************************************************************/
NIO_API_EXPORT short NIO_API_CALL NIO_GetError(
            HAND devHandle, B8 *str, U16 len);


#ifdef __cplusplus
}
#endif

#endif

