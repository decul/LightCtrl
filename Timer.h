#pragma once 
#include <Arduino.h>
#include "limits.h"

class Timer {
private:
    long dueTime = LONG_MAX;
    long startTime = -1;

    virtual long getTime();

public:
    long period;

    Timer() { 

    }

    // Create and start timer
    Timer(long time) {
        Start(time);
    }

    // Start timer beginning from now and set period from parameter
    void Start(long time) {
        startTime = getTime();
        period = time;
        dueTime = startTime + time;
    }

    // Start timer beginning from now
    void Restart() {
        startTime = getTime();
        dueTime = startTime + period;
    }

    /// Start timer beginning from prevoius due time
    void Continue() {
        startTime = dueTime;
        dueTime += period;
    }

    /// Start timer beginning from prevoius due time and set period from parameter
    void AddTime(long time) {
        startTime = dueTime;
        period = time;
        dueTime += time;
    }

    // Returns value indicating if timer has expired
    bool HasExpired() {
        long now = getTime();

        if (now < startTime)
            dueTime = 0;

        return now >= dueTime;
    }

    // Method waits for timer to expire and then returns
    void WaitForExpiration() {
        long now;
        do {
            now = getTime();

            if (now < startTime)
                dueTime = 0;
        }
        while (now < dueTime);
    }
};

class MillisTimer : public Timer {
    inline long getTime() {
        return millis();
    }
};

class MicrosTimer : public Timer {
    inline long getTime() {
        return micros();
    }
};