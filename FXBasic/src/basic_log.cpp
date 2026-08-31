#include "basic_log.h"

#include <chrono>
#include <assert.h>

namespace FX {

    std::mutex& BasicLog::mutex()
    {
        static std::mutex instance;
        return instance;
    }

    void BasicLog::describe(LogType type)
    {
        if (PrintTime)
        {
            auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::tm stamp = { 0 };
            ::localtime_s(&stamp, &now);

            std::cout << "[";
            if (stamp.tm_hour < 10)
            {
                std::cout << "0";
            }
            std::cout << stamp.tm_hour;
            std::cout << ":";
            if (stamp.tm_min < 10)
            {
                std::cout << "0";
            }
            std::cout << stamp.tm_min;
            std::cout << ":";
            if (stamp.tm_sec < 10)
            {
                std::cout << "0";
            }
            std::cout << stamp.tm_sec;
            std::cout << "]";
        }

        switch (type)
        {
            case kInfo: std::cout << "[INFO] "; break;
            case kWarn: std::cout << "[WARN] "; break;
            case kError: std::cout << "[ERROR] "; break;
            default: assert(0); break;
        }
    }

} // namespace FX
