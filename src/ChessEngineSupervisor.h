#pragma once
#include "ChessEngineOptions.h"
#include "ChessEngineTimer.h"

namespace ChessEngine {
using NodesCount = uint32_t;
constexpr NodesCount NEXT_CHECK_NODES_COUNT_OFFSET = 1000;

class CyclicPull {
 public:
    virtual void pull() {};
    virtual ~CyclicPull() = default;
};

class Supervisor {
 public:
    constexpr Supervisor(const Options& options) :
        options(options),
        enabled(),
        stopped(),
        nodesCount(),
        nextCheckNodesCount(),
        timer(),
        time(),
        nextPullTime(),
        cyclicPullPeriod(),
        defaultCyclicPull(),
        cyclicPull(&defaultCyclicPull) {}

    void begin() {
        enabled = false;
        stopped = false;
        timer.begin();
        time = 0;
        nodesCount = 0;
        updateNextCheckNodesCount();
        nextPullTime = cyclicPullPeriod;
    }

    void enable() { enabled = true; }
    void stop() { stopped = true; }
    bool isStopped() const { return stopped; }

    void incrementNodesCount() { nodesCount++; }
    NodesCount getNodesCount() const { return nodesCount; }

    void updateTime() { time = timer.value(); }
    Milliseconds getTime() const { return time; }

    void setCyclicPull(CyclicPull& pull, Milliseconds period) {
        cyclicPull = &pull;
        cyclicPullPeriod = period;
    }

    void checkCyclicPull() {
        if (cyclicPull == &defaultCyclicPull || time < nextPullTime) {
            return;
        }
        nextPullTime += cyclicPullPeriod;
        cyclicPull->pull();
    }

    bool checkTimeLimit() {
        if (enabled && time > options.timeLimit) {
            stopped = true;
        } else {
            checkCyclicPull();
        }
        return stopped;
    }

    bool tryCheckTimeLimit() {
        if (stopped) {
            return stopped;
        }
        if (!isTimeLimitCheckNeeded()) {
            return stopped;
        }
        updateNextCheckNodesCount();
        updateTime();
        return checkTimeLimit();
    }

 private:
    bool isTimeLimitCheckNeeded() const { return nodesCount >= nextCheckNodesCount; }
    void updateNextCheckNodesCount() { nextCheckNodesCount = nodesCount + NEXT_CHECK_NODES_COUNT_OFFSET; }

    const Options& options;

    bool enabled;
    bool stopped;
    NodesCount nodesCount;
    NodesCount nextCheckNodesCount;
    Timer timer;
    Milliseconds time;

    Milliseconds nextPullTime;
    Milliseconds cyclicPullPeriod;
    CyclicPull defaultCyclicPull;
    CyclicPull* cyclicPull;
};
}
