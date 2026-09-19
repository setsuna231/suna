#include "flags.h"
#include "dispatch.h"
#include <stdio.h>

static void print_version(void)
{
    printf("suna: 0.0.0\n");
}

static void print_help(void)
{
    printf("usage: suna <command> [args]\n");
    printf("\n");
    printf("commands:\n");
    printf("  nuke      delete (moves to recycle bin)\n");
    printf("  rummage   list directory\n");
    printf("  peek      read file\n");
    printf("  forage    search files and content\n");
    printf("  poof      create file or directory\n");
    printf("  scribble  edit file\n");
    printf("  shove     move / rename\n");
    printf("  twin      copy\n");
}

int main(int argc, char **argv)
{
    struct global_flags g = {0};

    int new_argc = parse_global_flags(argc, argv, &g);

    if (g.version)
    {
        print_version();
        return 0;
    }

    if (g.help)
    {
        print_help();
        return 0;
    }

    return dispatch(new_argc, argv, &g);
}