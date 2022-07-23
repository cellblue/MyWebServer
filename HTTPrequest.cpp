#include "HTTPrequest.h"

const std::unordered_set<std::string>HTTPrequest::DEAFAULT_HTML{
    "/index","/welcome","/video","/picture"
};
HTTPrequest::HTTPrequest(){
    init();
}
void HTTPrequest::init()
{
    _method = _path = _version = _body = "";
    state = LINE;
    header.clear();
    post.clear();
}
bool HTTPrequest::parse(Buffer& buffer)
{
    //解析每一行
    const char CRLF[] = "\r\n";
    if (buffer.readAbleBytes() <= 0) return false;

    while (buffer.readAbleBytes() && state != FINISH) {
        const char* End = std::search(buffer.curReadPtr(),buffer.curWritePtrConst(),CRLF,CRLF+2);
        std::string line(buffer.curReadPtr(),End);
	  std::cout<<line<<std::endl;
        switch(state) {
            case LINE:
                //解析行
                if (!parseRequestLine(line)) return false;
                parsePath();
                break;
            case HEADERS:
                parseRequestHeader(line);
                if (buffer.readAbleBytes() <= 2) {
                    state = FINISH;
                }
                break;
            case BODY:
                parseRequestBody(line);
                break;
            default:
                break;
        }
        if (End == buffer.curWritePtr()) break;
        buffer.updateReadPtrToNow(End+2);
    }
    return true;
}
std::string HTTPrequest::path() const{
    return _path;
}

std::string& HTTPrequest::path()
{
    return _path;
}

std::string HTTPrequest::method() const
{
    return _method;
}

std::string HTTPrequest::version() const
{
    return _version;
}


std::string HTTPrequest::getPost(const std::string& key) const
{
    assert(key != "");
    if (post.count(key) == 1) {
        return post.find(key)->second;
    }
    return "";
}

std::string HTTPrequest::getPost(const char* key) const
{
    assert(key != nullptr);
    if (post.count(key) == 1) {
        return post.find(key)->second;
    }
    return "";
}

bool HTTPrequest::isKeepAlive() const
{
    if (header.count("Connection") == 1) {
        return header.find("Connection")->second == "keep-alive" && _version == "1.1";
    }
    return false;
}

bool HTTPrequest::parseRequestLine(const std::string& line)
{
    std::regex re("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch Match;
    if (std::regex_match(line, Match, re)) {
        _method = Match[1];
        _path = Match[2];
        _version = Match[3];
        state = HEADERS;
        return true;
    }
   return false;
}

void HTTPrequest::parseRequestHeader(const std::string& line)
{
    std::regex re("^([^:]*): ?(.*)$");
    std::smatch Match;
    if (std::regex_match(line, Match, re)) {
        header[Match[1]] = Match[2];
    }
    else state = BODY;
}

void HTTPrequest::parseRequestBody(const std::string& line)
{
    _body = line;
    if (_method == "POST" && header["Content-Type"] == "application/x-www-form-urlencoded")
        parsePost();
    state = FINISH;
}
/*

d = a+b+c;


*/
void HTTPrequest::parsePath()
{
    if (_path == "/") _path = "/index.html";
    else {
        for (auto& item : DEAFAULT_HTML) {
            if (_path == item) {
                _path += ".html";
                return;
            }
        }
    }
}

void HTTPrequest::parsePost()
{
    //用+代替空格，=分割k,v，%分割数值，&分割语句
    if (_body.size() == 0) return ;
    std::string key, value;
    int num = 0;
    int n = _body.size();
    int i = 0, j = 0;
    for (char ch = _body[i]; i < n; i++) {
        switch (ch)
        {
        case '=':
            key = _body.substr(j,i-j);
            j = i + 1;
            break;
        case '+':
            _body[i] = ' ';
            break;
        case '&':
            value = _body.substr(j,i-j);
            j = i + 1;
            post[key] = value;
            break;
        case '%':
            num = convertHex(_body[i + 1]) * 16 + convertHex(_body[i + 2]);
            _body[i + 2] = num % 10 + '0'; n /= 10;
            _body[i + 1] = num % 10 + '0'; n /= 10;
            _body[i ] = num % 10 + '0'; n /= 10;
            break;
        default:
            break;
        }
    }
}

int HTTPrequest::convertHex(char ch)
{
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    return ch;
}
