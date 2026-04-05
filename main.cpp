/*******************************************************
 * File : main.cpp
 * Desc : 主程序入口 (修复版)
 ******************************************************/
#include "config.h"
#include "monitor.h"
#include "standby.h"
#include "utils.h"
#include <iostream>
#include <thread>
#include <chrono>

namespace fs = std::filesystem;

int main() {
	// 隐藏控制台窗口
    HWND console = GetConsoleWindow();
    ShowWindow(console, SW_HIDE);
	
    // 初始化配置
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    Config cfg;
    cfg.Load();
    cfg.InitProcesses();

    // 初始化进程监控
    ProcessMonitor::Init(cfg.processes);

    Utils::Log(u8"程序启动成功，开始监控...");

    bool alreadyResetToday = false; // 防止在 00:00 分内多次重置

    // 主循环
    while (true) {
        // 1. 进程监控
        ProcessMonitor::Check(cfg);
        cfg.SaveRuntime();

        // 2. 时段限制检查
        if (StandbyManager::ShouldTrigger(cfg.standbyPeriods)) {
            StandbyManager::DoStandby();
        }

        // 3. 每日维护
        if (Utils::IsMidnight()) {
            if (!alreadyResetToday) {
                Utils::Log(u8"进入午夜维护时段，重置每日数据...");
                cfg.ResetDaily();
                // 清理当前目录下的旧日志 (保留7天)
                Utils::CleanOldLogs(fs::current_path(), 7);
                alreadyResetToday = true;
            }
        } else {
            // 只要时间过了 00:00，重置标记，允许第二天再次触发
            alreadyResetToday = false;
        }

        // 4. 休眠
        std::this_thread::sleep_for(std::chrono::seconds(cfg.checkInterval));
    }

    return 0;
}
