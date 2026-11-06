#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>

#include "subcommands.h"
#include "../flags.h"
#include "../util.h"

int shove(int argc, char **argv, struct global_flags *g)
{
    int target_count = argc - 1;

    if (target_count < 2)
    {
        fprintf(stderr, "suna: i need a source and a destination.\n");
        return 2;
    }
    if (target_count > 2)
    {
        fprintf(stderr, "suna: just a source and a destination. i'm not a moving truck.\n");
        return 2;
    }

    const char *raw_src = argv[1];
    const char *dst = argv[2];

    size_t dst_len = strlen(dst);
    bool dst_is_dir = (dst_len > 0 && (dst[dst_len - 1] == '\\' || dst[dst_len - 1] == '/'));

    wchar_t wdst[MAX_PATH];
    normalize_path(dst, wdst, MAX_PATH);

    if (dst_is_dir)
    {
        DWORD attrs = GetFileAttributesW(wdst);
        if (attrs == INVALID_FILE_ATTRIBUTES || !(attrs & FILE_ATTRIBUTE_DIRECTORY))
        {
            fprintf(stderr, "suna: '%s' isn't a real folder. where do you expect me to put this?\n", dst);
            return 1;
        }
    }

    // Expand the source (supporting globs)
    int src_count = 0;
    char **src_list = expand_glob(raw_src, &src_count);

    if (src_count == 0)
    {
        fprintf(stderr, "suna: i can't find '%s'. did you hallucinate it?\n", raw_src);
        free(src_list);
        return 1;
    }
    if (src_count > 1 && !dst_is_dir)
    {
        fprintf(stderr, "suna: you can't shove multiple files into one file! destination must be a folder.\n");
        free_glob_list(src_list, src_count);
        return 1;
    }

    // Pre-flight
    for (int i = 0; i < src_count; i++)
    {
        wchar_t wsrc[MAX_PATH];
        normalize_path(src_list[i], wsrc, MAX_PATH);

        if (GetFileAttributesW(wsrc) == INVALID_FILE_ATTRIBUTES)
        {
            fprintf(stderr, "suna: i can't find '%s'.\n", src_list[i]);
            free_glob_list(src_list, src_count);
            return 1;
        }

        if (!can_delete(wsrc))
        {
            fprintf(stderr, "suna: '%s' is locked. someone's using it.\n", src_list[i]);
            free_glob_list(src_list, src_count);
            return 1;
        }
    }

    if (g->dry_run)
    {
        printf("suna: *scoots* dry run - would shove %d thing(s) to '%s'.\n", src_count, dst);
        free_glob_list(src_list, src_count);
        return 0;
    }

    // Execution
    for (int i = 0; i < src_count; i++)
    {
        wchar_t wsrc[MAX_PATH];
        normalize_path(src_list[i], wsrc, MAX_PATH);

        wchar_t final_wdst[MAX_PATH];
        char final_dst_str[MAX_PATH];

        if (dst_is_dir)
        {
            const char *base = strrchr(src_list[i], '\\');
            if (!base)
                base = strrchr(src_list[i], '/');
            if (base)
                base++;
            else
                base = src_list[i];

            snprintf(final_dst_str, sizeof(final_dst_str), "%s%s", dst, base);
            normalize_path(final_dst_str, final_wdst, MAX_PATH);
        }
        else
        {
            wcscpy(final_wdst, wdst);
        }

        if (!MoveFileW(wsrc, final_wdst))
        {
            DWORD err = GetLastError();
            if (err == ERROR_ALREADY_EXISTS)
            {
                fprintf(stderr, "suna: '%s' is already there. i'm not overwriting your stuff.\n", src_list[i]);
            }
            else if (err == ERROR_NOT_SAME_DEVICE)
            {
                fprintf(stderr, "suna: that's too far! i don't cross drives yet.\n");
            }
            else
            {
                fprintf(stderr, "suna: *grunts* couldn't move '%s' (error %lu).\n", src_list[i], err);
            }
            free_glob_list(src_list, src_count);
            return 1;
        }
    }

    if (src_count == 1)
    {
        printf("suna: *shove* '%s' is now at '%s'.\n", src_list[0], dst);
    }
    else
    {
        printf("suna: *shove* moved %d things into '%s'.\n", src_count, dst);
    }

    free_glob_list(src_list, src_count);
    return 0;
}