#include <stdio.h>
#include <string.h>
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

    int target_count = w - 1;
    if (target_count == 0)
    {
        fprintf(stderr, "suna: *pouts* are you gonna give me something to blow up or what?\n");
        return 2;
    }

    // --- PRE-FLIGHT VALIDATION LOOP ---
    wchar_t wpath[MAX_PATH];
    bool has_dangerous = false;

    for (int i = 1; i <= target_count; i++)
    {
        normalize_path(argv[i], wpath, MAX_PATH);

        DWORD attrs = GetFileAttributesW(wpath);
        if (attrs == INVALID_FILE_ATTRIBUTES)
        {
            fprintf(stderr, "suna: lol you made that up. '%s' doesn't exist.\n", argv[i]);
            return 1;
        }

        bool is_sysattr = (attrs & FILE_ATTRIBUTE_SYSTEM) != 0;
        bool is_readonly = (attrs & FILE_ATTRIBUTE_READONLY) != 0;
        bool is_dangerous = is_dangerous_path(wpath) || is_sysattr;

        if (is_dangerous)
            has_dangerous = true;

        if (is_dangerous && !(opts.obliterate && opts.iafs))
        {
            fprintf(stderr, "suna: wow, '%s'? bold. type --obliterate -i-am-fucking-sure if you're actually insane.\n", argv[i]);
            return 2;
        }

        if (is_readonly && !opts.obliterate)
        {
            fprintf(stderr, "suna: '%s' is read-only. use --obliterate if you really wanna break it.\n", argv[i]);
            return 1;
        }

        if (!can_delete(wpath))
        {
            fprintf(stderr, "suna: '%s' is locked, dummy. someone else is holding it.\n", argv[i]);
            return 1;
        }
    }

    if (has_dangerous && !(opts.obliterate && opts.iafs))
    {
        fprintf(stderr, "suna: you snuck a dangerous path in here. i need --obliterate -i-am-fucking-sure for the whole batch.\n");
        return 2;
    }

    // --- DRY RUN ---
    if (g->dry_run)
    {
        printf("suna: *cocks bazooka* dry run time~\n");
        for (int i = 1; i <= target_count; i++)
        {
            normalize_path(argv[i], wpath, MAX_PATH);
            DWORD attrs = GetFileAttributesW(wpath);

            if (opts.obliterate)
            {
                printf("  %s -> OBLITERATED", argv[i]);
            }
            else
            {
                printf("  %s -> into the bin", argv[i]);
            }

            if ((attrs & FILE_ATTRIBUTE_SYSTEM) || is_dangerous_path(wpath))
            {
                printf("  [DANGEROUS]*");
            }
            if (attrs & FILE_ATTRIBUTE_READONLY)
            {
                printf("  [READ-ONLY]");
            }
            printf("\n");
        }
        if (has_dangerous)
            printf("\n* = system/protected. requires -i-am-fucking-sure\n");
        return 0;
    }

    // --- EXECUTION ---
    wchar_t *from_buf = calloc(MAX_PATH * target_count + 2, sizeof(wchar_t));
    wchar_t *ptr = from_buf;

    for (int i = 1; i <= target_count; i++)
    {
        normalize_path(argv[i], wpath, MAX_PATH);
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

    if (result != 0 || op.fAnyOperationsAborted)
    {
        fprintf(stderr, "suna: *frowns* the boom didn't work. (error %d)\n", result);
        return 1;
    }

    if (target_count == 1)
    {
        printf("suna: hehe, '%s' went boom.\n", argv[1]);
    }
    else
    {
        printf("suna: hehehe, wiped %d things off the map.\n", target_count);
    }

    return 0;
}