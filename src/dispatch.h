#ifndef SUNA_DISPATCH_H
#define SUNA_DISPATCH_H

#include "flags.h"

/* Every subcommand has this signature. argc/argv here are what's LEFT
 * after global flags were stripped. */
typedef int (*command_fn)(int argc, char **argv, struct global_flags *g);

// One row per subcommand.
struct command
{
    const char *name;
    command_fn fn;
};

// Look up argv[1] in the table and run it. Returns exit code.

int dispatch(int argc, char **argv, struct global_flags *g);

#endif