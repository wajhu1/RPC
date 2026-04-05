#include "mprpcapplication.h"
#include <iostream>
#include <unistd.h>
#include <string>

MprpcConfig MprpcApplication::m_config;

void ShowArgsHelp(){
    std::cout<<"format: command -i <configfile>"<<std::endl;
}

void MprpcApplication::Init(int argc, char ** argv){
    if(argc < 2){
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }

    int c = 0;
    std::string config_file;
    //getopt函数：第三个参数如：ab:c，当看到-a/-b/-c就进行处理，其中当看到-b就将后面的参数放到optarg里面后面没有就返回':'，当-后面都不是就返回'?'
    while((c = getopt(argc,argv, "i:")) != -1){
        switch(c){
            case 'i':
                //将配置文件名存在optarg
                config_file = optarg;
                break;
            //找不到i
            case '?':
                std::cout << "invalid args!"<< std::endl;
                ShowArgsHelp();
                exit(EXIT_FAILURE);
            //-i后面没有参数
            case ':':
                std::cout << "need <configfile>" <<std::endl;
                ShowArgsHelp();
                exit(EXIT_FAILURE);
            default:
                break;
        }
    }
    //开始加载配置文件 
    m_config.LoadConfigFile(config_file.c_str());

    std::cout << "rpcserverip:"<<m_config.Load("rpcserverip") << std::endl;
    std::cout << "rpcserverport:"<<m_config.Load("rpcserverport") << std::endl;
    std::cout << "zookeeperip:"<<m_config.Load("zookeeperip") << std::endl;
    std::cout << "zookeeperport:"<<m_config.Load("zookeeperport") << std::endl;
                

}

MprpcApplication& MprpcApplication::GetInstance(){
    static MprpcApplication app;
    return app;
}


MprpcConfig& MprpcApplication::GetConfig(){
    return m_config;
}