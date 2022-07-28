#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>

#include "buffer.h"
#include "SqlConnectPool.h"
class HTTPrequest
{
public:
    enum PARSE_STATE {
        LINE,
        HEADERS,
        BODY,
        FINISH,
    };
    enum HTTP_CODE {
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURSE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
    };
    HTTPrequest();
    ~HTTPrequest() = default;
    void init();
    bool parse(Buffer& buffer);//解析http请求
    
    //获取私有信息
    std::string path() const;
    std::string& path();
    std::string method() const;
    std::string version() const;
    std::string getPost(const std::string& key) const;
    std::string getPost(const char* key) const;
    //判断keepAlive
    bool isKeepAlive()const;

private:
    PARSE_STATE state;
    bool parseRequestLine(const std::string& line);//请求行
    void parseRequestHeader(const std::string& line);//请求头
    void parseRequestBody(const std::string& line);//请求体

    void parsePath();//处理路径信息
    void parsePost();//处理post报文

    static bool UesrCheck(const std::string& name,const std::string& pwd,bool isLogin);

    std::string _path, _method, _version, _body;//方法，路径，版本，实体
    static int convertHex(char ch);//格式转换


    std::unordered_map<std::string, std::string> header;
    std::unordered_map<std::string, std::string> post;

    static const std::unordered_set<std::string>DEAFAULT_HTML;
};
#endif
