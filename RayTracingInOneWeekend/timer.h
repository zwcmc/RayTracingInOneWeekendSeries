#ifndef TIMER_H
#define TIMER_H

#include <chrono>

class timer
{
public:
    timer() = default;

    void start(const std::string& timer_text)
    {
        text = timer_text;
        begin = std::chrono::high_resolution_clock::now();
    }

    void stop()
    {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = end - begin;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        std::clog << "\r" << text << " costs: " << ms << " ms.\n";
    }

private:
    std::string text;
    std::chrono::high_resolution_clock::time_point begin;
};

#endif