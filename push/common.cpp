#include "common.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <algorithm>

using namespace std;

const string LOG_FILE = "process_monitor.log";

void Log(const string& msg) {
    ofstream logFile(LOG_FILE, ios::app);
    if (logFile.is_open()) {
        auto now = chrono::system_clock::now();
        auto time = chrono::system_clock::to_time_t(now);
        struct tm timeinfo{};
        localtime_s(&timeinfo, &time);
        logFile << "[" << put_time(&timeinfo, "%c") << "] " << msg << endl;
    }
}

bool IsWeekend() {
    auto t = time(nullptr);
    struct tm timeinfo{};
    localtime_s(&timeinfo, &t);
    return (timeinfo.tm_wday == 0 || timeinfo.tm_wday == 6);
}

bool IsMidnight() {
    auto t = time(nullptr);
    struct tm timeinfo{};
    localtime_s(&timeinfo, &t);
    return timeinfo.tm_hour == 0 && timeinfo.tm_min == 0;
}

bool IsSundayMidnight() {
    auto t = time(nullptr);
    struct tm timeinfo{};
    localtime_s(&timeinfo, &t);
    return timeinfo.tm_wday == 0 && timeinfo.tm_hour == 0 && timeinfo.tm_min == 0;
}
