#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <memory>
#include <windows.h>
#include <filesystem>

// ------------------ RAII 句柄管理 ------------------
struct HandleDeleter {
    void operator()(HANDLE handle) const {
        if (handle != INVALID_HANDLE_VALUE && handle != nullptr) {
            CloseHandle(handle);
        }
    }
};

using UniqueHandle = std::unique_ptr<std::remove_pointer<HANDLE>::type, HandleDeleter>;

// ------------------ 全局工具 ------------------
namespace Utils {
    void Log(const std::string& msg);
    bool IsWeekend();
    bool IsMidnight();
    bool IsSundayMidnight();
    
    // 【修复】将 CleanOldLogs 声明移入 Utils 命名空间
    void CleanOldLogs(const std::filesystem::path& logPath, int days = 7);
}

#endif
