#ifndef _GRAPHICS_INTERACTOR_H_
#define _GRAPHICS_INTERACTOR_H_

#include "basic_vector.h"

namespace FX {

    class GraphicsWindow;

    class GraphicsInteractor {
    public:
        friend class GraphicsWindow;

        enum InteractionFlag : unsigned int {
            kMouseMove = 1,
            kMouseButton = 2,
            kMouseScroll = 4,
        };

        unsigned int flag(void) const;
        
        const vec2f& mousePos(void) const;
        float mouseScroll(void) const;

    protected:
        GraphicsInteractor(void) = default;

        void resetFlag(void);

        void setMousePos(const vec2f& pos);
        void setMouseScroll(float scroll);

    protected:
        vec2f m_mousePos = { -1e6, -1e6 };
        float m_mouseScroll = 0;
        unsigned int m_flag = 0;
    };

} // namespace FX

#endif // _GRAPHICS_INTERACTOR_H_
