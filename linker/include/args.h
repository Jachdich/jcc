#ifndef __ARGS_H
#define __ARGS_H

struct Args {
    int status;
    char **ifnames;
    char *ofname;
    int n_ifs;
};

typedef struct Args Args;

void usage(char *ename);

Args parse_args(int argc, char **argv);

#endif
