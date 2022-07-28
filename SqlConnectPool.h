#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <iostream>
#include <semaphore.h>
#include <thread>
#include <assert.h>

class SqlConnect {
public:
    static SqlConnect* Instance();
	
    MYSQL* GetConnect();
    void FreeConnect(MYSQL* conn);
    int GetFreeConnectCnt();

    void Init(const char* host, int port,
        const char* user, const char* pwd,
        const char* db, int Size);
    void Close();
	
private:
    SqlConnect() { useCnt = 0; freeCnt = 0; }
    ~SqlConnect() { Close(); }

    int MAX_CON;
    int useCnt;
    int freeCnt;

    std::queue<MYSQL*> Q;
    std::mutex m_mutex;
    sem_t m_sem;
};
class SqlRaii{//通过raii将sql绑定
public:
	SqlRaii(MYSQL** _sql,SqlConnect *_SqlPool){
		assert(_SqlPool);
		*_sql = _SqlPool->GetConnect();
		SqlPool = _SqlPool;
		sql = *_sql;
	}
	~SqlRaii(){
		if(sql) SqlPool->FreeConnect(sql);
	}
private:
	MYSQL *sql;
	SqlConnect* SqlPool;
};
#endif // SQLCONNPOOL_H
