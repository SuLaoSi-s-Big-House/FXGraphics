#include "graphics_window_impl.h"

#include <assert.h>
#include "basic_log.h"
#include "graphics_gpu_item.h"
#include "graphics_window.h"

namespace FX {

    namespace {

        inline void windowResizeCallback(GLFWwindow* window, int width, int height)
        {
            assert(window != nullptr);
            auto itr = GraphicsWindowImpl::windowMap().find(window);
            assert(itr->second != nullptr);
            itr->second->setSize(static_cast<unsigned short>(std::max(0, width)), static_cast<unsigned short>(std::max(0, height)));
        }

        inline void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
        {
            assert(window != nullptr);
            auto itr = GraphicsWindowImpl::windowMap().find(window);
            assert(itr->second != nullptr);
            itr->second->interactor().setCursorPos(xpos, ypos);
        }

        inline void cursorEnterCallback(GLFWwindow* window, int entered)
        {
            assert(window != nullptr);
            auto itr = GraphicsWindowImpl::windowMap().find(window);
            assert(itr->second != nullptr);
            itr->second->interactor().setCursorIn(entered != 0);
        }

        inline void mouseButtonCallback(GLFWwindow* window, int point, int action, int mods)
        {
            assert(window != nullptr);
            auto itr = GraphicsWindowImpl::windowMap().find(window);
            assert(itr->second != nullptr);

            GraphicsInteractor::MouseButton button = GraphicsInteractor::MouseButton::kLeft;
            switch (point)
            {
                case GLFW_MOUSE_BUTTON_LEFT: button = GraphicsInteractor::MouseButton::kLeft; break;
                case GLFW_MOUSE_BUTTON_RIGHT: button = GraphicsInteractor::MouseButton::kRight; break;
                case GLFW_MOUSE_BUTTON_MIDDLE: button = GraphicsInteractor::MouseButton::kMiddle; break;
                case GLFW_MOUSE_BUTTON_4: button = GraphicsInteractor::MouseButton::kSide1; break;
                case GLFW_MOUSE_BUTTON_5: button = GraphicsInteractor::MouseButton::kSide2; break;
                default: break;
            }
            itr->second->interactor().addButtonAction(button, action, mods);
        }

        inline void mouseScrollCallback(GLFWwindow* window, double, double yoffset)
        {
            assert(window != nullptr);
            auto itr = GraphicsWindowImpl::windowMap().find(window);
            assert(itr->second != nullptr);
            itr->second->interactor().setMouseScroll(yoffset);
        }

    }  // namespace

    GraphicsWindowImpl::GraphicsWindowImpl(GraphicsWindow* pApi, unsigned short width, unsigned short height, const std::string& title, bool isMultiSample)
        : m_pApi(pApi), m_windowSize({ width, height }), m_title(title), m_isMultiSample(isMultiSample)
    {
        // glfw初始化
        if (glfwInit() == false)
        {
            BasicLog::out(BasicLog::kError, "CANNOT INIT GLFW!");
            glfwTerminate();
            return;
        }

        // 创建430版本的OpenGL context
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // 默认为4x多重采样
        m_isMultiSample ? glfwWindowHint(GLFW_SAMPLES, 4) : glfwWindowHint(GLFW_SAMPLES, 0);

#ifdef FX_APPLE
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        // 创建glfw3窗口
        m_pWindowHandle = glfwCreateWindow(m_windowSize.x, m_windowSize.y, m_title.c_str(), 0, 0);
        if (m_pWindowHandle == nullptr)
        {
            BasicLog::out(BasicLog::kError, "CANNOT CREATE WINDOW [", m_title, "]!");
        }

        glfwMakeContextCurrent(m_pWindowHandle);
        s_pCurrentWindow = this;
        s_windowMap.insert({ m_pWindowHandle, this });

        // 垂直同步
        glfwSwapInterval(1);

        // 窗口事件的回调函数
        glfwSetWindowSizeCallback(m_pWindowHandle, windowResizeCallback);
        glfwSetCursorPosCallback(m_pWindowHandle, cursorPositionCallback);
        glfwSetCursorEnterCallback(m_pWindowHandle, cursorEnterCallback);
        glfwSetMouseButtonCallback(m_pWindowHandle, mouseButtonCallback);
        glfwSetScrollCallback(m_pWindowHandle, mouseScrollCallback);

        // 获取初始的鼠标位置
        double x = 0;
        double y = 0;
        glfwGetCursorPos(m_pWindowHandle, &x, &y);

        if (x > 0 && x < width && y > 0 && y < height)
        {
            m_interactor.setCursorIn(true);
            m_interactor.setCursorPos(x, y);
        }

        // 加载OpenGL函数
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            BasicLog::out(BasicLog::kError, "CANNOT INIT OPENGL!");
        }

        m_isMultiSample ? glEnable(GL_MULTISAMPLE) : glDisable(GL_MULTISAMPLE);
        glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);    // 使用OpenGL图元重启
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);    // 解除OpenGL texture 4字节对齐的限制
    }

    GraphicsWindowImpl::~GraphicsWindowImpl()
    {
        use();

        for (auto pItem : m_itemList)
        {
            pItem->clearItem(this);
        }
        m_itemList.clear();

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

    void GraphicsWindowImpl::use()
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

    void GraphicsWindowImpl::frame()
    {
        if (s_pCurrentWindow != this)
        {
            BasicLog::out(BasicLog::kWarn, "Frame a window that is not currently used.");
        }

        glfwSwapBuffers(m_pWindowHandle);

        m_interactor.reset();
        glfwPollEvents();
    }

    void GraphicsWindowImpl::setSize(unsigned short width, unsigned short height)
    {
        m_windowSize = { width, height };
    }

    vec2us GraphicsWindowImpl::size() const
    {
        return m_windowSize;
    }

    bool GraphicsWindowImpl::shouldClose() const
    {
        return glfwWindowShouldClose(m_pWindowHandle);
    }

    GraphicsInteractor& GraphicsWindowImpl::interactor()
    {
        return m_interactor;
    }

    void GraphicsWindowImpl::addItem(GraphicsGPUItem* pItem)
    {
        assert(pItem != nullptr);
        m_itemList.insert(pItem);
    }

    void GraphicsWindowImpl::removeItem(GraphicsGPUItem* pItem)
    {
        assert(pItem != nullptr);
        m_itemList.erase(pItem);
    }

    void GraphicsWindowImpl::addToDelete(ItemInfo* pItem)
    {
        assert(pItem != nullptr);
        s_pCurrentWindow == this ? delete pItem : m_itemsToDelete.push_back(pItem);
    }

    GraphicsWindowImpl* GraphicsWindowImpl::currentWindow()
    {
        return s_pCurrentWindow;
    }

    const GraphicsWindowImpl::WindowMap& GraphicsWindowImpl::windowMap()
    {
        return s_windowMap;
    }

    GraphicsWindowImpl* GraphicsWindowImpl::s_pCurrentWindow = nullptr;
    GraphicsWindowImpl::WindowMap GraphicsWindowImpl::s_windowMap;

} // namespace FX
