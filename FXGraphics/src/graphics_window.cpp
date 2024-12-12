#include "graphics_window.h"

#include <assert.h>
#include "basic_log.h"

namespace FX {

    GraphicsWindow::GraphicsWindow(unsigned short width, unsigned short height, const std::string& title, bool isMultiSample)
        : m_windowSize({ width, height }), m_bufferSize({ width, height }), m_title(title), m_isMultiSample(isMultiSample),
          m_creationTime(std::chrono::steady_clock::now())
    {
        // init glfw
        if (glfwInit() == false)
        {
            BasicLog::out(BasicLog::kError, "CANNOT INIT GLFW!");
            glfwTerminate();
            return;
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        m_isMultiSample ? glfwWindowHint(GLFW_SAMPLES, 4) : glfwWindowHint(GLFW_SAMPLES, 0);
#ifdef FX_APPLE
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        // create window
        m_pWindowHandle = glfwCreateWindow(m_windowSize.x, m_windowSize.y, m_title.c_str(), 0, 0);
        if (m_pWindowHandle == nullptr)
        {
            BasicLog::out(BasicLog::kError, "CANNOT CREATE WINDOW [", m_title, "]!");
        }

        glfwMakeContextCurrent(m_pWindowHandle);
        s_pCurrentWindow = this;
        s_windowMap.insert({ m_pWindowHandle, this });

        glfwSwapInterval(1);

        // init glad
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            BasicLog::out(BasicLog::kError, "CANNOT INIT OPENGL!");
        }

        m_isMultiSample ? glEnable(GL_MULTISAMPLE) : glDisable(GL_MULTISAMPLE);
        glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
    }

    GraphicsWindow::~GraphicsWindow()
    {
        s_windowMap.erase(m_pWindowHandle);
        glfwDestroyWindow(m_pWindowHandle);

        auto pCurrent = glfwGetCurrentContext();
        if (pCurrent != nullptr)
        {
            auto itr = s_windowMap.find(pCurrent);
            s_pCurrentWindow = itr != s_windowMap.end() ? itr->second : nullptr;
        }
        else
        {
            s_pCurrentWindow = nullptr;
            glfwMakeContextCurrent(0);
        }

        if (s_windowMap.empty())
        {
            glfwTerminate();
        }
    }

    void GraphicsWindow::use()
    {
        if (s_pCurrentWindow != this)
        {
            glfwMakeContextCurrent(m_pWindowHandle);
            m_isMultiSample ? glEnable(GL_MULTISAMPLE) : glDisable(GL_MULTISAMPLE);
            s_pCurrentWindow = this;
        }
    }

    void GraphicsWindow::frame()
    {
        if (s_pCurrentWindow != this)
        {
            BasicLog::out(BasicLog::kWarn, "Frame a window that is not currently used.");
        }

        glfwSwapBuffers(m_pWindowHandle);
        glfwPollEvents();
    }

    bool GraphicsWindow::shouldClose() const
    {
        return glfwWindowShouldClose(m_pWindowHandle);
    }

    GraphicsWindow* GraphicsWindow::currentWindow()
    {
        return s_pCurrentWindow;
    }

    GraphicsWindow* GraphicsWindow::s_pCurrentWindow = nullptr;
    GraphicsWindow::WindowMap GraphicsWindow::s_windowMap;

} // namespace FX
