/*******************************************************
 * File : standby.cpp
 * Desc : 关机管理 (修复版)
 ******************************************************/
#include "standby.h"
#include "utils.h"
#include <windows.h>
#include <vector>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <thread>

// 辅助调试函数
static std::string GetDayMaskName(int mask) {
    if (mask == 1) return "Daily";
    if (mask == 2) return "Weekday";
    if (mask == 3) return "Weekend";
    return "Unknown";
}

bool StandbyManager::ShouldTrigger(const std::vector<StandbyPeriod>& periods) {
    if (periods.empty()) {
        return false;
    }

    SYSTEMTIME st;
    GetLocalTime(&st);
    int nowMins = st.wHour * 60 + st.wMinute;
    bool isWeekend = (st.wDayOfWeek == 0 || st.wDayOfWeek == 6);

    for (size_t i = 0; i < periods.size(); ++i) {
        const auto& p = periods[i];
        int startMins = p.startH * 60 + p.startM;
        int endMins = p.endH * 60 + p.endM;

        // 检查星期匹配
        bool dayMatch = false;
        if (p.daysMask == 1) dayMatch = true;
        else if (p.daysMask == 2 && !isWeekend) dayMatch = true;
        else if (p.daysMask == 3 && isWeekend) dayMatch = true;

        if (!dayMatch) continue;

        // 检查时间匹配
        bool timeMatch = false;
        if (startMins > endMins) {
            // 跨天情况 (如 22:00 - 06:00)
            if (nowMins >= startMins || nowMins < endMins) timeMatch = true;
        } else {
            // 同一天
            if (nowMins >= startMins && nowMins < endMins) timeMatch = true;
        }

        if (timeMatch) {
            Utils::Log(u8"匹配关机时段: [" + GetDayMaskName(p.daysMask) + u8"] " + 
                      std::to_string(p.startH) + ":" + std::to_string(p.startM));
            return true;
        }
    }
    return false;
}

void StandbyManager::DoStandby() {
    Utils::Log(u8"触发关机指令: 系统即将关闭...");

    // 1. 尝试获取关机权限
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
        tkp.PrivilegeCount = 1;
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
        CloseHandle(hToken);
    }

    // 2. 执行强制关机
    BOOL result = ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 0);
    if (!result) {
        Utils::Log(u8"关机失败: 可能缺少管理员权限 (Error: " + std::to_string(GetLastError()) + u8")");
    } else {
        Utils::Log(u8"关机信号已发送");
    }

    // 3. 防抖：防止失败后瞬间再次触发
    std::this_thread::sleep_for(std::chrono::seconds(60));
}
