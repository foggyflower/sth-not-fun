/*******************************************************
 * File : monitor.cpp
 * Desc : 进程监控实现 (修复版)
 ******************************************************/
#include "monitor.h"
#include "utils.h"
#include <windows.h>
#include <tlhelp32.h>
#include <algorithm>

// 不使用 using namespace std; 以保持风格统一

void ProcessMonitor::Init(const std::vector<ProcessInfo>& processes) {
    Utils::Log(u8"进程监控初始化完成");
}

void ProcessMonitor::Check(Config& cfg) {
    // 直接遍历并修改配置中的进程引用
    for (auto& p : cfg.processes) {
        UpdateProcessInfo(p);
    }
}

void ProcessMonitor::UpdateProcessInfo(ProcessInfo& p) {
    // 使用 RAII 包装句柄，确保函数退出时自动关闭
    HANDLE hRawSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hRawSnapshot == INVALID_HANDLE_VALUE) return;
    UniqueHandle hSnapshot(hRawSnapshot);

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);

    bool isFoundNow = false;

    if (Process32First(hSnapshot.get(), &pe)) {
        do {
            // 比较进程名 (不区分大小写)
            if (_stricmp(pe.szExeFile, p.name.c_str()) == 0) {
                isFoundNow = true;
                
                // 【核心修复】只有在进程从“停止”变为“运行”时，才记录启动时间
                if (!p.isRunning) {
                    p.startTime = std::chrono::steady_clock::now();
                    Utils::Log(u8"检测到进程启动: " + p.name);
                }
                break; // 找到目标进程，退出循环
            }
        } while (Process32Next(hSnapshot.get(), &pe));
    }

    // 更新运行状态
    p.isRunning = isFoundNow;

    // 【新增】如果进程已停止，这里可以选择重置 startTime 或保持原样
    // 由于累计时间是在保存时计算的，这里不需要额外操作
    if (!isFoundNow && p.isRunning) {
         Utils::Log(u8"检测到进程退出: " + p.name);
    }
}
