#ifndef __LINKER_H
#define __LINKER_H
#include <stdint.h>
#include <stdlib.h>

size_t linker_link(uint8_t **input, size_t n, size_t *inp_lens, uint8_t **data);

#endif
