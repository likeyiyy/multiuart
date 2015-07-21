/*************************************************************************
	> File Name: config.c
	> Author: li.ky
	> Mail: likeyiyy@sina.com
	> Created Time: Fri 17 Jul 2015 01:14:13 PM CST
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include "llog.h"
#include "config.h"

static inline int get_file_length(FILE * fp)
{
    fseek(fp, 0, SEEK_END);
    int length = ftell(fp);
    rewind(fp);
    return length;
}

static inline char * get_variable_value(char * start)
{
    char * temp = start;
    while(*temp++ != '=')
        continue;
    temp++;
    while(*temp++ == ' ')
        continue;
    char * var_start = temp - 1;

    while(!((*temp == ' ')||(*temp == '\n')||(*temp == '#')))
    {
        temp++;
        continue;
    }
    char * var_end = temp;
    int var_len = var_end - var_start;
    char * need_free = malloc(var_len + 1);
    strncpy(need_free,var_start,var_len);
    need_free[var_len] = '\0';
    return need_free;
}
static inline void get_dev_config_from_buffer(config_t * config, char * buffer, int file_length)
{
    char * cur = NULL ;
    int i = 0;
    cur = buffer;
    while(cur)
    {
        if((cur = strstr(cur, "<dev>")) != NULL)
        {
            char * namestart           = strstr(cur,"name");
            char * rbaudstart          = strstr(namestart,"rbaud");
            char * protocolstart       = strstr(rbaudstart,"protocol");
            char * name_need_free      = get_variable_value(namestart);
            char * rbaud_need_free     = get_variable_value(rbaudstart);
            char * protocol_need_free  = get_variable_value(protocolstart);
            strcpy(config->devs[i].name,name_need_free);
            config->devs[i].rbaud = atoi(rbaud_need_free);
            strcpy(config->devs[i].protocol, protocol_need_free);
            free(name_need_free);
            free(rbaud_need_free);
            free(protocol_need_free);
            LOG_NOTICE("%s,%d,%s",config->devs[i].name,
                                  config->devs[i].rbaud,
                                  config->devs[i].protocol);
            cur = protocolstart;
            i++;
        }
    }
}
static inline int get_dev_nums(char * buffer, int file_length)
{
    int start_times = 0;
    int end_times = 0;

    char * cur = NULL ;
    cur = buffer;
    while(cur)
    {
        if((cur = strstr(cur, "<dev>")) != NULL)
        {
            start_times++;
            cur += strlen("<dev>");
        }
    }
    cur = buffer;
    while(cur)
    {
        if((cur = strstr(cur, "</dev>")) != NULL)
        {
            end_times++;
            cur += strlen("</dev>");
        }
    }
    return start_times == end_times ? start_times : -1;
}

int read_config_file(char * filename, config_t * config)
{
    VERIFY(config,"config is NULL");
    VERIFY(filename,"filename is NULL");
    FILE * fp = NULL;
    fp = fopen(filename,"r");
    VERIFY(fp,"%s can not open.",filename);
    LOG_NOTICE("open %s config file SUCCEED",filename);

    /* 1. get file length */
    int file_length = get_file_length(fp);
    char * temp   = malloc(file_length + 1);
    char * buffer = malloc(file_length + 1);
    VERIFY(buffer,"malloc buffer failed");
    /* 2. read to buffer */
    int result = fread(buffer, 1, file_length, fp);
    VERIFY(result == file_length, "read config file error");
    buffer[file_length + 1] = '\0';
    int counter = 0;
    for(int i = 0; i < file_length; i++)
    {
        if(buffer[i] != '#')
        {
            temp[counter++] = buffer[i];
        }
        else
        {
            while(buffer[i] != '\n')
            {
                i++;
            }
            temp[counter++] = buffer[i];
        }
    }
    buffer = temp;
    file_length = strlen(temp);

    /* 3. get_dev_nums  */
    int dev_nums = get_dev_nums(buffer, file_length);
    VERIFY(dev_nums > 0, "config file not right");
    config->dev_nums = dev_nums;
    config->devs = malloc(sizeof(config_dev_t) * dev_nums);
    VERIFY(config->devs,"config devs alloc failed");

    /* 4. get every dev config */
    get_dev_config_from_buffer(config, buffer, file_length);
    return 0;
}


//test
#ifdef DEBUG_CONFIG
int main(int argc, char ** argv)
{
    config_t config;
    llog_init(LL_DEBUG,stdout);
    
    read_config_file("./uart_daemon.conf",&config);
}
#endif
