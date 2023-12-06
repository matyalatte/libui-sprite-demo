#ifdef _WIN32
#include <Windows.h>
#include <direct.h>
#include <string>

#ifndef MAX_PATH
    #define MAX_PATH  260
#endif
#else  // _WIN32

#include <limits.h>
#include <unistd.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#else
#include <stdio.h>
#endif

#ifndef PATH_MAX
    #define PATH_MAX  260
#endif
#endif  // _WIN32

#include "env_utils.hpp"

#ifdef _WIN32
// for Windows

static std::string UTF16toUTF8(const wchar_t* str) {
    int size = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
    std::string utf8str(size - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, str, -1, &utf8str[0], size - 1, NULL, NULL);
    return utf8str;
}

static std::wstring UTF8toUTF16(const char* str) {
    int size = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    std::wstring utf16str(size - 1, '\0');
    MultiByteToWideChar(CP_UTF8, 0, str, -1, &utf16str[0], size - 1);
    return utf16str;
}

std::string GetExecutablePath() {
    wchar_t filename[MAX_PATH + 1];
    filename[MAX_PATH] = 0;
    GetModuleFileNameW(NULL, filename, MAX_PATH);
    return UTF16toUTF8(filename);
}

void SetCwd(const std::string& path) {
    std::wstring wpath = UTF8toUTF16(path.c_str());
    _wchdir(wpath.c_str());
}

#else  // _WIN32

// for linux/unix systems

std::string GetExecutablePath() {
    char path[PATH_MAX + 1];
    path[PATH_MAX] = 0;
#ifdef __linux__
    const size_t LINKSIZE = 100;
    char link[LINKSIZE];
    snprintf(link, LINKSIZE, "/proc/%d/exe", getpid() );
    int path_size = readlink(link, path, PATH_MAX);
    if (path_size == -1)
        path_size = 0;
    path[path_size] = 0;
#else
    uint32_t bufsize = PATH_MAX;
    _NSGetExecutablePath(path, &bufsize);
#endif
    if (path[0] == 0)
        return "/";
    return path;
}

void SetCwd(const std::string& path) {
    chdir(path.c_str());
}

#endif  // _WIN32

std::string GetDirectory(const std::string& path) {
    size_t pos = path.find_last_of("/\\");
    return (std::string::npos == pos)
        ? ""
        : path.substr(0, pos);
}
