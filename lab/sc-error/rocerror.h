/* rocerror.h */
#ifndef ROCERROR_H
#define ROCERROR_H

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>

void unix_error(char *msg);
void posix_error(int code, char *msg);
void dns_error(char *msg);
void gai_error(int code, char *msg);
void app_error(char *msg);

#endif
