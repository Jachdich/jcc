#ifndef __ARGS_H
#define __ARGS_H

struct Args {
    int status;
    int debug;
    char *ifname;
    char *ofname;
};

typedef struct Args Args;

void usage(char *ename);

Args parse_args(int argc, char **argv);

#endif
