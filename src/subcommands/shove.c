#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>

#include "subcommands.h"
#include "../flags.h"
#include "../util.h"

int shove(int argc, char **argv, struct global_flags *g)
{
    int target_count = argc - 1;

    if (target_count == 0)
    {
        fprintf(stderr, "suna: shove what where? give me a source and a destination.\n");
        return 2;
    }
    if (target_count == 1)
    {
        fprintf(stderr, "suna: you forgot the destination, genius.\n");
        return 2;
    }
    if (target_count > 2)
    {
        fprintf(stderr, "suna: too many things! just a source and a destination.\n");
        return 2;
    }

    const char *src = argv[1];
    const char *dst = argv[2];

    // Check if user explicitly meant a directory by adding a slash
    size_t dst_len = strlen(dst);
    bool dst_is_dir = (dst_len > 0 && (dst[dst_len - 1] == '\\' || dst[dst_len - 1] == '/'));

    wchar_t wsrc[MAX_PATH];
    wchar_t wdst[MAX_PATH];

    normalize_path(src, wsrc, MAX_PATH);
    normalize_path(dst, wdst, MAX_PATH);

    // Check source exists
    if (GetFileAttributesW(wsrc) == INVALID_FILE_ATTRIBUTES)
    {
        fprintf(stderr, "suna: i can't find '%s'. did you hallucinate it?\n", src);
        return 1;
    }

    char final_dst_str[MAX_PATH];

    if (dst_is_dir)
    {
        // Ensure the destination directory actually exists
        DWORD attrs = GetFileAttributesW(wdst);
        if (attrs == INVALID_FILE_ATTRIBUTES || !(attrs & FILE_ATTRIBUTE_DIRECTORY))
        {
            fprintf(stderr, "suna: '%s' isn't a real folder. where do you expect me to put this?\n", dst);
            return 1;
        }

        // Extract the base filename from the source
        const char *base = strrchr(src, '\\');
        if (!base)
            base = strrchr(src, '/');
        if (base)
            base++;
        else
            base = src;

        // Construct the new path: <dst>/<basename>
        snprintf(final_dst_str, sizeof(final_dst_str), "%s%s", dst, base);
        normalize_path(final_dst_str, wdst, MAX_PATH);
    }

    // Check destination DOES NOT exist (whether it's a file or inside a folder)
    if (GetFileAttributesW(wdst) != INVALID_FILE_ATTRIBUTES)
    {
        fprintf(stderr, "suna: '%s' is already there. i'm not overwriting your stuff.\n", dst_is_dir ? final_dst_str : dst);
        return 1;
    }

    // Check if source is locked
    if (!can_delete(wsrc))
    {
        fprintf(stderr, "suna: '%s' is locked. someone's using it.\n", src);
        return 1;
    }

    // Dry Run
    if (g->dry_run)
    {
        if (dst_is_dir)
        {
            printf("suna: *scoots* dry run - would shove '%s' into folder '%s'.\n", src, dst);
        }
        else
        {
            printf("suna: *scoots* dry run - would shove '%s' to '%s'.\n", src, dst);
        }
        return 0;
    }

    // Execution
    if (MoveFileW(wsrc, wdst))
    {
        if (dst_is_dir)
        {
            printf("suna: *shove* '%s' is now inside '%s'.\n", src, dst);
        }
        else
        {
            printf("suna: *shove* '%s' is now '%s'.\n", src, dst);
        }
        return 0;
    }
    else
    {
        DWORD err = GetLastError();
        if (err == ERROR_NOT_SAME_DEVICE)
        {
            fprintf(stderr, "suna: that's too far! i don't cross drives yet.\n");
        }
        else if (err == ERROR_ACCESS_DENIED)
        {
            fprintf(stderr, "suna: i'm not allowed to move that.\n");
        }
        else
        {
            fprintf(stderr, "suna: *grunts* couldn't move it (error %lu).\n", err);
        }
        return 1;
    }
}