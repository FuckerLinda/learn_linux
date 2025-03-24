/* rocerror.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include "rocerror.h"

void unix_error(char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(EXIT_FAILURE);
}

void posix_error(int code, char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(code));
    exit(EXIT_FAILURE);
}

void gai_error(int code, char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, gai_strerror(code));
    exit(EXIT_FAILURE);
}

void app_error(char *msg)
{
    fprintf(stderr, "Application Error: %s\n", msg);
    exit(EXIT_FAILURE);
}

void dns_error(char *msg)
{
    fprintf(stderr, "DNS Error: %s\n", msg);
    exit(EXIT_FAILURE);
}
