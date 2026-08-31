#ifndef _BASIC_LOG_H_
#define _BASIC_LOG_H_

#include <iostream>
#include <mutex>

namespace FX {

    class BasicLog {
    protected:
        BasicLog(void) = default;
        ~BasicLog(void) = default;

    public:
        static constexpr bool EnableLog = true;
        static constexpr bool PrintTime = true;

        enum LogType : unsigned char {
            kInfo = 0,
            kWarn,
            kError
        };

        // 可能从多个线程并发调用（如GraphicsScene的并行generate），一条日志的输出需要互斥，
        // 否则时间戳与消息体会交错。锁实体藏在basic_log.cpp的函数内静态中，确保跨DLL唯一
        template<typename... Messages>
        static void out(LogType type, Messages... messages)
        {
            if (EnableLog)
            {
                std::lock_guard<std::mutex> lock(mutex());
                describe(type);
                print(messages...);
            }
        }

    protected:
        static void describe(LogType type);

        template<typename Message, typename... Messages>
        static void print(Message message, Messages... messages)
        {
            std::cout << message;
            print(messages...);
        }

        static void print(void)
        {
            std::cout << std::endl;
        }

    private:
        static std::mutex& mutex(void);
    };

} // namespace FX

#endif // _BASIC_LOG_H_
