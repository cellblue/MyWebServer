#include "HTTPresponse.h"
const std::unordered_map<std::string, std::string> HTTPresponse::SUFFIX_TYPE = {
    { ".html",  "text/html" },
    { ".xml",   "text/xml" },
    { ".xhtml", "application/xhtml+xml" },
    { ".txt",   "text/plain" },
    { ".rtf",   "application/rtf" },
    { ".pdf",   "application/pdf" },
    { ".word",  "application/nsword" },
    { ".png",   "image/png" },
    { ".gif",   "image/gif" },
    { ".jpg",   "image/jpeg" },
    { ".jpeg",  "image/jpeg" },
    { ".au",    "audio/basic" },
    { ".mpeg",  "video/mpeg" },
    { ".mpg",   "video/mpeg" },
    { ".avi",   "video/x-msvideo" },
    { ".gz",    "application/x-gzip" },
    { ".tar",   "application/x-tar" },
    { ".css",   "text/css "},
    { ".js",    "text/javascript "},
};

const std::unordered_map<int, std::string> HTTPresponse::CODE_STATUS = {
    { 200, "OK" },
    { 400, "Bad Request" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
};

const std::unordered_map<int, std::string> HTTPresponse::CODE_PATH = {
    { 400, "/400.html" },
    { 403, "/403.html" },
    { 404, "/404.html" },
};

HTTPresponse::HTTPresponse() 
{
    code = -1;
    path = srcDir = "";
    isKeepAlive = false;
    File = nullptr; 
    FileStat = { 0 };
}

HTTPresponse::~HTTPresponse()
{
    if (File) {
        munmap(File, FileStat.st_size);
        File = nullptr;
    }
}

void HTTPresponse::init(const std::string& srcDir, std::string& path, bool isKeepAlive, int code) {
    assert(srcDir != "");
    if (File) { unmapFile(); }
    this->code = code;
    this->isKeepAlive = isKeepAlive;
    this->path = path;
    this->srcDir = srcDir;
    this->File = nullptr;
    this->FileStat = { 0 };
}

void HTTPresponse::makeResponse(Buffer& buffer)
{
    //判断请求的资源
    if (stat((srcDir + path).data(), &FileStat) < 0
        || S_ISDIR(FileStat.st_mode)) code = 404;
    else if (!(FileStat.st_mode & S_IROTH)) code = 403;
    else if (code == -1) code = 200;
    errorHTML();
    addResponseLine(buffer);
    addResponseHeader(buffer);
    addResponseContent(buffer);
}

void HTTPresponse::unmapFile()
{
    if (File) {
        munmap(File, FileStat.st_size);
        File = nullptr;
    }
}

char* HTTPresponse::file()
{
    return File;
}

size_t HTTPresponse::fileLen() const
{
    return FileStat.st_size;
}

void HTTPresponse::errorContent(Buffer& buffer, std::string message)
{
    std::string body;
    std::string status;
    body += "<html><title>Error<title>";
    body += "<body bgcolor=\"ffffff\">";
    if (CODE_STATUS.count(code)) status = CODE_STATUS.find(code)->second;
    else status = "Bad Request";
    body += std::to_string(code) + " : " + status + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>TinyWebServer</em></body></html>";

    buffer.append("Content-length: "+std::to_string(body.size())+"\r\n\r\n");
}

int HTTPresponse::Code() const
{
    return code;
}

void HTTPresponse::addResponseLine(Buffer& buffer)
{
    std::string status;
    if (CODE_STATUS.count(code)) {
        status = CODE_STATUS.find(code)->second;
    }
    else {
        code = 400;
        status = CODE_STATUS.find(code)->second;
    }
    buffer.append("HTTP/1.1"+std::to_string(code)+" "+status+"\r\n");
}

void HTTPresponse::addResponseHeader(Buffer& buffer)
{
    buffer.append("Connection: ");
    if (isKeepAlive) {
        buffer.append("keep-alive\r\n");
        buffer.append("keep-alive: max=6, timeout=120\r\n");
    }
    else {
        buffer.append("close\r\n");
    }
    buffer.append("Content-type: "+getFileType()+"\r\n");
}

void HTTPresponse::addResponseContent(Buffer& buffer)
{
    int srcFd = open((srcDir+path).data(),O_RDONLY);
    if (srcFd < 0) {
        errorContent(buffer,"File NotFound!");
        return;
    }
    //MAP_PRIVATE建立一个写入拷贝时的私有映射
    int* Res = (int*)mmap(0,FileStat.st_size,PROT_READ,MAP_PRIVATE,srcFd,0);
    if (*Res == -1) {
        errorContent(buffer, "File NotFound!");
        return;
    }
    File = (char*)Res;
    close(srcFd);
    buffer.append("Content-length: " + std::to_string(FileStat.st_size) + "\r\n\r\n");
}

void HTTPresponse::errorHTML()
{
    if (CODE_PATH.count(code)) {
        path = CODE_PATH.find(code)->second;
        stat((srcDir+path).data(),&FileStat);
    }
}

std::string HTTPresponse::getFileType()
{
    std::string::size_type id = path.find_last_of('.');
    if (id == std::string::npos) {
        return "text/plain";
    }
    std::string suffix = path.substr(id);
    if (SUFFIX_TYPE.count(suffix)) {
        return SUFFIX_TYPE.find(suffix)->second;
    }
    return "text/plain";
}


