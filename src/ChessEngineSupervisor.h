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
    Supervisor(const Options& options) :
        options(options),
        stopped(),
        nodesCount(),
        nextCheckNodesCount(),
        timer(),
        time(),
        nextPullTime(),
        cyclicPullPeriod(),
        dafaultCyclicPull(),
        cyclicPull(&dafaultCyclicPull) {}

    void begin() {
        stopped = false;
        timer.begin();
        time = 0;
        nodesCount = 0;
        updateNextCheckNodesCount();
        nextPullTime = cyclicPullPeriod;
    }

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
        if (cyclicPull == &dafaultCyclicPull || time < nextPullTime) {
            return;
        }
        nextPullTime += cyclicPullPeriod;
        cyclicPull->pull();
    }

    bool checkTimeLimit() {
        if (time > options.timeLimit) {
            stopped = true;
            return stopped;
        }

        checkCyclicPull();
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

    bool stopped;
    NodesCount nodesCount;
    NodesCount nextCheckNodesCount;
    Timer timer;
    Milliseconds time;

    Milliseconds nextPullTime;
    Milliseconds cyclicPullPeriod;
    CyclicPull dafaultCyclicPull;
    CyclicPull* cyclicPull;
};
}
