#ifndef SUNA_UTIL_H
#define SUNA_UTIL_H

#include <stdbool.h>
#include <wchar.h>

// Convert between UTF-8 (argv) and UTF-16 (Windows API)
void to_wide(const char *input, wchar_t *output, int out_size);
void to_utf8(const wchar_t *input, char *output, int out_size);

// Normalizes to absolute, lowercase, backslashes, no trailing slash (except C:\)
void normalize_path(const char *input, wchar_t *output, int out_size);

// Checks against system roots, env vars, and PATH
bool is_dangerous_path(const wchar_t *abs_path);

// Checks if the file is locked or denies delete access
bool can_delete(const wchar_t *wpath);

// Expands wildcards (*, ?) into a list of actual filenames
char **expand_glob(const char *pattern, int *out_count);
void free_glob_list(char **list, int count);

#endif