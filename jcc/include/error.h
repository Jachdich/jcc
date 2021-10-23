#ifndef __ERROR_H
#define __ERROR_H

struct Error {
    int code;
    char *message;
};

typedef struct Error Error;
void error_free(Error *e);
Error error_construct(int status_code, const char *static_msg);
#endif
