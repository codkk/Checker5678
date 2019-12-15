#pragma once
//AdoDatabase.h文件
class AdoDatabase
{
public:
	AdoDatabase(void);
	~AdoDatabase(void);

	_ConnectionPtr m_pConnection;//连接数据库智能指针
	_variant_t RecordsAffected;//操作完成后所影响的行数
	_RecordsetPtr m_pRecordset; //记录集对象,执行返回结果存储的容器
	CString m_connectstr;  //连接命令
	CString m_errormessage;
								// 连接数据库
	BOOL ConnectDB(void);
	// 关闭数据库
	BOOL CloseConn(void);
	// 执行sql语句
	BOOL ExecuteSQL(CString sqlstr, CString typeStr);
	// 向数据库中插入数据
	BOOL InsertStrData(CString sqlstr);
	// 更新数据库数据
	BOOL UpdateStrData(CString sqlstr);
	// 查询数据库数据
	BOOL SelectStrData(CString sqlstr);
	// 删除
	BOOL DeleteStrData(CString sqlstr);
	// 查询语句执行
	BOOL ExecuteSelSQL(CString sqlstr);
};