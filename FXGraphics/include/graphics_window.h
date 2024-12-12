#ifndef _GRAPHICS_WINDOW_H_
#define _GRAPHICS_WINDOW_H_

#include <string>
#include <map>
#include <chrono>
#include "glad.h"
#include "glfw3.h"

#include "basic_vector.h"
#include "basic_macro.h"

namespace FX {

    class GraphicsWindow {
    public:
        GraphicsWindow(unsigned short width, unsigned short height, const std::string& title = "FXGraphics", bool isMultiSample = true);
        virtual ~GraphicsWindow(void);

        DELETE_COPY_AND_MOVE_CONSTRUCT(GraphicsWindow);

    public:
        void use(void);
        void frame(void);
        bool shouldClose(void) const;

        static GraphicsWindow* currentWindow(void);

    protected:
        GLFWwindow* m_pWindowHandle = nullptr;
        vec2us m_windowSize = { 1280, 720 };
        vec2us m_bufferSize = { 1280, 720 };
        std::string m_title;
        const std::chrono::steady_clock::time_point m_creationTime;
        bool m_isMultiSample = true;

        static GraphicsWindow* s_pCurrentWindow;
        using WindowMap = std::map<const GLFWwindow*, GraphicsWindow*>;
        static WindowMap s_windowMap;
    };

} // namespace FX

#endif // _GRAPHICS_WINDOW_H_
