#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>

#include "subcommands.h"
#include "../flags.h"
#include "../util.h"

struct poof_opts
{
    bool dir;
};

int poof(int argc, char **argv, struct global_flags *g)
{
    struct poof_opts opts = {0};
    int w = 1;

    // Parse flags
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--dir") == 0)
        {
            opts.dir = true;
        }
        else
        {
            argv[w++] = argv[i];
        }
    }

    int target_count = w - 1;
    if (target_count == 0)
    {
        fprintf(stderr, "suna: poof what? you didn't give me a name.\n");
        return 2;
    }
    if (target_count > 1)
    {
        fprintf(stderr, "suna: one thing at a time! i can't poof a whole army.\n");
        return 2;
    }

    const char *target = argv[1];
    wchar_t wpath[MAX_PATH];
    to_wide(target, wpath, MAX_PATH);

    // Check if it already exists
    DWORD attrs = GetFileAttributesW(wpath);
    if (attrs != INVALID_FILE_ATTRIBUTES)
    {
        fprintf(stderr, "suna: uh, '%s' is already here. i'm not a duplicator.\n", target);
        return 1;
    }

    // Dry Run
    if (g->dry_run)
    {
        if (opts.dir)
        {
            printf("suna: *waves wand* dry run - would poof directory '%s'.\n", target);
        }
        else
        {
            printf("suna: *waves wand* dry run - would poof file '%s'.\n", target);
        }
        return 0;
    }

    // Execution
    bool success = false;
    DWORD err = 0;

    if (opts.dir)
    {
        if (CreateDirectoryW(wpath, NULL))
        {
            success = true;
        }
        else
        {
            err = GetLastError();
        }
    }
    else
    {
        // CREATE_NEW ensures we fail if the file already exists
        HANDLE h = CreateFileW(wpath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h != INVALID_HANDLE_VALUE)
        {
            CloseHandle(h);
            success = true;
        }
        else
        {
            err = GetLastError();
        }
    }

    // Error handling
    if (!success)
    {
        if (err == ERROR_PATH_NOT_FOUND)
        {
            fprintf(stderr, "suna: where is this supposed to go? the folder doesn't exist.\n");
        }
        else if (err == ERROR_ACCESS_DENIED)
        {
            fprintf(stderr, "suna: i'm not allowed to poof there.\n");
        }
        else
        {
            fprintf(stderr, "suna: *fizzle* something went wrong (error %lu).\n", err);
        }
        return 1;
    }

    // Success
    if (opts.dir)
    {
        printf("suna: *poof* directory '%s' appeared.\n", target);
    }
    else
    {
        printf("suna: *poof* file '%s' appeared.\n", target);
    }

    return 0;
}