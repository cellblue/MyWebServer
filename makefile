CXX=g++
CFLAGS=-std=c++11 -O2 -Wall -g
CXXFLAGS=-std=c++11 -O2 -Wall -g

TARGET:=myserver
SOURCE:=$(wildcard ../*.cpp)
OBJS=./buffer.cpp ./HTTPrequest.cpp ./HTTPresponse.cpp ./HTTPconnection.cpp \
     ./timer.cpp ./epoller.cpp ./WebServer.cpp ./main.cpp

$(TARGET):$(OBJS)
	$(CXX) $(CXXFLAGS)  $(OBJS) -o ./bin/$(TARGET) -pthread

