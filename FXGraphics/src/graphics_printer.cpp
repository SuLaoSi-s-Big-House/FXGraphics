#include "graphics_printer.h"

#include <assert.h>
#include "glad.h"
#include "basic_log.h"
#include "graphics_window.h"

namespace FX {

    bool GraphicsPrinter::isReady() const
    {
        return m_ready;
    }

    void GraphicsPrinter::draw(unsigned int num) const
    {
        glMultiDrawElementsIndirect((GLenum)m_type, GL_UNSIGNED_INT, 0, num, 0);
    }

    void GraphicsPrinter::addProgram()
    {
        m_programs.emplace_back(new GraphicsProgram);
        _updateReady();
    }

    void GraphicsPrinter::addShader(GPUItemType type, const std::ifstream& file)
    {
        m_shaders.emplace_back(new GraphicsShader(type, file));
        _updateReady();
        _dirtyPrograms();
    }

    void GraphicsPrinter::addShader(GPUItemType type, const std::string& source)
    {
        m_shaders.emplace_back(new GraphicsShader(type, source));
        _updateReady();
        _dirtyPrograms();
    }

    void GraphicsPrinter::_use(unsigned int program, const std::vector<unsigned int>& shaders)
    {
        assert(program < m_programs.size() && m_programs[program] != nullptr);
        assert(GraphicsWindow::currentWindow() != nullptr);

        auto pProgram = static_cast<ProgramInfo*>(m_programs[program]->getOrCreate(true));

        for (auto i : shaders)
        {
            if (i >= m_shaders.size() || m_shaders[i] == nullptr)
            {
                assert(0);
                continue;
            }

            auto pShader = static_cast<ShaderInfo*>(m_shaders[i]->getOrCreate());
            if (pShader->isDirty())
            {
                pShader->compile();
                pShader->setDirty(false);
            }
            glAttachShader(pProgram->m_handle, pShader->m_handle);
        }

        pProgram->link();
        pProgram->use();
        pProgram->setDirty(false);
    }

    void GraphicsPrinter::_updateReady()
    {
        if (m_ready)
        {
            return;
        }

        if (m_programs.empty() || m_shaders.size() < 2)
        {
            return;
        }

        bool hasVertex = false;
        bool hasFragment = false;
        for (auto&& pShader : m_shaders)
        {
            assert(pShader);
            if (pShader != nullptr)
            {
                hasVertex |= pShader->type() == GPUItemType::kVtxShader;
                hasFragment |= pShader->type() == GPUItemType::kFrgShader;
            }
        }

        if (hasVertex && hasFragment)
        {
            m_ready = true;
        }
    }

    void GraphicsPrinter::_dirtyPrograms()
    {
        for (auto&& pProgram : m_programs)
        {
            assert(pProgram);
            if (pProgram != nullptr)
            {
                pProgram->setDirty(true);
            }
        }
    }

    GraphicsNormalPrinter::GraphicsNormalPrinter(PrintType type) : GraphicsPrinter(type)
    {
        m_programs.emplace_back(new GraphicsProgram);
    }

    void GraphicsNormalPrinter::use(PrintPipeline)
    {
        if (!m_ready)
        {
            BasicLog::out(BasicLog::kWarn, "Cannot use this printer, because there are not enough shaders available.");
            return;
        }

        assert(!m_programs.empty() && m_programs[0] != nullptr);

        auto pWindow = GraphicsWindow::currentWindow();
        if (pWindow == nullptr)
        {
            assert(pWindow);
            BasicLog::out(BasicLog::kWarn, "No window is used, cannot use a printer.");
            return;
        }

        auto pProgram = m_programs[0]->get();
        if (pProgram != nullptr && !pProgram->isDirty())
        {
            pProgram->use();
            return;
        }

        unsigned int size = static_cast<unsigned int>(m_shaders.size());
        std::vector<unsigned int> shaders(size);
        for (unsigned int i = 0; i < size; i++)
        {
            shaders[i] = i;
        }
        _use(0, shaders);
    }

} // namespace FX
