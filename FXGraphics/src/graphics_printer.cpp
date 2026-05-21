#include "graphics_printer.h"

#include <assert.h>
#include "glad.h"
#include "basic_log.h"
#include "graphics_window.h"
#include "graphics_scene.h"

namespace FX {

    GraphicsPrinter::~GraphicsPrinter()
    {
        if (!m_sceneList.empty())
        {
            std::unordered_map<GraphicsScene*, unsigned int> sceneList = m_sceneList;
            for (auto&& itr : sceneList)
            {
                itr.first->removePrinter(this);
            }
        }
    }

    bool GraphicsPrinter::isReady() const
    {
        return m_ready;
    }

    void GraphicsPrinter::setCompatible(bool compatible)
    {
        if (m_compatible != compatible)
        {
            m_compatible = compatible;
            m_drawFunc = compatible ? &GraphicsPrinter::_drawLegacy : &GraphicsPrinter::_drawAdvance;
        }
    }

    void GraphicsPrinter::draw(PrimitiveMode mode, unsigned int num) const
    {
        assert(m_drawFunc != nullptr);
        (this->*m_drawFunc)(mode, num);
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

    void GraphicsPrinter::_drawAdvance(PrimitiveMode mode, unsigned int num) const
    {
        glMultiDrawElementsIndirect((GLenum)mode, GL_UNSIGNED_INT, 0, num, 0);
    }

    void GraphicsPrinter::_drawLegacy(PrimitiveMode mode, unsigned int num) const
    {
        struct DrawElementsCommand {
            unsigned int indexNum = 0;
            unsigned int instanceNum = 1;
            unsigned int indexStart = 0;
            int vertexOffset = 0;
            unsigned int instanceOffset = 0;
        };

#if defined(_DEBUG) || defined(DEBUG)
        int dibo = 0;
        glGetIntegerv(GL_DRAW_INDIRECT_BUFFER_BINDING, &dibo);
        assert(dibo != 0);
        int size = 0;
        glGetBufferParameteriv(GL_DRAW_INDIRECT_BUFFER, GL_BUFFER_SIZE, &size);
        assert(size >= num * sizeof(DrawElementsCommand));
#endif

        auto pData = new DrawElementsCommand[num];
        glGetBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0, num * sizeof(DrawElementsCommand), pData);
        for (unsigned int i = 0; i < num; i++)
        {
            glDrawElements((GLenum)mode, pData[i].indexNum, GL_UNSIGNED_INT, (void*)(pData[i].indexStart * sizeof(unsigned int)));
        }
        delete[] pData;
    }

    void GraphicsPrinter::addScene(GraphicsScene* pScene)
    {
        assert(pScene);

        auto itr = m_sceneList.find(pScene);
        if (itr == m_sceneList.end())
        {
            m_sceneList.insert({ pScene, 1 });
        }
        else
        {
            itr->second++;
        }
    }

    void GraphicsPrinter::eraseScene(GraphicsScene* pScene)
    {
        assert(pScene);

        auto itr = m_sceneList.find(pScene);
        assert(itr != m_sceneList.end());

        if (itr->second == 1)
        {
            m_sceneList.erase(itr);
        }
        else
        {
            itr->second--;
        }
    }

    GraphicsNormalPrinter::GraphicsNormalPrinter()
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
