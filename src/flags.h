#ifndef SUNA_FLAGS_H
#define SUNA_FLAGS_H

#include <stdbool.h>

struct global_flags
{
    bool verbose;
    bool quiet;
    bool no_color;
    bool dry_run;
    bool help;
    bool version;
};

/* Walk argv, strip any recognized global flags, compact what's left in place.
 * Returns the new argc (argv is modified). After this call, argv[1] is the
 * subcommand and argv[2..] are the subcommand's own args and flags.
 */

int parse_global_flags(int argc, char **argv, struct global_flags *g);

#endif