#ifndef _GRAPHICS_INTERACTOR_H_
#define _GRAPHICS_INTERACTOR_H_

#include <array>
#include <chrono>

#include "basic_vector.h"

namespace FX {

    using EventFlag = unsigned int;

    constexpr EventFlag WindowResizeFlag = 1 << 0;
    constexpr EventFlag MouseMoveFlag = 1 << 1;
    constexpr EventFlag MouseClickFlag = 1 << 2;
    constexpr EventFlag MouseDragFlag = 1 << 3;
    constexpr EventFlag MouseScrollFlag = 1 << 4;

    enum class MouseButton : unsigned char {
        kLeft = 0,
        kRight,
        kMiddle,
        kSide1,
        kSide2
    };

    using ClockType = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<ClockType>;

    struct DragInfo {
        MouseButton button = MouseButton::kLeft;
        vec2d startPos;
        TimePoint startTime;
    };

    class GraphicsWindowImpl;

    class GraphicsInteractor {
    protected:
        friend class GraphicsWindowImpl;

        GraphicsInteractor(void) = default;
        ~GraphicsInteractor(void) = default;

    public:
        bool isCursorIn(void) const;
        vec2d cursorPos(void) const;

        bool isButtonPressing(MouseButton button) const;

        EventFlag enventFlag(void) const;

        const DragInfo& dragInfo(void) const;
        double mouseScroll(void) const;

        void setCursorIn(bool isIn);
        void setCursorPos(double x, double y);
        void setMouseScroll(double scroll);

        void addButtonAction(MouseButton button, int action, int mode);

        void addResizeFlag(void);

        void reset(void);

    protected:
        static constexpr unsigned char MouseButtonNum = 5;
        std::array<bool, MouseButtonNum> m_mouseStatus = {};

        static constexpr unsigned char MouseHistoryNum = 10;
        struct MouseHistory {
            MouseButton button = MouseButton::kLeft;
            vec2d position;
            TimePoint time;
            int action = 0;
            int mode = 0;
        };

        std::array<MouseHistory, MouseHistoryNum> m_mouseHistory;
        unsigned char m_start = 0;
        unsigned char m_length = 0;

        EventFlag m_enventFlag = 0;

        DragInfo m_dragInfo;
        double m_mouseScroll = 0;

        // TODO 解析事件
        // TODO 键盘

        vec2d m_cursorPos;
        bool m_isCursorIn = false;
    };

} // namespace FX

#endif // _GRAPHICS_INTERACTOR_H_
