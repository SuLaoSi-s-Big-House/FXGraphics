#ifndef _GRAPHICS_INTERACTOR_H_
#define _GRAPHICS_INTERACTOR_H_

#include <array>
#include <chrono>

#include "basic_vector.h"

namespace FX {

    using EnventFlag = unsigned int;

    constexpr EnventFlag MouseMoveFlag = 1 << 0;
    constexpr EnventFlag MouseClickFlag = 1 << 1;
    constexpr EnventFlag MouseDragFlag = 1 << 2;
    constexpr EnventFlag MouseScrollFlag = 1 << 3;
    constexpr EnventFlag KeyboardFlag = 1 << 4;

    class GraphicsWindowImpl;

    class GraphicsInteractor {
    protected:
        friend class GraphicsWindowImpl;

        GraphicsInteractor(void) = default;
        ~GraphicsInteractor(void) = default;

    public:
        enum class MouseButton : unsigned char {
            kLeft = 0,
            kRight,
            kMiddle,
            kSide1,
            kSide2
        };

        bool isCursorIn(void) const;
        vec2d cursorPos(void) const;

        bool isButtonPressing(MouseButton button) const;

        EnventFlag enventFlag(void) const;

        double mouseScroll(void) const;

        void setCursorIn(bool isIn);
        void setCursorPos(double x, double y);
        void setMouseScroll(double scroll);

        void addButtonAction(MouseButton button, int action, int mode);

        void reset(void);

    protected:
        static constexpr unsigned char MouseButtonNum = 5;
        std::array<bool, MouseButtonNum> m_mouseStatus = {};

        static constexpr unsigned char MouseHistoryNum = 10;
        struct MouseHistory {
            vec2d position;
            std::chrono::time_point<std::chrono::high_resolution_clock> time;
            int action = 0;
            int mode = 0;
            MouseButton button = MouseButton::kLeft;
        };

        std::array<MouseHistory, MouseHistoryNum> m_mouseHistory;
        unsigned char m_start = 0;
        unsigned char m_length = 0;

        EnventFlag m_enventFlag = 0;

        // TODO 记录envent数据

        double m_mouseScroll = 0;

        vec2d m_cursorPos;
        bool m_isCursorIn = false;
    };

} // namespace FX

#endif // _GRAPHICS_INTERACTOR_H_
