#include "StdAfx.h"
#include "AdoDatabase.h"


AdoDatabase::AdoDatabase(void)
{
}


AdoDatabase::~AdoDatabase(void)
{
}

// �������ݿ�
BOOL AdoDatabase::ConnectDB(void)
{
	::CoInitialize(NULL);
	try
	{
		//m_pConnection.CreateInstance(__uuidof(Connection));
		//m_pConnection->Open("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=Northwind.mdb","","",adModeUnknown);

		m_pConnection.CreateInstance(_T("ADODB.Connection"));//����connections����
		//_bstr_t strConnect = "Provider=SQLOLEDB.1;User ID=cqf;Password=123;Persist Security Info =false;Data Source=10.6.11.38,1433;Initial Catalog=lanmonitor";
		//_bstr_t strConnect = "Provider=SQLOLEDB;Server=mfcetn03;Database=VEN;uid=etuser;pwd=mflex123";
		_bstr_t strConnect = m_connectstr;
		/*
		_bstr_t strConnect = ��Provider=SQLOLEDB; 
		Server=mfcetn03; //�������򱾵ؼ������
		Database=VEN; //Databaseָ��ϵͳ�е����ݿ�
		uid=etuser; //�û���
		pwd=mflex123��; //����  ���ݿ����: ET_Record_Temp
		*/
		m_pConnection->Open(strConnect, "", "", adModeUnknown);
	}
	catch (_com_error e)
	{
		CString errormessage;
		errormessage.Format(_T("�������ݿ�ʧ��!\r������Ϣ:%s"), e.ErrorMessage());
		//AfxMessageBox(errormessage);
		m_errormessage = errormessage;
		return -1;
	}

	return 0;
}


// �ر����ݿ�
BOOL AdoDatabase::CloseConn(void)
{
	if (m_pRecordset != NULL)//�����Ϊ����ر����ݼ�
	{
		m_pRecordset->Close();
		m_pRecordset = NULL;
	}
	if (m_pConnection->State)
	{
		m_pConnection->Close();
		m_pConnection = NULL;
	}
	return 0;
}


// ִ��sql���
BOOL AdoDatabase::ExecuteSQL(CString sqlstr, CString typeStr)
{
	try
	{
		_bstr_t strCmd = sqlstr;//"INSERT INTO Employees(EmployeeID,FirstName,LastName,HireDate,City,Country) VALUES(10,'Mary','Williams','15/4/1993 12:00:00','New York','USA')";
		m_pConnection->Execute(strCmd, &RecordsAffected, adCmdText);
	}
	catch (_com_error &e)
	{
		//AfxMessageBox(e.Description() + ":" + typeStr);
		m_errormessage = e.Description() + ":" + typeStr;
		return -1;
	}
	return 0;
}

// �����ݿ��в�������
BOOL AdoDatabase::InsertStrData(CString sqlstr)
{
	return 	ExecuteSQL(sqlstr, _T("Insert!"));
}


// �������ݿ�����
BOOL AdoDatabase::UpdateStrData(CString sqlstr)
{
	return ExecuteSQL(sqlstr, _T("Update!"));
}


// ��ѯ���ݿ�����
BOOL AdoDatabase::SelectStrData(CString sqlstr)
{
	m_pRecordset.CreateInstance("ADODB.Recordset"); //ΪRecordset���󴴽�ʵ��
	return  ExecuteSelSQL(sqlstr);
}



// ɾ��
BOOL AdoDatabase::DeleteStrData(CString sqlstr)
{
	return ExecuteSQL(sqlstr, _T("Select!"));
}


// ��ѯ���ִ��
BOOL AdoDatabase::ExecuteSelSQL(CString sqlstr)
{
	try
	{
		_bstr_t strCmd = sqlstr;//"INSERT INTO Employees(EmployeeID,FirstName,LastName,HireDate,City,Country) VALUES(10,'Mary','Williams','15/4/1993 12:00:00','New York','USA')";
		m_pRecordset = m_pConnection->Execute(strCmd, &RecordsAffected, adCmdText);
	}
	catch (_com_error &e)
	{
		//AfxMessageBox(e.Description());
		m_errormessage.Format(_T("%s"), e.Description());
		return -1;
	}
	return 0;
}