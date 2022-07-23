#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <unordered_map>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>//文件管理
#include <sys/mman.h>//内存管理
#include <assert.h>

#include "buffer.h"
class HTTPresponse
{
public:

	HTTPresponse();
	~HTTPresponse();

	void init(const std::string& srcDir, std::string& path, bool isKeepAlive, int code);
	void makeResponse(Buffer& buffer);
	void unmapFile();
	char* file();
	size_t fileLen() const;
	void errorContent(Buffer& buffer, std::string message);
	int Code()const;

private:
	void addResponseLine(Buffer& buffer);
	void addResponseHeader(Buffer& buffer);
	void addResponseContent(Buffer& buffer);

	void errorHTML();
	std::string getFileType();//文件类型信息

	int code;
	bool isKeepAlive;
	std::string path;
	std::string srcDir;

	char* File;
	struct stat FileStat;
	static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
	static const std::unordered_map<int, std::string> CODE_STATUS;
	static const std::unordered_map<int, std::string> CODE_PATH;
};
#endif
