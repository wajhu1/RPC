#include "rpcprovider.h"
#include "mprpcapplication.h"
#include <string>
#include <functional>
#include <google/protobuf/descriptor.h>
#include "rpcheader.pb.h"

void RpcProvider::NotifyService(google::protobuf::Service *service){

    ServiceInfo service_info;


    //获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    //获取服务名字
    std::string service_name = pserviceDesc->name();
    //获取服务对象service的方法数量
    int methodCnt = pserviceDesc->method_count();

    for(int i = 0; i < methodCnt; ++i){
        const google::protobuf::MethodDescriptor* pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name, pmethodDesc});
    }
    service_info.m_service = service;
    m_serviceMap.insert({service_name, service_info});
}

void RpcProvider::Run(){
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port);

    //创建TcpServer对象
    muduo::net::TcpServer server(&m_eventLoop,address,"RpcProvider");
    //绑定连接回调和消息读写回调函数
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection,this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage,this
        , std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));


    //设置muduo库的线程数量
    server.setThreadNum(4);

    std::cout<<"start"<<std::endl;

    //启动网络服务
    server.start();
    m_eventLoop.loop();
}


void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn){
    if(!conn->connected()){
        conn->shutdown();
    }
}

/*
在框架内部，RpcProvider和RpcConsumer协商好之间通信用的protobuf数据类型
service_name method_name args 定义proto的message类型，进行数据头的序列化和反序列化
                              service_name method_name args_size
header_size(4字节) + header_str + args_str
前面4字节按照二进制存

*/

void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr& conn
    ,muduo::net::Buffer* buffer,muduo::Timestamp){
    //网络上接收的远程rpc调用请求的字符流 login args
    std::string recv_buf = buffer->retrieveAllAsString();

    //从字符流中读取前4字节的内容
    uint32_t header_size = 0;
    recv_buf.copy((char*)&header_size, 4, 0);
    
    //根据header_size读取数据头的原始字符流，反序列化数据，得到rpc请求的详细信息
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if(rpcHeader.ParseFromString(rpc_header_str)){
        //数据头序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }else{
        //数据头序列化失败
        std::cout << "rpc_header_str"<<rpc_header_str <<"parse error!" <<std::endl;
    }

    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    auto it = m_serviceMap.find(service_name);
    if(it == m_serviceMap.end()){
        std::cout << service_name<<"is not exist1"<<std::endl;
        return;
    }

    google::protobuf::Service *service = it->second.m_service;

    auto mit = it->second.m_methodMap.find(method_name);
    if(mit == it->second.m_methodMap.end()){
        std::cout << service_name << ": "<<method_name << "is not exist"<<std::endl;
        return;
    }

    const google::protobuf::MethodDescriptor *method = mit->second;

    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if(!request->ParseFromString(args_str)){
        std::cout<<"request parse error! connect:"<<args_str<<std::endl; 
    }

    google::protobuf::Message *response = service->GetResponsePrototype(method).New();


    google::protobuf::Closure *done = 
        google::protobuf::NewCallback<RpcProvider
            , const muduo::net::TcpConnectionPtr&, google::protobuf::Message*>(this, &RpcProvider::SendRpcResponse,conn, response);



    service->CallMethod(method, nullptr, request, response, done);



}


void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message* response){
    std::string response_str;
    if(response->SerializeToString(&response_str)){
        conn->send(response_str);
    }else{
        std::cout << "Serialize response_str error!"<<std::endl;
    }
    conn->shutdown();
}