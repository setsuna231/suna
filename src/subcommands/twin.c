#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>

#include "subcommands.h"
#include "../flags.h"
#include "../util.h"

int twin(int argc, char **argv, struct global_flags *g)
{
    int target_count = argc - 1;

    if (target_count == 0)
    {
        fprintf(stderr, "suna: twin what? give me a source and a destination.\n");
        return 2;
    }
    if (target_count == 1)
    {
        fprintf(stderr, "suna: you forgot where to put the copy, genius.\n");
        return 2;
    }
    if (target_count > 2)
    {
        fprintf(stderr, "suna: too many things! just a source and a destination.\n");
        return 2;
    }

    const char *raw_src = argv[1];
    const char *dst = argv[2];

    // Check if the destination has a trailing slash
    size_t dst_len = strlen(dst);
    bool dst_is_dir = (dst_len > 0 && (dst[dst_len - 1] == '\\' || dst[dst_len - 1] == '/'));

    wchar_t wdst[MAX_PATH];
    normalize_path(dst, wdst, MAX_PATH);

    // If they gave a trailing slash, make sure the folder actually exists
    if (dst_is_dir)
    {
        DWORD attrs = GetFileAttributesW(wdst);
        if (attrs == INVALID_FILE_ATTRIBUTES || !(attrs & FILE_ATTRIBUTE_DIRECTORY))
        {
            fprintf(stderr, "suna: '%s' isn't a real folder. where do you expect me to put this?\n", dst);
            return 1;
        }
    }

    // 1. Expand the source (so it handles *.txt)
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
        fprintf(stderr, "suna: you can't twin multiple files into one file! destination must be a folder.\n");
        free_glob_list(src_list, src_count);
        return 1;
    }

    // 2. Pre-flight Check (Loop through every file we matched)
    for (int i = 0; i < src_count; i++)
    {
        wchar_t wsrc[MAX_PATH];
        normalize_path(src_list[i], wsrc, MAX_PATH);

        DWORD src_attrs = GetFileAttributesW(wsrc);
        if (src_attrs == INVALID_FILE_ATTRIBUTES)
        {
            fprintf(stderr, "suna: i can't find '%s'.\n", src_list[i]);
            free_glob_list(src_list, src_count);
            return 1;
        }

        if (src_attrs & FILE_ATTRIBUTE_DIRECTORY)
        {
            fprintf(stderr, "suna: i can't twin whole folders yet! just files for now.\n");
            free_glob_list(src_list, src_count);
            return 1;
        }
    }

    // 3. Dry Run
    if (g->dry_run)
    {
        printf("suna: *clones* dry run - would twin %d file(s) to '%s'.\n", src_count, dst);
        free_glob_list(src_list, src_count);
        return 0;
    }

    // 4. Execution Loop (Copy each file)
    for (int i = 0; i < src_count; i++)
    {
        wchar_t wsrc[MAX_PATH];
        normalize_path(src_list[i], wsrc, MAX_PATH);

        wchar_t final_wdst[MAX_PATH];

        if (dst_is_dir)
        {
            // Extract the base filename from the source
            const char *base = strrchr(src_list[i], '\\');
            if (!base)
                base = strrchr(src_list[i], '/');
            if (base)
                base++;
            else
                base = src_list[i];

            // Construct the new path: <dst>/<basename>
            char final_dst_str[MAX_PATH];
            snprintf(final_dst_str, sizeof(final_dst_str), "%s%s", dst, base);
            normalize_path(final_dst_str, final_wdst, MAX_PATH);
        }
        else
        {
            wcscpy(final_wdst, wdst);
        }

        // CopyFileW's third argument (TRUE) means "do not overwrite if exists"
        if (!CopyFileW(wsrc, final_wdst, TRUE))
        {
            DWORD err = GetLastError();
            if (err == ERROR_ALREADY_EXISTS)
            {
                fprintf(stderr, "suna: '%s' is already there. i'm not overwriting your stuff.\n", src_list[i]);
            }
            else if (err == ERROR_ACCESS_DENIED)
            {
                fprintf(stderr, "suna: i'm not allowed to copy there.\n");
            }
            else
            {
                fprintf(stderr, "suna: *grunts* couldn't copy '%s' (error %lu).\n", src_list[i], err);
            }
            free_glob_list(src_list, src_count);
            return 1;
        }
    }

    // 5. Success
    if (src_count == 1)
    {
        printf("suna: *twin* '%s' has a clone at '%s'.\n", src_list[0], dst);
    }
    else
    {
        printf("suna: *twin* cloned %d files into '%s'.\n", src_count, dst);
    }

    free_glob_list(src_list, src_count);
    return 0;
}