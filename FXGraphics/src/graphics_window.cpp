#include "graphics_window.h"

#include <assert.h>
#include "graphics_window_impl.h"

namespace FX {

    GraphicsWindow::GraphicsWindow(unsigned short width, unsigned short height, const std::string& title, bool isMultiSample)
    {
        m_pImpl = new GraphicsWindowImpl(this, width, height, title, isMultiSample);
    }

    GraphicsWindow::~GraphicsWindow()
    {
        assert(m_pImpl != nullptr);
        delete m_pImpl;
    }

    void GraphicsWindow::use()
    {
        m_pImpl->use();
    }

    void GraphicsWindow::frame()
    {
        m_pImpl->frame();
    }

    vec2us GraphicsWindow::size() const
    {
        return m_pImpl->size();
    }

    bool GraphicsWindow::shouldClose() const
    {
        return m_pImpl->shouldClose();
    }

    const GraphicsInteractor& GraphicsWindow::interactor() const
    {
        return m_pImpl->interactor();
    }

    const std::string& GraphicsWindow::vendor() const
    {
        return m_pImpl->vendor();
    }

    GraphicsWindow* GraphicsWindow::currentWindow()
    {
        if (GraphicsWindowImpl::currentWindow())
        {
            return GraphicsWindowImpl::currentWindow()->m_pApi;
        }
        return nullptr;
    }

} // namespace FX
