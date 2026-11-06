#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>

#include "subcommands.h"
#include "../flags.h"
#include "../util.h"

// Helper function for the recursive search
static void forage_dir(const wchar_t *dir_path, const char *pattern, bool *found_any)
{
    wchar_t search_path[MAX_PATH];
    if (wcslen(dir_path) > 0 && dir_path[wcslen(dir_path) - 1] == L'\\')
    {
        swprintf(search_path, MAX_PATH, L"%ls*", dir_path);
    }
    else
    {
        swprintf(search_path, MAX_PATH, L"%ls\\*", dir_path);
    }

    WIN32_FIND_DATAW find_data;
    HANDLE hFind = FindFirstFileW(search_path, &find_data);

    if (hFind == INVALID_HANDLE_VALUE)
        return; // Can't read this dir, skip it

    do
    {
        // Skip . and ..
        if (wcscmp(find_data.cFileName, L".") == 0 || wcscmp(find_data.cFileName, L"..") == 0)
        {
            continue;
        }

        wchar_t full_path[MAX_PATH];
        if (dir_path[wcslen(dir_path) - 1] == L'\\')
        {
            swprintf(full_path, MAX_PATH, L"%ls%ls", dir_path, find_data.cFileName);
        }
        else
        {
            swprintf(full_path, MAX_PATH, L"%ls\\%ls", dir_path, find_data.cFileName);
        }

        char utf8_path[MAX_PATH];
        to_utf8(full_path, utf8_path, MAX_PATH);

        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            // It's a folder, recurse into it
            forage_dir(full_path, pattern, found_any);
        }
        else
        {
            // It's a file, check the filename first
            char utf8_name[MAX_PATH];
            to_utf8(find_data.cFileName, utf8_name, MAX_PATH);

            if (strstr(utf8_name, pattern) != NULL)
            {
                printf("*name*    %s\n", utf8_path);
                *found_any = true;
            }

            // Now check the content
            HANDLE hFile = CreateFileW(full_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile != INVALID_HANDLE_VALUE)
            {
                char buffer[4096];
                DWORD bytes_read = 0;
                bool match_in_content = false;

                while (ReadFile(hFile, buffer, sizeof(buffer) - 1, &bytes_read, NULL) && bytes_read > 0)
                {
                    buffer[bytes_read] = '\0'; // Null-terminate for strstr
                    if (strstr(buffer, pattern) != NULL)
                    {
                        match_in_content = true;
                        break; // Found it, no need to read the rest of the file
                    }
                }
                CloseHandle(hFile);

                if (match_in_content)
                {
                    printf("*content* %s\n", utf8_path);
                    *found_any = true;
                }
            }
        }
    } while (FindNextFileW(hFind, &find_data) != 0);

    FindClose(hFind);
}

int forage(int argc, char **argv, struct global_flags *g)
{
    if (argc < 2)
    {
        fprintf(stderr, "suna: forage for what? give me a pattern.\n");
        return 2;
    }

    const char *pattern = argv[1];
    const char *target_dir = (argc >= 3) ? argv[2] : ".";

    wchar_t wpath[MAX_PATH];
    normalize_path(target_dir, wpath, MAX_PATH);

    DWORD attrs = GetFileAttributesW(wpath);
    if (attrs == INVALID_FILE_ATTRIBUTES || !(attrs & FILE_ATTRIBUTE_DIRECTORY))
    {
        fprintf(stderr, "suna: '%s' isn't a folder. i can't forage there.\n", target_dir);
        return 1;
    }

    if (g->dry_run)
    {
        printf("suna: *sniffs the air* dry run - would forage for '%s' in '%s'.\n", pattern, target_dir);
        return 0;
    }

    bool found_any = false;
    forage_dir(wpath, pattern, &found_any);

    if (!found_any)
    {
        printf("suna: i sniffed around but couldn't find '%s' anywhere.\n", pattern);
    }

    return 0;
}