#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>

#include "subcommands.h"
#include "../flags.h"
#include "../util.h"

int peek(int argc, char **argv, struct global_flags *g)
{
    int max_lines = -1;
    int w = 1;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--squint") == 0)
        {
            if (i + 1 < argc)
            {
                max_lines = atoi(argv[++i]);
                if (max_lines <= 0)
                {
                    fprintf(stderr, "suna: squint how much? give me a real number.\n");
                    return 2;
                }
            }
            else
            {
                fprintf(stderr, "suna: you forgot the number after --squint.\n");
                return 2;
            }
        }
        else
        {
            argv[w++] = argv[i];
        }
    }

    int target_count = w - 1;

    if (target_count == 0)
    {
        fprintf(stderr, "suna: peek at what? give me a file.\n");
        return 2;
    }
    if (target_count > 1)
    {
        fprintf(stderr, "suna: one file at a time!\n");
        return 2;
    }

    const char *target = argv[1];
    wchar_t wpath[MAX_PATH];
    normalize_path(target, wpath, MAX_PATH);

    DWORD attrs = GetFileAttributesW(wpath);
    if (attrs == INVALID_FILE_ATTRIBUTES)
    {
        fprintf(stderr, "suna: i can't find '%s'.\n", target);
        return 1;
    }

    if (attrs & FILE_ATTRIBUTE_DIRECTORY)
    {
        fprintf(stderr, "suna: '%s' is a folder, dummy. i can't peek inside a folder.\n", target);
        return 1;
    }

    if (g->dry_run)
    {
        if (max_lines != -1)
        {
            printf("suna: *squints* dry run - would peek at the first %d lines of '%s'.\n", max_lines, target);
        }
        else
        {
            printf("suna: *squints* dry run - would peek at all of '%s'.\n", target);
        }
        return 0;
    }

    HANDLE h = CreateFileW(wpath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "suna: i can't open '%s'. access denied?\n", target);
        return 1;
    }

    char buffer[4096];
    DWORD bytes_read = 0;
    BOOL success = FALSE;
    int lines_printed = 0;
    bool first_chunk = true;

    while ((success = ReadFile(h, buffer, sizeof(buffer), &bytes_read, NULL)) && bytes_read > 0)
    {

        DWORD start_idx = 0;

        // BOM Detection (only on the first chunk)
        if (first_chunk)
        {
            first_chunk = false;

            // UTF-8 BOM: EF BB BF
            if (bytes_read >= 3 && (unsigned char)buffer[0] == 0xEF && (unsigned char)buffer[1] == 0xBB && (unsigned char)buffer[2] == 0xBF)
            {
                start_idx = 3;
            }
            // UTF-16 LE BOM: FF FE
            else if (bytes_read >= 2 && (unsigned char)buffer[0] == 0xFF && (unsigned char)buffer[1] == 0xFE)
            {
                start_idx = 2;
                // Note: This will still print UTF-16 text with nulls interleaved,
                // but at least the weird box is gone.
            }
        }

        for (DWORD i = start_idx; i < bytes_read; i++)
        {
            fputc(buffer[i], stdout);

            if (buffer[i] == '\n')
            {
                lines_printed++;
                if (max_lines != -1 && lines_printed >= max_lines)
                {
                    goto done_reading;
                }
            }
        }
    }

done_reading:
    CloseHandle(h);

    if (!success)
    {
        fprintf(stderr, "\nsuna: *hiccup* something went wrong while reading.\n");
        return 1;
    }

    if (max_lines != -1 && lines_printed == max_lines)
    {
        printf("\n");
    }

    return 0;
}