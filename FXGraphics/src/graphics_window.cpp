#include "graphics_window.h"

#include <assert.h>
#include "basic_log.h"
#include "graphics_gpu_item.h"

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
        glfwSetWindowUserPointer(m_pWindowHandle, this);
        s_pCurrentWindow = this;
        s_windowCount++;

        glfwSetCursorPosCallback(m_pWindowHandle, &GraphicsWindow::mouseMoveCallBack);
        glfwSetScrollCallback(m_pWindowHandle, &GraphicsWindow::mouseScrollCallBack);

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
        use();

        for (auto pItem : m_itemList)
        {
            pItem->clearItem(this);
        }
        m_itemList.clear();

        glfwDestroyWindow(m_pWindowHandle);

        auto pCurrent = glfwGetCurrentContext();
        if (pCurrent != nullptr)
        {
            s_pCurrentWindow = static_cast<GraphicsWindow*>(glfwGetWindowUserPointer(pCurrent));
        }
        else
        {
            s_pCurrentWindow = nullptr;
            glfwMakeContextCurrent(0);
        }

        s_windowCount--;
        if (s_windowCount == 0)
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

        for (auto pItem : m_itemsToDelete)
        {
            delete pItem;
        }
        m_itemsToDelete.clear();
    }

    void GraphicsWindow::frame()
    {
        if (s_pCurrentWindow != this)
        {
            BasicLog::out(BasicLog::kWarn, "Frame a window that is not currently used.");
        }

        glfwSwapBuffers(m_pWindowHandle);
        m_interator.resetFlag();
        glfwPollEvents();
    }

    bool GraphicsWindow::shouldClose() const
    {
        return glfwWindowShouldClose(m_pWindowHandle);
    }

    const GraphicsInteractor& GraphicsWindow::interator() const
    {
        return m_interator;
    }

    GraphicsWindow* GraphicsWindow::currentWindow()
    {
        return s_pCurrentWindow;
    }

    void GraphicsWindow::addItem(GraphicsGPUItem* pItem)
    {
        assert(pItem != nullptr);
        m_itemList.insert(pItem);
    }

    void GraphicsWindow::removeItem(GraphicsGPUItem* pItem)
    {
        assert(pItem != nullptr);
        m_itemList.erase(pItem);
    }

    void GraphicsWindow::addToDelete(ItemInfo* pItem)
    {
        assert(pItem != nullptr);
        s_pCurrentWindow == this ? delete pItem : m_itemsToDelete.push_back(pItem);
    }

    void GraphicsWindow::mouseMoveCallBack(GLFWwindow* window, double xpos, double ypos)
    {
        assert(window != nullptr);
        auto pWindow = static_cast<GraphicsWindow*>(glfwGetWindowUserPointer(window));
        assert(pWindow != nullptr);
        pWindow->m_interator.setMousePos({ static_cast<float>(xpos), static_cast<float>(ypos) });
    }

    void GraphicsWindow::mouseScrollCallBack(GLFWwindow* window, double, double yoffset)
    {
        assert(window != nullptr);
        auto pWindow = static_cast<GraphicsWindow*>(glfwGetWindowUserPointer(window));
        assert(pWindow != nullptr);
        pWindow->m_interator.setMouseScroll(static_cast<float>(yoffset));
    }

    GraphicsWindow* GraphicsWindow::s_pCurrentWindow = nullptr;
    unsigned int GraphicsWindow::s_windowCount = 0;

} // namespace FX
