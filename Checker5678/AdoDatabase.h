#pragma once
//AdoDatabase.h�ļ�
class AdoDatabase
{
public:
	AdoDatabase(void);
	~AdoDatabase(void);

	_ConnectionPtr m_pConnection;//�������ݿ�����ָ��
	_variant_t RecordsAffected;//������ɺ���Ӱ�������
	_RecordsetPtr m_pRecordset; //��¼������,ִ�з��ؽ���洢������
	CString m_connectstr;  //��������
	CString m_errormessage;
								// �������ݿ�
	BOOL ConnectDB(void);
	// �ر����ݿ�
	BOOL CloseConn(void);
	// ִ��sql���
	BOOL ExecuteSQL(CString sqlstr, CString typeStr);
	// �����ݿ��в�������
	BOOL InsertStrData(CString sqlstr);
	// �������ݿ�����
	BOOL UpdateStrData(CString sqlstr);
	// ��ѯ���ݿ�����
	BOOL SelectStrData(CString sqlstr);
	// ɾ��
	BOOL DeleteStrData(CString sqlstr);
	// ��ѯ���ִ��
	BOOL ExecuteSelSQL(CString sqlstr);
};