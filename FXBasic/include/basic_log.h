#ifndef _BASIC_LOG_H_
#define _BASIC_LOG_H_

#include <iostream>

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

        template<typename... Messages>
        static constexpr void out(LogType type, Messages... messages)
        {
            if (EnableLog)
            {
                describe(type);
                print(messages...);
            }
        }

    protected:
        static void describe(LogType type);

        template<typename Message, typename... Messages>
        static constexpr void print(Message message, Messages... messages)
        {
            std::cout << message;
            print(messages...);
        }

        static void print(void)
        {
            std::cout << std::endl;
        }
    };

} // namespace FX

#endif // _BASIC_LOG_H_
