#ifndef STANDBY_H
#define STANDBY_H

#include <vector>
#include "config.h"

// 移除了 using namespace std;

class StandbyManager {
public:
    static bool ShouldTrigger(const std::vector<StandbyPeriod>& periods);
    static void DoStandby();
};

#endif
