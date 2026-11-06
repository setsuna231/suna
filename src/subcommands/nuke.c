#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <shellapi.h>

#include "subcommands.h"
#include "../flags.h"
#include "../util.h"

struct nuke_opts
{
    bool obliterate;
    bool iafs;
};

int nuke(int argc, char **argv, struct global_flags *g)
{
    struct nuke_opts opts = {0};
    int w = 1;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--obliterate") == 0)
        {
            opts.obliterate = true;
        }
        else if (strcmp(argv[i], "-i-am-fucking-sure") == 0)
        {
            opts.iafs = true;
        }
        else
        {
            argv[w++] = argv[i];
        }
    }

    if (w == 1)
    {
        fprintf(stderr, "suna: *pouts* are you gonna give me something to blow up or what?\n");
        return 2;
    }

    int total_targets = 0;
    char **targets = NULL;

    for (int i = 1; i < w; i++)
    {
        int count = 0;
        char **expanded = expand_glob(argv[i], &count);
        if (count > 0)
        {
            targets = realloc(targets, (total_targets + count) * sizeof(char *));
            for (int j = 0; j < count; j++)
            {
                targets[total_targets++] = expanded[j];
            }
            free(expanded);
        }
    }

    if (total_targets == 0)
    {
        fprintf(stderr, "suna: lol you made that up. nothing matched.\n");
        free(targets);
        return 1;
    }

    wchar_t wpath[MAX_PATH];
    bool has_dangerous = false;

    for (int i = 0; i < total_targets; i++)
    {
        normalize_path(targets[i], wpath, MAX_PATH);

        DWORD attrs = GetFileAttributesW(wpath);
        if (attrs == INVALID_FILE_ATTRIBUTES)
        {
            fprintf(stderr, "suna: lol you made that up. '%s' doesn't exist.\n", targets[i]);
            free_glob_list(targets, total_targets);
            return 1;
        }

        bool is_sysattr = (attrs & FILE_ATTRIBUTE_SYSTEM) != 0;
        bool is_readonly = (attrs & FILE_ATTRIBUTE_READONLY) != 0;
        bool is_dangerous = is_dangerous_path(wpath) || is_sysattr;

        if (is_dangerous)
            has_dangerous = true;

        if (is_dangerous && !(opts.obliterate && opts.iafs))
        {
            fprintf(stderr, "suna: wow, '%s'? bold. type --obliterate -i-am-fucking-sure if you're actually insane.\n", targets[i]);
            free_glob_list(targets, total_targets);
            return 2;
        }

        if (is_readonly && !opts.obliterate)
        {
            fprintf(stderr, "suna: '%s' is read-only. use --obliterate if you really wanna break it.\n", targets[i]);
            free_glob_list(targets, total_targets);
            return 1;
        }

        if (!can_delete(wpath))
        {
            fprintf(stderr, "suna: '%s' is locked, dummy. someone else is holding it.\n", targets[i]);
            free_glob_list(targets, total_targets);
            return 1;
        }
    }

    if (has_dangerous && !(opts.obliterate && opts.iafs))
    {
        fprintf(stderr, "suna: you snuck a dangerous path in here. i need --obliterate -i-am-fucking-sure for the whole batch.\n");
        free_glob_list(targets, total_targets);
        return 2;
    }

    if (g->dry_run)
    {
        printf("suna: *cocks bazooka* dry run time~\n");
        for (int i = 0; i < total_targets; i++)
        {
            normalize_path(targets[i], wpath, MAX_PATH);
            DWORD attrs = GetFileAttributesW(wpath);

            if (opts.obliterate)
            {
                printf("  %s -> *vaporized*", targets[i]);
            }
            else
            {
                printf("  %s -> into the bin", targets[i]);
            }

            if ((attrs & FILE_ATTRIBUTE_SYSTEM) || is_dangerous_path(wpath))
            {
                printf("  *dangerous*");
            }
            if (attrs & FILE_ATTRIBUTE_READONLY)
            {
                printf("  *read-only*");
            }
            printf("\n");
        }
        if (has_dangerous)
            printf("\n* = system/protected. requires -i-am-fucking-sure\n");
        free_glob_list(targets, total_targets);
        return 0;
    }

    wchar_t *from_buf = calloc(MAX_PATH * total_targets + 2, sizeof(wchar_t));
    wchar_t *ptr = from_buf;

    for (int i = 0; i < total_targets; i++)
    {
        normalize_path(targets[i], wpath, MAX_PATH);
        DWORD attrs = GetFileAttributesW(wpath);

        if ((attrs & FILE_ATTRIBUTE_READONLY) && opts.obliterate)
        {
            SetFileAttributesW(wpath, attrs & ~FILE_ATTRIBUTE_READONLY);
        }

        wcscpy(ptr, wpath);
        ptr += wcslen(wpath) + 1;
    }
    *ptr = L'\0';

    SHFILEOPSTRUCTW op = {0};
    op.wFunc = FO_DELETE;
    op.pFrom = from_buf;
    op.fFlags = FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI | FOF_NO_UI;

    if (!opts.obliterate)
    {
        op.fFlags |= FOF_ALLOWUNDO;
    }

    int result = SHFileOperationW(&op);
    free(from_buf);
    free_glob_list(targets, total_targets);

    if (result != 0 || op.fAnyOperationsAborted)
    {
        fprintf(stderr, "suna: *frowns* the boom didn't work. (error %d)\n", result);
        return 1;
    }

    if (total_targets == 1)
    {
        printf("suna: *boom* hehe, it went boom.\n");
    }
    else
    {
        printf("suna: *boom* hehehe, wiped %d things off the map.\n", total_targets);
    }

    return 0;
}