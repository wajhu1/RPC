#include "mprpcchannel.h"
#include <string>
#include "rpcheader.pb.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include "mprpcapplication.h"

void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method
                    ,google::protobuf::RpcController* controller
                    ,const google::protobuf::Message* request
                    ,google::protobuf::Message* response
                    ,google::protobuf::Closure* done){
    const google::protobuf::ServiceDescriptor* sd = method->service();
    std::string service_name = sd->name();
    std::string method_name = method->name();

    uint32_t args_size = 0;
    std::string args_str;
    if(request->SerializeToString(&args_str)){
        args_size = args_str.size();
    }else{
        std::cout << "serialize request error!" <<std::endl;
        return ;
    }

    mprpc::RpcHHeader rpcHeader;

    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);


    uint32_t header_size = 0;
    std::string rpc_header_str;
    if(rpcHeader.SerializeToString(&rpc_header_str)){
        header_size = rpc_header_str.size();
    }else{
        std::cout<< "serialize request error!"<<std::endl;
        return;
    }

    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char*)&header_size, 4));
    send_rpc_str += rpc_header_str;
    send_rpc_str += args_str;

    int clientfd = socket(AF_INET, SOCK_STREAM,0);

    if(-1 == clientfd){
        std::cout << "create socket error! errno:"<<errno <<std::endl;
        exit(EXIT_FAILURE);
    }

    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    
    if(-1 == connect(clientfd, (struct sockaddr*)&server_addr, sizeof(server_addr))){
        std::cout << "create socket error! errno:"<< errno <<std::endl;
        exit(EXIT_FAILURE);
    }

    if(-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0)){
        std::cout << "send error! errno"<<errno <<std::endl;
        close(clientfd);
        return;
    }

    int recv_size = 0;
    char buf[1024] = {0};
    if(-1 == (recv_size = recv(clientfd, buf, 1024, 0))){
        std::cout << "recv error! errno: "<<errno <<std::endl;
        close(clientfd);
        return;
    }

    // std::string response_str(buf, 0, recv_size);
    // if(!response->ParseFromString(response_str)){
    if(!response->ParseFromArray(buf, recv_size)){
        std::cout << "parse error! response_str"<<buf<<std::endl;
        close(clientfd);
        return;
    }

    close(clientfd);



}
