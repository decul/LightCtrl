#pragma once 

class Timer {
private:
    long dueTime;
    long startTime;
    int period;

    virtual long getTime();

public:
    Timer() { 

    }

    Timer(long time) {
        Start(time);
    }

    void Start(int time) {
        startTime = getTime();
        period = time;
        dueTime = startTime + time;
    }

    bool HasExpired() {
        long now = getTime();

        if (now < startTime)
            dueTime = 0;

        return now >= dueTime;
    }

    void WaitForExpiration() {
        long now;
        do {
            now = getTime();

            if (now < startTime)
                dueTime = 0;
        }
        while (now < dueTime);
    }

    void Continue() {
        startTime = dueTime;
        dueTime += period;
    }

    void AddTime(int time) {
        startTime = dueTime;
        period = time;
        dueTime += time;
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