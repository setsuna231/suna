#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>
#include <windows.h>
#include <stdlib.h>
#include "util.h"

void to_wide(const char *input, wchar_t *output, int out_size)
{
    MultiByteToWideChar(CP_UTF8, 0, input, -1, output, out_size);
}

void to_utf8(const wchar_t *input, char *output, int out_size)
{
    WideCharToMultiByte(CP_UTF8, 0, input, -1, output, out_size, NULL, NULL);
}

void normalize_path(const char *input, wchar_t *output, int out_size)
{
    wchar_t winput[MAX_PATH];
    wchar_t expanded[MAX_PATH];

    to_wide(input, winput, MAX_PATH);
    ExpandEnvironmentStringsW(winput, expanded, MAX_PATH);

    // Resolves . and .. and makes it absolute
    GetFullPathNameW(expanded, out_size, output, NULL);

    // Lowercase for safe comparison
    for (wchar_t *p = output; *p; p++)
        *p = towlower(*p);

    // Strip trailing slash (but don't nuke C:\)
    size_t len = wcslen(output);
    if (len > 1 && (output[len - 1] == L'\\' || output[len - 1] == L'/'))
    {
        if (len > 3 || output[1] != L':')
        {
            output[len - 1] = L'\0';
        }
    }
}

static bool is_under_w(const wchar_t *path, const wchar_t *root)
{
    size_t root_len = wcslen(root);
    if (wcsncmp(path, root, root_len) != 0)
        return false;
    if (path[root_len] == L'\0')
        return true;
    return path[root_len] == L'\\';
}

bool is_dangerous_path(const wchar_t *abs_path)
{
    // 1. Drive roots (e.g. "c:\")
    if (wcslen(abs_path) == 3 && abs_path[1] == L':' && abs_path[2] == L'\\')
        return true;

    // 2. Env vars
    const wchar_t *env_vars[] = {
        L"SystemRoot", L"ProgramFiles", L"ProgramFiles(x86)",
        L"ProgramData", L"USERPROFILE"};

    wchar_t buf[MAX_PATH];
    for (int i = 0; i < 5; i++)
    {
        if (GetEnvironmentVariableW(env_vars[i], buf, MAX_PATH) > 0)
        {
            for (wchar_t *p = buf; *p; p++)
                *p = towlower(*p);
            size_t len = wcslen(buf);
            if (len > 1 && (buf[len - 1] == L'\\' || buf[len - 1] == L'/'))
            {
                if (len > 3 || buf[1] != L':')
                    buf[len - 1] = L'\0';
            }
            if (is_under_w(abs_path, buf))
                return true;
        }
    }

    // 3. System and User PATH
    if (GetEnvironmentVariableW(L"PATH", buf, MAX_PATH) > 0)
    {
        wchar_t *ctx = NULL;
        wchar_t *token = wcstok_s(buf, L";", &ctx);
        while (token)
        {
            for (wchar_t *p = token; *p; p++)
                *p = towlower(*p);
            size_t len = wcslen(token);
            if (len > 1 && (token[len - 1] == L'\\' || token[len - 1] == L'/'))
            {
                if (len > 3 || token[1] != L':')
                    token[len - 1] = L'\0';
            }
            if (wcslen(token) > 0 && is_under_w(abs_path, token))
                return true;
            token = wcstok_s(NULL, L";", &ctx);
        }
    }

    return false;
}

bool can_delete(const wchar_t *wpath)
{
    HANDLE h = CreateFileW(
        wpath,
        DELETE,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS, // Required for directories
        NULL);

    if (h == INVALID_HANDLE_VALUE)
        return false;

    CloseHandle(h);
    return true;
}

char **expand_glob(const char *pattern, int *out_count)
{
    *out_count = 0;

    // No wildcards? Just return the pattern as a single-item list.
    if (strchr(pattern, '*') == NULL && strchr(pattern, '?') == NULL)
    {
        char **list = malloc(sizeof(char *));
        list[0] = _strdup(pattern);
        *out_count = 1;
        return list;
    }

    wchar_t wpattern[MAX_PATH];
    to_wide(pattern, wpattern, MAX_PATH);

    WIN32_FIND_DATAW find_data;
    HANDLE hFind = FindFirstFileW(wpattern, &find_data);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        return NULL; // No matches found
    }

    int capacity = 10;
    char **list = malloc(capacity * sizeof(char *));

    do
    {
        if (wcscmp(find_data.cFileName, L".") == 0 || wcscmp(find_data.cFileName, L"..") == 0)
        {
            continue;
        }

        if (*out_count >= capacity)
        {
            capacity *= 2;
            list = realloc(list, capacity * sizeof(char *));
        }

        char utf8_name[MAX_PATH];
        to_utf8(find_data.cFileName, utf8_name, MAX_PATH);

        // Preserve the directory prefix if one was provided (e.g., "src/*.c")
        const char *last_slash = strrchr(pattern, '\\');
        if (!last_slash)
            last_slash = strrchr(pattern, '/');

        if (last_slash)
        {
            int prefix_len = (int)(last_slash - pattern) + 1;
            char full_path[MAX_PATH];
            strncpy(full_path, pattern, prefix_len);
            full_path[prefix_len] = '\0';
            strcat(full_path, utf8_name);
            list[*out_count] = _strdup(full_path);
        }
        else
        {
            list[*out_count] = _strdup(utf8_name);
        }

        (*out_count)++;

    } while (FindNextFileW(hFind, &find_data) != 0);

    FindClose(hFind);
    return list;
}

void free_glob_list(char **list, int count)
{
    if (!list)
        return;
    for (int i = 0; i < count; i++)
    {
        free(list[i]);
    }
    free(list);
}