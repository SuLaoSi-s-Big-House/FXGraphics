#include "graphics_interactor.h"

namespace FX {

    unsigned int GraphicsInteractor::flag() const
    {
        return m_flag;
    }

    const vec2f& GraphicsInteractor::mousePos() const
    {
        return m_mousePos;
    }

    float GraphicsInteractor::mouseScroll() const
    {
        return m_mouseScroll;
    }

    void GraphicsInteractor::resetFlag()
    {
        m_flag = 0;
    }

    void GraphicsInteractor::setMousePos(const vec2f& pos)
    {
        m_mousePos = pos;
        m_flag |= kMouseMove;
    }

    void GraphicsInteractor::setMouseScroll(float scroll)
    {
        m_mouseScroll = scroll;
        m_flag |= kMouseScroll;
    }

} // namespace FX
