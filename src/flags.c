#include "flags.h"
#include <string.h>

int parse_global_flags(int argc, char **argv, struct global_flags *g)
{
    int w = 1; /* write index, 0 is program name, always kept */

    for (int r = 1; r < argc; r++)
    {
        char *arg = argv[r];

        if (strcmp(arg, "--verbose") == 0)
        {
            g->verbose = true;
        }
        else if (strcmp(arg, "--quiet") == 0)
        {
            g->quiet = true;
        }
        else if (strcmp(arg, "--no-color") == 0)
        {
            g->no_color = true;
        }
        else if (strcmp(arg, "--dry-run") == 0)
        {
            g->dry_run = true;
        }
        else if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0)
        {
            g->help = true;
        }
        else if (strcmp(arg, "--version") == 0 || strcmp(arg, "-v") == 0)
        {
            g->version = true;
        }
        else
        {
            argv[w++] = arg;
        }
    }

    return w;
}
