#include <stdio.h>
#include <string.h>
#include "dispatch.h"
#include "subcommands/subcommands.h"

static const struct command commands[] = {
    {"nuke", nuke},
    {"rummage", rummage},
    {"peek", peek},
    {"forage", forage},
    {"poof", poof},
    {"scribble", scribble},
    {"shove", shove},
    {"twin", twin},
};

#define NUM_COMMANDS (sizeof(commands) / sizeof(commands[0]))

int dispatch(int argc, char **argv, struct global_flags *g)
{
    if (argc < 2)
    {
        fprintf(stderr, "suna: no command given\n");
        return 2;
    }

    const char *name = argv[1];

    for (size_t i = 0; i < NUM_COMMANDS; i++)
    {
        if (strcmp(name, commands[i].name) == 0)
        {
            return commands[i].fn(argc - 1, argv + 1, g);
        }
    }

    fprintf(stderr, "suna: unknown command '%s'\n", name);
    return 2;
}