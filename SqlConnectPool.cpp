#include "SqlConnectPool.h"

SqlConnect* SqlConnect::Instance()
{
    static SqlConnect Connects;
    return &Connects;
}

MYSQL* SqlConnect::GetConnect()
{
    MYSQL* sql = nullptr;
    if (Q.empty()) {
        std::cout << "SQL busy!!!" << std::endl;
        return nullptr;
    }
    sem_wait(&m_sem);
    {
        std::lock_guard<std::mutex> lk(m_mutex);
        sql = Q.front();
        Q.pop();
    }
    return sql; 
}

void SqlConnect::FreeConnect(MYSQL* sql)
{
    assert(sql);
    std::lock_guard<std::mutex> lk(m_mutex);
    Q.push(sql);
    sem_post(&m_sem);
}

int SqlConnect::GetFreeConnectCnt()
{
    std::lock_guard<std::mutex> lk(m_mutex);
    return Q.size();
}

void SqlConnect::Init(const char* host, int port, const char* user, const char* pwd, const char* db, int Size)
{
    assert(Size > 0);
    for (int i = 0; i < Size; i++) {
        MYSQL *sql = nullptr;
        sql = mysql_init(sql);
        if (!sql) {
            std::cout << "Mysql init error!" << std::endl;
            assert(sql);
        }
        sql = mysql_real_connect(sql, host, user, pwd, db, port, nullptr, 0);
        if (!sql) std::cout << "Mysql Connect error!" << std::endl;
        Q.push(sql);
    }
    MAX_CON = Size;
    sem_init(&m_sem,0,MAX_CON);
}

void SqlConnect::Close()
{
    std::lock_guard<std::mutex> lk(m_mutex);
    while (!Q.empty()) {
        MYSQL* item = Q.front();
        Q.pop();
        mysql_close(item);
    }
    mysql_library_end();
}

