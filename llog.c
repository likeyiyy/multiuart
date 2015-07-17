/*************************************************************************
	> File Name: llog.c
	> Author: likeyi
	> Mail: likeyi@sina.com 
	> Created Time: Wed 15 Jul 2015 07:18:40 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>
#include <string.h>
#include "llog.h"
static pthread_mutex_t global_mutex;
static LogLevel  global_LogLevel;
static FILE * global_fp = NULL;

int currenttime(char * currTime)
{
	struct tm *ptm = NULL;
	time_t tme;
	tme = time(NULL);
	ptm = localtime(&tme);
	char szTime[256];
	memset(szTime, 0, 256);
	sprintf(szTime, "[%d-%02d-%02d %02d:%02d:%02d] ", (ptm->tm_year + 1900),
		ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	strcpy(currTime, szTime);
	return 0;
}

int llog_init(LogLevel level, FILE * fp)
{
    if(fp == NULL)
    {
        printf("[LLOG]: init llog failed: fp is NULL\n");
        exit(-1);
    }
    pthread_mutex_init(&global_mutex,NULL);
    global_LogLevel = level;
    global_fp = fp;

    return 0;
}

char loglevel[6][10] = {"",
                        "[ERROR] ",
                        "[WARNING] ",
                        "[NOTICE] ",
                        "[TRACE] ",
                        "[DEBUG] "};
static char * getLogLevel(LogLevel level)
{
    if(level > LL_DEBUG || level < LL_ERROR)
    {
        return "";
    }
    else
    {
        return loglevel[level];
    }

}
int llog(LogLevel level, char * format, ...)
{
    if(level > global_LogLevel)
    {
        return 0;
    }
    char temp[1024];
    memset(temp, 0, 1024);
    char currTime[256];
    memset(currTime, 0, 256);
    currenttime(currTime);
            
    va_list args;
    va_start( args, format  );
    vsprintf(temp, format, args );
    va_end( args  );

    pthread_mutex_lock(&global_mutex);
    if (global_fp)
    {
        fputs(currTime, global_fp);
        fputs(getLogLevel(level), global_fp);
        fputs(temp, global_fp);
        fputs("\n", global_fp);
    }
    fflush(global_fp);
    pthread_mutex_unlock(&global_mutex);
    return 0;
}

