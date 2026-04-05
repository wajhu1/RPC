#pragma once
#include "google/protobuf/service.h"
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <memory>
#include <unordered_map>

class RpcProvider{
public:
    void NotifyService(google::protobuf::Service *service);

    void Run();

private:

    //组合EventLoop
    muduo::net::EventLoop m_eventLoop;


    struct ServiceInfo{
        google::protobuf::Service *m_service;
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> m_methodMap;
    };

    std::unordered_map<std::string , ServiceInfo> m_serviceMap;

    //新的socket连接回调
    void OnConnection(const muduo::net::TcpConnectionPtr&);
    //读写事件回调
    void OnMessage(const muduo::net::TcpConnectionPtr&,
                            muduo::net::Buffer*,
                            muduo::Timestamp);


    void SendRpcResponse(const muduo::net::TcpConnectionPtr&, google::protobuf::Message*);
};