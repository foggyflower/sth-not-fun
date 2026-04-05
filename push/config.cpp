/*******************************************************
 * File : config.cpp
 * Desc : 配置文件处理 (修复版)
 ******************************************************/
#include "config.h"
#include "utils.h"
#include <fstream>
#include <sstream>
#include <windows.h>
#include <iomanip>

namespace fs = std::filesystem;

Config::Config() : checkInterval(10) {
    processes = {
        {"PlantsVsZombies.exe", 1800, 3600, 0, false, {}},
        {"notepad.exe", 300, 600, 0, false, {}}
    };
    standbyPeriods = {
        {22, 30, 6, 0, 1} // 22:30-06:00 每天
    };
}

static bool ParseTime(const std::string& timeStr, int& hour, int& minute) {
    size_t colonPos = timeStr.find(':');
    try {
        if (colonPos != std::string::npos) {
            hour = std::stoi(timeStr.substr(0, colonPos));
            minute = std::stoi(timeStr.substr(colonPos + 1));
            return true;
        } else {
            hour = std::stoi(timeStr);
            minute = 0;
            return true;
        }
    } catch (...) {
        return false;
    }
}

void Config::Load() {
    const fs::path configPath = "process_config.txt";
    if (!fs::exists(configPath)) {
        Utils::Log(u8"创建默认配置文件");
        Save();
        return;
    }

    std::ifstream ifs(configPath);
    if (!ifs.is_open()) {
        Utils::Log(u8"无法打开配置文件，使用默认设置");
        return;
    }

    processes.clear();
    standbyPeriods.clear();

    std::string line;
    while (std::getline(ifs, line)) {
        std::istringstream iss(line);
        std::string key;
        if (!(iss >> key)) continue;
        if (key.empty() || key[0] == '#') continue;

        if (key == u8"进程名" || key == "process_name") {
            ProcessInfo p;
            if (iss >> p.name >> p.workdayLimit >> p.weekendLimit) {
                p.accumulated = 0;
                p.isRunning = false;
                p.startTime = {};
                processes.push_back(p);
            }
            continue;
        }

        if (key == u8"待机时段" || key == "standby_period") {
            StandbyPeriod period;
            std::string startStr, endStr, daysStr;
            if (iss >> startStr >> endStr >> daysStr) {
                if (!ParseTime(startStr, period.startH, period.startM)) continue;
                if (!ParseTime(endStr, period.endH, period.endM)) continue;

                if (daysStr == u8"每天" || daysStr == "daily") period.daysMask = 1;
                else if (daysStr == u8"工作日" || daysStr == "weekday") period.daysMask = 2;
                else if (daysStr == u8"周末" || daysStr == "weekend") period.daysMask = 3;
                else continue;

                standbyPeriods.push_back(period);
            }
            continue;
        }

        if (key == u8"检查间隔" || key == "check_interval") {
            if (iss >> checkInterval) {
                if (checkInterval < 1) checkInterval = 10;
            }
            continue;
        }
    }
    Utils::Log(u8"配置加载完成");
}

void Config::Save() {
    std::ofstream ofs("process_config.txt");
    if (!ofs.is_open()) return;

    ofs << u8"# 进程监控配置文件\n";
    for (const auto& p : processes) {
        ofs << u8"进程名 " << p.name << " " << p.workdayLimit << " " << p.weekendLimit << "\n";
    }
    
    ofs << u8"\n# 待机时段\n";
    for (const auto& p : standbyPeriods) {
        ofs << u8"待机时段 " 
            << std::setw(2) << std::setfill('0') << p.startH << ":" 
            << std::setw(2) << std::setfill('0') << p.startM << " " 
            << std::setw(2) << std::setfill('0') << p.endH << ":" 
            << std::setw(2) << std::setfill('0') << p.endM << " ";
        if (p.daysMask == 1) ofs << u8"每天";
        else if (p.daysMask == 2) ofs << u8"工作日";
        else if (p.daysMask == 3) ofs << u8"周末";
        ofs << "\n";
    }
    
    ofs << u8"\n检查间隔 " << checkInterval << "\n";
}

void Config::ResetDaily() {
    for (auto& p : processes) {
        p.accumulated = 0;
        // 重置时建议清除运行状态，防止状态残留
        p.isRunning = false; 
    }
    Utils::Log(u8"已重置每日累计时间");
}

void Config::SaveRuntime() {
    std::ofstream ofs("process_runtime.txt");
    auto now = std::chrono::steady_clock::now();
    for (const auto& p : processes) {
        int total = p.accumulated;
        if (p.isRunning) {
            total += std::chrono::duration_cast<std::chrono::seconds>(now - p.startTime).count();
        }
        ofs << p.name << " " << total << "\n";
    }
    // 备份逻辑略 (保持原样即可)
}

void Config::InitProcesses() {
    for (auto& p : processes) {
        p.accumulated = 0;
        p.isRunning = false;
        p.startTime = {};
    }
}
