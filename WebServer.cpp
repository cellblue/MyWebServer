#include "WebServer.h"

WebServer::WebServer(int port, int trigMode, int timeoutMs, bool optLinger, 
	int sqlPort, const char* sqlUser, const  char* sqlPwd,const char* dbName,int sqlSize,	
	int threadNum):
    port(port), openLinger(optLinger), timeoutMS(timeoutMS), isClose(false),
    timer(new TimerManager()), threadpool(new ThreadPool(threadNum)), epoller(new Epoller())
{
    srcDir = getcwd(nullptr,256);
    assert(srcDir);
    strncat(srcDir,"/resources/",16);
    HTTPconnection::userCount = 0;
    HTTPconnection::srcDir = srcDir;
    SqlConnect::Instance()->Init("localhost",sqlPort,sqlUser,sqlPwd,dbName,sqlSize);

    initEventMode(trigMode);
    if (!initSocket()) isClose = true;
}

WebServer::~WebServer()
{
    //std::cout<<"xi gou"<<std::endl;
    close(listenFd);
    isClose = true;
    free(srcDir);
    SqlConnect::Instance()->Close();
}

void WebServer::Start()
{
    int timeMS = -1;//epoll wait timeout==-1就是无事件一直阻塞
    if (!isClose)
    {
        std::cout << "============================";
        std::cout << "Server Start!";
        std::cout << "============================";
        std::cout << std::endl;
    }

    while(!isClose){
        if (timeoutMS > 0) {
            timeMS = timer->nextHandle();
        }

        int eventCnt = epoller->wait(timeMS);
        for (int i = 0; i < eventCnt; ++i) {
            int fd = epoller->getEventFd(i);
            uint32_t events = epoller->getEvents(i);
            //std::cout<<"this fd :"<<fd<<" listenfd :"<<listenFd<<" events: "<<events<<std::endl;
            if (fd == listenFd) {
                handleListen();
            }
            else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                assert(users.count(fd) > 0);
                closeHTTP(&users[fd]);
            }
            else if (events & EPOLLIN) {
                assert(users.count(fd) > 0);
                handleRead(&users[fd]);
            }
            else if (events & EPOLLOUT) {
                assert(users.count(fd) > 0);
                handleWrite(&users[fd]);
            }
            else {
                std::cout << "Unexpected event" << std::endl;
            }
        }
    }
}

bool WebServer::initSocket()
{
    int ret;
    struct sockaddr_in addr;
    if (port > 65535 || port < 1024) {
	std::cout<<"Port number error!"<<std::endl;
	return false;
     }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    struct linger optLinger = {0};
    if (openLinger) {
        optLinger.l_onoff = 1;
        optLinger.l_linger = 1;
    }
    
    listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd < 0)  {
    	std::cout<<"Create socket error!"<<std::endl;
    	return false;
    }
    ret = setsockopt(listenFd,SOL_SOCKET,SO_LINGER,&optLinger,sizeof optLinger);
    if (ret == -1) {
    	  std::cout<<"Init linger error!"<<std::endl;
        close(listenFd);
        return false;
    }
    
    int optval = 1;
    ret = setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof (int));

    if (ret == -1) {
   	 std::cout<<"set socket setsockopt error !"<<std::endl;
        close(listenFd);
        return false;
    }
    ret = bind(listenFd, (struct sockaddr*)&addr, sizeof(addr));

    if (ret < 0) {
	std::cout<<"Bind Port"<<port<<" error!"<<std::endl;
        close(listenFd);
        return false;
    }
    ret = listen(listenFd, 6);

    if (ret < 0) {
   	 printf("Listen port:%d error!\n", port);
        close(listenFd);
        return false;
    }
    ret = epoller->addFd(listenFd,listenEvent|EPOLLIN);

    if (ret == 0) {
   	 printf("Add listen error!\n");
        close(listenFd);
        return false;
    }
    setFdNonblock(listenFd);
    printf("Server port:%d\n", port);
    return true;
}

void WebServer::initEventMode(int trigMode)//切换模式
{
    listenEvent = EPOLLRDHUP;
    connectionEvent = EPOLLONESHOT | EPOLLRDHUP;
    switch (trigMode) {
    case 0:
        break;
    case 1:
        connectionEvent |= EPOLLET;
        break;
    case 2:
        listenEvent |= EPOLLET;
        break;
    case 3:
        listenEvent |= EPOLLET;
        connectionEvent |= EPOLLET;
        break;
    default:
        listenEvent |= EPOLLET;
        connectionEvent |= EPOLLET;
        break;
    }
    HTTPconnection::isET = (connectionEvent & EPOLLET );
}

void WebServer::addHTTP(int fd, sockaddr_in addr)
{
    assert(fd > 0);
    users[fd].initHTTPconnection(fd,addr);
    if (timeoutMS > 0) {
        timer->addTimer(fd,timeoutMS,std::bind(&WebServer::closeHTTP,this,&users[fd]));
    }
    epoller->addFd(fd,EPOLLIN|connectionEvent);
    setFdNonblock(fd);
}

void WebServer::closeHTTP(HTTPconnection* client)
{
    assert(client);
    std::cout<<"client"<<client->getFd()<<" quit!"<<std::endl;
    epoller->delFd(client->getFd());
    client->closeHTTPConnection();
}

void WebServer::handleListen()
{
    struct sockaddr_in addr;
    socklen_t len = sizeof addr;
    do{
	 int fd = accept(listenFd,(struct sockaddr *)&addr,&len);
    if (fd <= 0) return;
    else if (HTTPconnection::userCount >= 65536) {
        sendError(fd,"Server busy!");
        return;
     }
    else addHTTP(fd,addr);
    }while(listenEvent & EPOLLET);
}

void WebServer::handleRead(HTTPconnection* client)
{
    assert(client);
    extentTime(client);
    threadpool->submit(std::bind(&WebServer::onRead,this,client));
}

void WebServer::handleWrite(HTTPconnection* client)
{
    assert(client);
    extentTime(client);
    threadpool->submit(std::bind(&WebServer::onWrite, this, client));
}

void WebServer::onRead(HTTPconnection* client)
{
    assert(client);
    int readErrno = 0;
    int ret = client->readBuffer(&readErrno);
    //std::cout<<"readErrno:"<<readErrno<<std::endl;
    if (ret <= 0 && readErrno != EAGAIN) {
	  std::cout<<"Do not read data!"<<std::endl;
        closeHTTP(client);
        return;
    }
    onProcess(client);
}

void WebServer::onWrite(HTTPconnection* client)
{
    assert(client);
    int writeErrno = 0;
    int ret = client->writeBuffer(&writeErrno);
    if (client->writeBytes() == 0) { // 传输完成 
        if (client->isKeepAlive()) {
            onProcess(client);
            return;
        }
    }
    else if (ret < 0) {
        if (writeErrno == EAGAIN) {
            epoller->modFd(client->getFd(),connectionEvent | EPOLLOUT);
            return;
        }
    }
    closeHTTP(client);
}

void WebServer::onProcess(HTTPconnection* client)
{
    if (client->handleHTTPConnection()) {
        epoller->modFd(client->getFd(), connectionEvent | EPOLLOUT);
    }
    else {
        epoller->modFd(client->getFd(), connectionEvent | EPOLLIN);
    }
}

void WebServer::sendError(int fd, const char* info)
{
    assert(fd > 0);
    int ret = send(fd,info,strlen(info),0);

    if (ret < 0) {
        std::cout << "send error to client" << fd << " error!" << std::endl;
    }
    close(fd);
}

void WebServer::extentTime(HTTPconnection* client)//修改时间
{
    assert(client);
    if (timeoutMS > 0) {
        timer->update(client->getFd(),timeoutMS);
    }
}
int WebServer::setFdNonblock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}
