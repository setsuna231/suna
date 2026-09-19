#include <stdio.h>
#include "subcommands.h"

int poof(int argc, char **argv, struct global_flags *g)
{
    (void)argc;
    (void)argv;
    (void)g;
    printf("poof: not implemented yet\n");
    return 1;
}