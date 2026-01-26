#ifndef _GRAPHICS_WINDOW_H_
#define _GRAPHICS_WINDOW_H_

#include <string>

#include "basic_vector.h"
#include "basic_macro.h"
#include "graphics_interactor.h"

namespace FX {

    class GraphicsWindowImpl;

    class GraphicsWindow {
    public:
        GraphicsWindow(unsigned short width, unsigned short height, const std::string& title = "FXGraphics", bool isMultiSample = true);
        virtual ~GraphicsWindow(void);

        DELETE_COPY_AND_MOVE_CONSTRUCT(GraphicsWindow);

    public:
        void use(void);
        void frame(void);

        vec2us size(void) const;

        bool shouldClose(void) const;

        const GraphicsInteractor& interactor(void) const;

        static GraphicsWindow* currentWindow(void);

    protected:
        GraphicsWindowImpl* m_pImpl = nullptr;
    };

} // namespace FX

#endif // _GRAPHICS_WINDOW_H_
