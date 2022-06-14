#!/usr/bin/bash

TARGET = sar_server sar_client

all:$(TARGET)

sar_server: server.cc sar_application.pb.cc
	g++ -o $@ $^ -std=c++11 -L./ -lsrpc -lpthread -lserviceMgr -ljsoncpp -lcurl

sar_client: client.cc sar_application.pb.cc
	g++ -o $@ $^ -std=c++11 -L./ -lsrpc -lpthread -lserviceMgr -ljsoncpp -lcurl

.phony: clean
clean:
	rm $(TARGET)