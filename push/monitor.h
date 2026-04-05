#ifndef MONITOR_H
#define MONITOR_H

#include "config.h"
#include <string>
#include <vector>
#include <windows.h>

// 移除了 using namespace std;

class ProcessMonitor {
public:
    static void Init(const std::vector<ProcessInfo>& processes);
    static void Check(Config& cfg);
private:
    static void UpdateProcessInfo(ProcessInfo& p);
};

#endif
