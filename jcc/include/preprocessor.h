#ifndef __PREPROCESSOR_H
#define __PREPROCESSOR_H
#include "../include/lexer.h"
#include "../include/error.h"
Error preprocess_tokens(LexTokenStream *input);
char *preprocess_includes(Reader *input);
#endif
