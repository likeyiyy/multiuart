/*************************************************************************
	> File Name: message.h
	> Author: li.ky
	> Mail: likeyiyy@sina.com
	> Created Time: Fri 17 Jul 2015 10:17:08 AM CST
 ************************************************************************/

#ifndef _MESSAGE_H
#define _MESSAGE_H

#define HEADER_LEN 3
#include <stdint.h>
typedef struct 
{
    uint8_t  index;
    uint16_t length;
    uint8_t * data;
    struct timeval stamp;
}message_t;

#endif
