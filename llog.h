/*************************************************************************
	> File Name: llog.h
	> Author: likeyi
	> Mail: likeyi@sina.com 
	> Created Time: Wed 15 Jul 2015 07:14:15 PM CST
 ************************************************************************/

#ifndef H_LLOG_H
#define H_LLOG_H

#include <stdlib.h>

typedef  enum LogLevel {  
    LL_DEBUG =   5,
    LL_TRACE =   4,
    LL_NOTICE =  3, 
    LL_WARNING = 2, 
    LL_ERROR =   1,
}LogLevel;

int llog_init(LogLevel level, FILE * fp);
int llog(LogLevel level, char * logformat, ...);

#define LOG_ERROR(log_fmt, log_arg...)                                                     \
        do{                                                                                \
                llog(LL_ERROR,   "[%s:%d][%s] " log_fmt,                                   \
                                             __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
            } while (0) 

#define LOG_WARN(log_fmt, log_arg...)                                                      \
        do{                                                                                \
                llog(LL_WARNING,   "[%s:%d][%s] " log_fmt ,                                \
                                             __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
            } while (0) 

#define LOG_NOTICE(log_fmt, log_arg...)                                                    \
        do{                                                                                \
                llog(LL_NOTICE,   "[%s:%d][%s] " log_fmt ,                                 \
                                             __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
            } while (0) 

#define LOG_TRACE(log_fmt, log_arg...)                                                     \
        do{                                                                                \
                llog(LL_TRACE,   "[%s:%d][%s] " log_fmt ,                                  \
                                             __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
            } while (0) 

#define LOG_DEBUG(log_fmt, log_arg...)                                                     \
        do{                                                                                \
                llog(LL_DEBUG,   "[%s:%d][%s] " log_fmt ,                                  \
                                             __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
            } while (0) 

#define VERIFY(VAL, log_fmt, log_arg...)                                                    \
        do{                                                                                \
            if(!(VAL))                                                                     \
            {                                                                              \
                llog(LL_ERROR,   "[%s:%d][%s] " log_fmt,                                   \
                                             __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
                exit(-1);                                                                  \
            }                                                                              \
        } while(0)                          

#endif
