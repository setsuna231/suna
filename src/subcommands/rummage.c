#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>

#include "subcommands.h"
#include "../flags.h"
#include "../util.h"

int rummage(int argc, char **argv, struct global_flags *g)
{
    int target_count = argc - 1;

    const char *target = (target_count == 0) ? "." : argv[1];

    if (target_count > 1)
    {
        fprintf(stderr, "suna: one folder or pattern at a time, greedy!\n");
        return 2;
    }

    wchar_t wpath[MAX_PATH];
    normalize_path(target, wpath, MAX_PATH);

    // Check if the target is an existing directory
    DWORD attrs = GetFileAttributesW(wpath);
    bool is_dir = (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY));

    if (attrs == INVALID_FILE_ATTRIBUTES && strchr(target, '*') == NULL && strchr(target, '?') == NULL)
    {
        fprintf(stderr, "suna: lol, i can't find '%s'. did you make it up?\n", target);
        return 1;
    }
    if (attrs != INVALID_FILE_ATTRIBUTES && !is_dir)
    {
        fprintf(stderr, "suna: '%s' is a file, dummy. i can't rummage through that.\n", target);
        return 1;
    }

    if (g->dry_run)
    {
        printf("suna: *digs around* dry run - would rummage through '%s'.\n", target);
        return 0;
    }

    wchar_t search_path[MAX_PATH];

    if (is_dir)
    {
        // If it's a folder, list everything inside it
        if (wcslen(wpath) > 0 && wpath[wcslen(wpath) - 1] == L'\\')
        {
            swprintf(search_path, MAX_PATH, L"%ls*", wpath);
        }
        else
        {
            swprintf(search_path, MAX_PATH, L"%ls\\*", wpath);
        }
    }
    else
    {
        // It's a wildcard pattern (e.g., *.c or src/*.h), use it directly
        wcscpy(search_path, wpath);
    }

    WIN32_FIND_DATAW find_data;
    HANDLE hFind = FindFirstFileW(search_path, &find_data);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        if (is_dir)
        {
            printf("suna: it's empty in here... spooky.\n");
        }
        else
        {
            fprintf(stderr, "suna: i found nothing matching '%s'. did you make it up?\n", target);
        }
        return 1;
    }

    char utf8_name[MAX_PATH];
    bool found_anything = false;

    do
    {
        if (wcscmp(find_data.cFileName, L".") == 0 || wcscmp(find_data.cFileName, L"..") == 0)
        {
            continue;
        }

        found_anything = true;
        to_utf8(find_data.cFileName, utf8_name, MAX_PATH);

        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            printf("*dir*  %s\n", utf8_name);
        }
        else
        {
            printf("*file* %s\n", utf8_name);
        }

    } while (FindNextFileW(hFind, &find_data) != 0);

    FindClose(hFind);

    if (!found_anything)
    {
        printf("suna: i found nothing matching '%s'.\n", target);
    }

    return 0;
}