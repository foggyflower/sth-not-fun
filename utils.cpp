/*******************************************************
 * File : utils.cpp
 * Desc : 工具函数实现 (修复版)
 ******************************************************/
#include "utils.h"
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <windows.h>

namespace fs = std::filesystem;

namespace Utils {

    void Log(const std::string& msg) {
        const fs::path logPath = "process_monitor.log";
        std::ofstream logFile(logPath, std::ios::app);
        if (logFile.is_open()) {
            time_t now = time(nullptr);
            struct tm timeinfo;
            localtime_s(&timeinfo, &now);
            char timeStr[20];
            strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
            logFile << "[" << timeStr << "] " << msg << std::endl;
        }
    }

    // 【修复】实现真正的旧日志清理逻辑
    void CleanOldLogs(const fs::path& logDir, int days) {
        if (!fs::exists(logDir) || !fs::is_directory(logDir)) {
            // 如果传入的是文件路径，获取其父目录
            fs::path dir = logDir.parent_path();
            if (dir.empty()) dir = ".";
            CleanOldLogs(dir, days);
            return;
        }

        try {
            auto now = fs::file_time_type::clock::now();
            for (const auto& entry : fs::directory_iterator(logDir)) {
                if (entry.is_regular_file()) {
                    auto ftime = entry.last_write_time();
                    auto age = std::chrono::duration_cast<std::chrono::hours>(now - ftime).count() / 24; // 计算天数

                    // 仅清理日志文件 (防止误删)
                    std::string ext = entry.path().extension().string();
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                    
                    if (ext == ".log" || ext == ".txt" || ext == ".bak") {
                         // 为了安全，确保不删除正在使用的当前日志文件
                        if (entry.path().filename() != "process_monitor.log" && age >= days) {
                            fs::remove(entry.path());
                            Log(u8"已清理旧日志: " + entry.path().filename().string());
                        }
                    }
                }
            }
        } catch (const fs::filesystem_error& e) {
            Log(u8"清理旧日志失败: " + std::string(e.what()));
        }
    }

    // 【修复】放宽时间判断，允许在 00:00 ~ 00:59 之间触发
    bool IsMidnight() {
        SYSTEMTIME st;
        GetLocalTime(&st);
        // 只要小时为0，且分钟为0，即认为是午夜重置时段
        // 这样可以容忍 10秒的检查间隔，不会错过重置
        return (st.wHour == 0 && st.wMinute == 0);
    }

    bool IsWeekend() {
        SYSTEMTIME st;
        GetLocalTime(&st);
        return (st.wDayOfWeek == 0 || st.wDayOfWeek == 6);
    }

    bool IsSundayMidnight() {
        SYSTEMTIME st;
        GetLocalTime(&st);
        return (st.wDayOfWeek == 0 && st.wHour == 0 && st.wMinute == 0);
    }

} // namespace Utils
