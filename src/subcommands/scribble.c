#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>

#include "subcommands.h"
#include "../flags.h"
#include "../util.h"

int scribble(int argc, char **argv, struct global_flags *g)
{
    const char *editor = NULL;
    int w = 1;

    // 1. Parse the --use flag
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--use") == 0)
        {
            if (i + 1 < argc)
            {
                editor = argv[++i];
            }
            else
            {
                fprintf(stderr, "suna: you forgot to tell me which editor to use after --use.\n");
                return 2;
            }
        }
        else
        {
            argv[w++] = argv[i];
        }
    }

    if (w < 2)
    {
        fprintf(stderr, "suna: scribble on what? give me a file.\n");
        return 2;
    }
    if (w > 2)
    {
        fprintf(stderr, "suna: one file at a time!\n");
        return 2;
    }

    const char *target = argv[1];
    wchar_t wpath[MAX_PATH];
    normalize_path(target, wpath, MAX_PATH);

    if (g->dry_run)
    {
        printf("suna: *grabs pen* dry run - would open '%s' in an editor.\n", target);
        return 0;
    }

    // Pre-create the file if it doesn't exist so editors don't prompt
    if (GetFileAttributesW(wpath) == INVALID_FILE_ATTRIBUTES)
    {
        HANDLE h = CreateFileW(wpath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h != INVALID_HANDLE_VALUE)
        {
            CloseHandle(h);
        }
    }

    // 2. Determine the editor to use
    char editor_cmd[MAX_PATH] = "";

    if (editor != NULL)
    {
        // User explicitly provided --use
        strncpy(editor_cmd, editor, MAX_PATH - 1);
    }
    else
    {
        // Check the EDITOR environment variable
        const char *env_editor = getenv("EDITOR");
        if (env_editor != NULL && strlen(env_editor) > 0)
        {
            strncpy(editor_cmd, env_editor, MAX_PATH - 1);
        }
        else
        {
            // Check if 'micro' is in the PATH
            char micro_path[MAX_PATH];
            if (SearchPathA(NULL, "micro.exe", NULL, MAX_PATH, micro_path, NULL) > 0)
            {
                strcpy(editor_cmd, "micro");
            }
            else
            {
                // Absolute fallback
                strcpy(editor_cmd, "notepad.exe");
            }
        }
    }

    // 3. Build the command line
    // We must quote the file path in case it has spaces (e.g. "C:\My Folder\file.txt")
    wchar_t weditor[MAX_PATH];
    to_wide(editor_cmd, weditor, MAX_PATH);

    wchar_t cmd_line[MAX_PATH * 2];
    swprintf(cmd_line, MAX_PATH * 2, L"\"%ls\" \"%ls\"", weditor, wpath);

    // 4. Launch the process
    STARTUPINFOW si = {0};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {0};

    // CreateProcessW needs the command line to be a modifiable buffer, hence the _wcsdup
    wchar_t *cmd_buffer = _wcsdup(cmd_line);

    if (!CreateProcessW(
            NULL,       // Application name (NULL means parse from command line)
            cmd_buffer, // Command line
            NULL,       // Process attributes
            NULL,       // Thread attributes
            FALSE,      // Inherit handles
            0,          // Creation flags
            NULL,       // Environment
            NULL,       // Current directory
            &si,        // Startup info
            &pi         // Process info
            ))
    {
        DWORD err = GetLastError();
        fprintf(stderr, "suna: *fumbles* couldn't open the editor '%s' (error %lu).\n", editor_cmd, err);
        free(cmd_buffer);
        return 1;
    }

    // Wait for the editor to close
    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    free(cmd_buffer);

    printf("suna: *puts pen away* finished scribbling on '%s'.\n", target);

    return 0;
}