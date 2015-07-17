/*************************************************************************
	> File Name: config.h
	> Author: li.ky
	> Mail: likeyiyy@sina.com
	> Created Time: Fri 17 Jul 2015 01:14:17 PM CST
 ************************************************************************/

#ifndef _CONFIG_H
#define _CONFIG_H

typedef struct
{
    char name[16];
    int rbaud;
    char protocol[16];
}config_dev_t;

typedef struct
{
    config_dev_t * devs;
    int dev_nums;
}config_t;


int read_config_file(char * filename, config_t * config);
#endif
