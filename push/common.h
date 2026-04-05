#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <memory>
#include <windows.h>

// ------------------ RAII 句柄管理 ------------------
struct HandleDeleter {
    void operator()(HANDLE handle) const {
        if (handle != INVALID_HANDLE_VALUE && handle != nullptr) {
            CloseHandle(handle);
        }
    }
};

using UniqueHandle = std::unique_ptr<std::remove_pointer<HANDLE>::type, HandleDeleter>;

// ------------------ 全局工具函数 ------------------
void Log(const std::string& msg);
bool IsWeekend();
bool IsMidnight();
bool IsSundayMidnight();

#endif // COMMON_H
