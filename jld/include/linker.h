#ifndef __LINKER_H
#define __LINKER_H
#include <stdint.h>
#include <stdlib.h>

enum LinkError {
    NONLINKABLE_OBJECT = -1,
    UNRESOLVED_SYMBOL = -2,
    INVALID_SYM_ID = -3,
    START_NOT_FOUND = -4,
};

int linker_link(uint8_t **input, size_t n, size_t *inp_lens, uint8_t **data);

#endif
