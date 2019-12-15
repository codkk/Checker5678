#include "StdAfx.h"
#include "AdoDatabase.h"


AdoDatabase::AdoDatabase(void)
{
}


AdoDatabase::~AdoDatabase(void)
{
}

// 连接数据库
BOOL AdoDatabase::ConnectDB(void)
{
	::CoInitialize(NULL);
	try
	{
		//m_pConnection.CreateInstance(__uuidof(Connection));
		//m_pConnection->Open("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=Northwind.mdb","","",adModeUnknown);

		m_pConnection.CreateInstance(_T("ADODB.Connection"));//创建connections对象
		//_bstr_t strConnect = "Provider=SQLOLEDB.1;User ID=cqf;Password=123;Persist Security Info =false;Data Source=10.6.11.38,1433;Initial Catalog=lanmonitor";
		//_bstr_t strConnect = "Provider=SQLOLEDB;Server=mfcetn03;Database=VEN;uid=etuser;pwd=mflex123";
		_bstr_t strConnect = m_connectstr;
		/*
		_bstr_t strConnect = “Provider=SQLOLEDB; 
		Server=mfcetn03; //服务器或本地计算机名
		Database=VEN; //Database指你系统中的数据库
		uid=etuser; //用户名
		pwd=mflex123”; //密码  数据库表名: ET_Record_Temp
		*/
		m_pConnection->Open(strConnect, "", "", adModeUnknown);
	}
	catch (_com_error e)
	{
		CString errormessage;
		errormessage.Format(_T("连接数据库失败!\r错误信息:%s"), e.ErrorMessage());
		//AfxMessageBox(errormessage);
		m_errormessage = errormessage;
		return -1;
	}

	return 0;
}


// 关闭数据库
BOOL AdoDatabase::CloseConn(void)
{
	if (m_pRecordset != NULL)//如果不为空则关闭数据集
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


// 执行sql语句
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

// 向数据库中插入数据
BOOL AdoDatabase::InsertStrData(CString sqlstr)
{
	return 	ExecuteSQL(sqlstr, _T("Insert!"));
}


// 更新数据库数据
BOOL AdoDatabase::UpdateStrData(CString sqlstr)
{
	return ExecuteSQL(sqlstr, _T("Update!"));
}


// 查询数据库数据
BOOL AdoDatabase::SelectStrData(CString sqlstr)
{
	m_pRecordset.CreateInstance("ADODB.Recordset"); //为Recordset对象创建实例
	return  ExecuteSelSQL(sqlstr);
}



// 删除
BOOL AdoDatabase::DeleteStrData(CString sqlstr)
{
	return ExecuteSQL(sqlstr, _T("Select!"));
}


// 查询语句执行
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