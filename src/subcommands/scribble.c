#include <stdio.h>
#include "subcommands.h"

int scribble(int argc, char **argv, struct global_flags *g)
{
    (void)argc;
    (void)argv;
    (void)g;
    printf("scribble: not implemented yet\n");
    return 1;
}