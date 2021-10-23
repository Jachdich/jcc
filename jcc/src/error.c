#include "../include/error.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void error_free(Error *e) {
    if (e->message != NULL) {
        free(e->message);
    }
}

Error error_construct(int status_code, const char *static_msg) {
    Error err;
    err.code = status_code;
    err.message = malloc(strlen(static_msg) + 1);
    strcpy(err.message, static_msg);
    return err;
}
