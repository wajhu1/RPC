#include "test.pb.h"
#include <iostream>
#include <string>

#if 0
int main(){
    //封装数据
    fixbug::LoginRequest req;
    req.set_name("zhang san");
    req.set_pwd("123456");
    //序列化 =》 char* 将序列化结果存放在send_str中
    std::string send_str;
    if(req.SerializeToString(&send_str)){
        std::cout << send_str <<std::endl;
    }

    //反序列化
    fixbug::LoginRequest reqB;
    if(reqB.ParseFromString(send_str)){
        std::cout << reqB.name() <<std::endl;
        std::cout << reqB.pwd() << std::endl;
    }

    return 0;
}   
#endif


#if 1

int main(){
    fixbug::LoginResponse rep;
    fixbug::ResultCode *rc = rep.mutable_result();
    rc->set_errcode(1);
    rc->set_errmsg("Login failed");
    
}



#endif