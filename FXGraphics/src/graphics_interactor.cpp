#include "graphics_interactor.h"

#include <assert.h>
#include <cmath>
#include "glfw3.h"

namespace FX {

    bool GraphicsInteractor::isCursorIn() const
    {
        return m_isCursorIn;
    }

    vec2d GraphicsInteractor::cursorPos() const
    {
        return m_cursorPos;
    }

    bool GraphicsInteractor::isButtonPressing(MouseButton button) const
    {
        assert(static_cast<unsigned char>(button) < MouseButtonNum);
        return m_mouseStatus[static_cast<unsigned char>(button)];
    }

    EnventFlag GraphicsInteractor::enventFlag() const
    {
        return m_enventFlag;
    }

    double GraphicsInteractor::mouseScroll() const
    {
        return m_mouseScroll;
    }

    void GraphicsInteractor::setCursorIn(bool isIn)
    {
        m_isCursorIn = isIn;
    }

    void GraphicsInteractor::setCursorPos(double x, double y)
    {
        if (m_cursorPos.x != x || m_cursorPos.y != y)
        {
            m_cursorPos = { x, y };
            m_enventFlag |= MouseMoveFlag;
        }

        if (m_length > 0)
        {
            unsigned char last = (m_start + m_length - 1) % MouseHistoryNum;
            if (m_mouseHistory[last].action != GLFW_RELEASE)
            {
                auto dis = pow(m_mouseHistory[last].position.x - x, 2) + pow(m_mouseHistory[last].position.y - y, 2);
                if (dis > 25)    // 当鼠标位置变化超过5像素时认为是拖拽
                {
                    m_enventFlag |= MouseDragFlag;
                }
            }
        }
    }

    void GraphicsInteractor::setMouseScroll(double scroll)
    {
        m_mouseScroll = scroll;
        m_enventFlag |= MouseScrollFlag;
    }

    void GraphicsInteractor::addButtonAction(MouseButton button, int action, int mode)
    {
        assert(static_cast<unsigned char>(button) < MouseButtonNum);
        m_mouseStatus[static_cast<unsigned char>(button)] = (action != GLFW_RELEASE);

        if (m_isCursorIn == false)
        {
            return;
        }

        if (action == GLFW_REPEAT && m_length > 0)
        {
            unsigned char last = (m_start + m_length - 1) % MouseHistoryNum;
            if (m_mouseHistory[last].button == button && m_mouseHistory[last].action == GLFW_PRESS)
            {
                return;    // 重复同一个按键，不需要记录
            }
        }

        unsigned char end = (m_start + m_length) % MouseHistoryNum;
        m_mouseHistory[end].position = m_cursorPos;
        m_mouseHistory[end].time = std::chrono::high_resolution_clock::now();
        m_mouseHistory[end].action = action;
        m_mouseHistory[end].mode = mode;
        m_mouseHistory[end].button = button;

        if (m_length == MouseHistoryNum)
        {
            m_start = (m_start + 1) % MouseHistoryNum;
        }
        else
        {
            m_length++;
        }

        if (action == GLFW_RELEASE)    // 仅在按键松开时生成click事件
        {
            m_enventFlag |= MouseClickFlag;
            m_enventFlag &= (~MouseDragFlag);    // 按键松开时一定不再会有拖拽事件
        }
    }

    void GraphicsInteractor::reset()
    {
        m_enventFlag &= MouseDragFlag;    // 不需要每帧清除drag事件
    }

} // namespace FX
