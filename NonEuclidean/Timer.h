#pragma once

#include <chrono>

class Timer
{
public:
    Timer() { Start(); }

    void Start() { t1 = std::chrono::system_clock::now(); }

    double GetSeconds()
    {
        auto now = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - t1);
        return static_cast<double>(elapsed.count()) / 1000000.0;
    }

private:
    std::chrono::time_point<std::chrono::system_clock> t1;
};
