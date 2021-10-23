#include "../include/codegen.h"
#include <stdlib.h>
#include <string.h>

Error cg_gen(AST *ast, char **code, int debug) {
    *code = malloc(30);
    strcpy(*code, "some src");
    return (Error){0, NULL};
}