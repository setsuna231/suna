#ifndef SUNA_SUBCOMMANDS_H
#define SUNA_SUBCOMMANDS_H

#include "../flags.h"

int nuke(int argc, char **argv, struct global_flags *g);
int rummage(int argc, char **argv, struct global_flags *g);
int peek(int argc, char **argv, struct global_flags *g);
int forage(int argc, char **argv, struct global_flags *g);
int poof(int argc, char **argv, struct global_flags *g);
int scribble(int argc, char **argv, struct global_flags *g);
int shove(int argc, char **argv, struct global_flags *g);
int twin(int argc, char **argv, struct global_flags *g);

#endif