#pragma once
#include "lockqueue.h"
#include <string>

enum LogLevel{
    INFO,
    ERROR,
};

class Logger{
public:

    void SetLogLevel(LogLevel level);

    void log(std::string msg);

    static Logger& GetInstance();

private:

    Logger();
    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;


    int m_loglevel;
    LockQueue<std::string> m_lckQue;
};


#define LOG_INFO(logmsgformat, ...) \
    do{\
        Logger &logger = Logger::GetInstance();\
        logger.SetLogLevel(INFO);\
        char c[1024] = {0};\
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.log(c); \
    }while(0);


#define LOG_ERR(logmsgformat, ...) \
    do{\
        Logger &logger = Logger::GetInstance();\
        logger.SetLogLevel(ERROR);\
        char c[1024] = {0};\
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.log(c); \
    }while(0);