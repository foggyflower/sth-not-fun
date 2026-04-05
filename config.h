#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>
#include <chrono>
#include <filesystem>

// 移除了 using namespace std;
namespace fs = std::filesystem;

struct ProcessInfo {
    std::string name;
    int workdayLimit;
    int weekendLimit;
    int accumulated;
    bool isRunning;
    std::chrono::steady_clock::time_point startTime;
};

struct StandbyPeriod {
    int startH, startM;
    int endH, endM;
    int daysMask; // 1: 每天, 2: 工作日, 3: 周末
};

class Config {
public:
    Config();
    void Load();
    void Save();
    void ResetDaily();
    void SaveRuntime();
    void InitProcesses();

    // 成员变量添加 std:: 前缀
    std::vector<ProcessInfo> processes;
    std::vector<StandbyPeriod> standbyPeriods;
    int checkInterval;

private:
    // 私有成员变量已移除 (根据注释)
};

#endif
