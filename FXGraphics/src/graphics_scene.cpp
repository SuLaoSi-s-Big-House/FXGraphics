#include "graphics_scene.h"

#include <assert.h>
#include "glm.hpp"
#include "glad.h"
#include "basic_log.h"
#include "graphics_window.h"
#include "graphics_printer.h"
#include "graphics_font_manager.h"
#include "graphics_camera.h"

namespace FX {

    namespace {
        struct NormalGlobalInfo {
            glm::mat4 vMatrix = glm::mat4(1.0f);
            glm::mat4 pMatrix = glm::mat4(1.0f);
            glm::mat4 vpMatrix = glm::mat4(1.0f);
            vec2i viewport = { 0, 0 };
        };
    }  // namespace

    GraphicsScene::GraphicsScene()
    {
        if (m_pEntityManager == nullptr)
        {
            m_pEntityManager = new GraphicsEntityManager(this);
        }
        if (m_pBufferManager == nullptr)
        {
            m_pBufferManager = new GraphicsBufferManager(this);
        }
    }

    GraphicsScene::~GraphicsScene()
    {
        if (m_pEntityManager != nullptr)
        {
            delete m_pEntityManager;
        }
        if (m_pBufferManager != nullptr)
        {
            delete m_pBufferManager;
        }
        for (auto&& pair : m_printerManager)
        {
            if (pair.second != nullptr)
            {
                pair.second->eraseScene(this);
            }
        }
        if (m_pCamera != nullptr)
        {
            m_pCamera->eraseScene(this);
        }
    }

    bool GraphicsScene::addEntity(GraphicsEntity* pEntity)
    {
        if (pEntity == nullptr)
        {
            BasicLog::out(BasicLog::kWarn, "Trying to add a null pointer as an entity, discard.");
            return false;
        }

        return m_pEntityManager->addEntity(pEntity);
    }

    bool GraphicsScene::removeEntity(GraphicsEntity* pEntity)
    {
        if (pEntity == nullptr)
        {
            BasicLog::out(BasicLog::kWarn, "Trying to remove a null pointer from entities, discard.");
            return false;
        }

        return m_pEntityManager->removeEntity(pEntity);
    }

    bool GraphicsScene::dirtyEntity(GraphicsEntity* pEntity, DirtyType type)
    {
        if (pEntity == nullptr)
        {
            BasicLog::out(BasicLog::kWarn, "Trying to set a null pointer entity dirty, discard.");
            return false;
        }

        return m_pEntityManager->dirtyEntity(pEntity, type);
    }

    bool GraphicsScene::addPrinter(GraphicsPrinter* pPrinter, EntityType type)
    {
        if (pPrinter == nullptr)
        {
            BasicLog::out(BasicLog::kWarn, "Trying to add a null pointer as a printer, discard.");
            return false;
        }

        auto itr = m_printerManager.find(type);
        if (itr != m_printerManager.end())
        {
            if (itr->second == pPrinter)
            {
                return false;
            }

            itr->second->eraseScene(this);
            pPrinter->addScene(this);
            itr->second = pPrinter;
        }
        else
        {
            m_printerManager.insert({ type, pPrinter });
            pPrinter->addScene(this);
        }

        return true;
    }

    bool GraphicsScene::removePrinter(GraphicsPrinter* pPrinter, EntityType type)
    {
        if (pPrinter == nullptr)
        {
            BasicLog::out(BasicLog::kWarn, "Trying to remove a null pointer from printers, discard.");
            return false;
        }

        auto itr = m_printerManager.find(type);
        if (itr != m_printerManager.end() && itr->second == pPrinter)
        {
            pPrinter->eraseScene(this);
            m_printerManager.erase(itr);
            return true;
        }

        return false;
    }

    bool GraphicsScene::removePrinter(GraphicsPrinter* pPrinter)
    {
        if (pPrinter == nullptr)
        {
            BasicLog::out(BasicLog::kWarn, "Trying to remove a null pointer from printers, discard.");
            return false;
        }

        bool success = false;
        for (auto itr = m_printerManager.begin(); itr != m_printerManager.end();)
        {
            if (itr->second == pPrinter)
            {
                pPrinter->eraseScene(this);
                itr = m_printerManager.erase(itr);
                success = true;
            }
            else
            {
                itr++;
            }
        }

        return success;
    }

    void GraphicsScene::bindCamera(GraphicsCamera* pCamera)
    {
        if (pCamera == nullptr)
        {
            BasicLog::out(BasicLog::kWarn, "Trying to bind a null pointer as a camera, discard.");
            return;
        }

        if (m_pCamera != nullptr)
        {
            m_pCamera->eraseScene(this);
        }
        m_pCamera = pCamera;
        pCamera->addScene(this);
    }

    void GraphicsScene::unbindCamera()
    {
        if (m_pCamera != nullptr)
        {
            m_pCamera->eraseScene(this);
            m_pCamera = nullptr;
        }
    }

    void GraphicsScene::draw()
    {
        assert(m_pEntityManager != nullptr);
        assert(m_pBufferManager != nullptr);

        generate();

        auto pWindow = GraphicsWindow::currentWindow();
        if (pWindow == nullptr)
        {
            assert(0);
            BasicLog::out(BasicLog::kWarn, "No window is used, cannot get draw a scene.");
            return;
        }

        if (pWindow->size() == vec2us{0, 0})
        {
            return;
        }

        beforeDraw();

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        auto& typeMap = GraphicsEntity::entityTypeMap();

        for (auto&& pair : m_pEntityManager->m_container)
        {
            auto type = pair.first;

            if (typeMap.count(type) == 0)
            {
                BasicLog::out(BasicLog::kWarn, "You should register type [", type, "] first before drawing.");
                continue;
            }

            auto mode = typeMap.find(type)->second;

            auto pPrinter = m_printerManager[type];

            if (pPrinter == nullptr || pPrinter->isReady() == false)
            {
                BasicLog::out(BasicLog::kWarn, "Cannot draw entities of type [", type, "] because printer is not ready.");
                continue;
            }

            pPrinter->use(NormalOpaquePipeline);

            for (int i = 0; i < pair.second.size(); i++)
            {
                auto& list = pair.second[i];

                if (list.entityList.empty())
                {
                    continue;
                }
                assert(list.isDirty() == false);

                if (list.pFont != nullptr)
                {
                    continue;    // 纹理文字只画透明
                }

                auto& buffers = m_pBufferManager->generate(list, type, i);

                assert(buffers.pDibos.size() == 2 && buffers.pDibos[0] != nullptr);
                auto pDibo = static_cast<DIBOInfo*>(buffers.pDibos[0]->getOrCreate());
                assert(pDibo != nullptr);

                if (pDibo->commandNum() == 0)
                {
                    continue;
                }

                pDibo->bind();

                auto pVao = static_cast<VAOInfo*>(buffers.pVao->getOrCreate());
                assert(pVao != nullptr);
                pVao->bind();
                auto pEbo = static_cast<EBOInfo*>(buffers.pEbo->getOrCreate());
                assert(pEbo != nullptr);
                pEbo->bind();
                auto pSsbo = static_cast<SSBOInfo*>(buffers.pSsbo->getOrCreate());
                assert(pSsbo != nullptr);
                pSsbo->bind(NormalProfileSlot);

                pPrinter->draw(mode, pDibo->commandNum());
            }
        }

        glEnable(GL_BLEND);
        glDepthMask(GL_FALSE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        for (auto&& pair : m_pEntityManager->m_container)
        {
            auto type = pair.first;

            if (typeMap.count(type) == 0)
            {
                continue;
            }

            auto mode = typeMap.find(type)->second;

            auto pPrinter = m_printerManager[type];

            if (pPrinter == nullptr || pPrinter->isReady() == false)
            {
                continue;
            }

            pPrinter->use(NormalTransPipeline);

            for (int i = 0; i < pair.second.size(); i++)
            {
                auto& list = pair.second[i];

                if (list.entityList.empty())
                {
                    continue;
                }
                assert(list.isDirty() == false);

                if (list.pFont != nullptr)
                {
                    auto font = GraphicsFontManager::instance().queryFont(*list.pFont);
                    assert(font.pTexture != nullptr);
                    auto pTexture = static_cast<TextureInfo*>(const_cast<GraphicsTexture*>(font.pTexture)->getOrCreate());
                    assert(pTexture != nullptr);
                    pTexture->bind(TextTextureUnit);
                }

                auto& buffers = m_pBufferManager->generate(list, type, i);

                assert(buffers.pDibos.size() == 2 && buffers.pDibos[1] != nullptr);
                auto pDibo = static_cast<DIBOInfo*>(buffers.pDibos[1]->getOrCreate());
                assert(pDibo != nullptr);

                if (pDibo->commandNum() == 0)
                {
                    continue;
                }

                pDibo->bind();

                auto pVao = static_cast<VAOInfo*>(buffers.pVao->getOrCreate());
                assert(pVao != nullptr);
                pVao->bind();
                auto pEbo = static_cast<EBOInfo*>(buffers.pEbo->getOrCreate());
                assert(pEbo != nullptr);
                pEbo->bind();
                auto pSsbo = static_cast<SSBOInfo*>(buffers.pSsbo->getOrCreate());
                assert(pSsbo != nullptr);
                pSsbo->bind(NormalProfileSlot);
                
                pPrinter->draw(mode, pDibo->commandNum());
            }
        }

        afterDraw();
    }

    void GraphicsScene::generate()
    {
        assert(m_pEntityManager != nullptr);
        assert(m_pBufferManager != nullptr);

        for (auto&& pair : m_pEntityManager->m_container)
        {
            auto type = pair.first;

            for (int i = 0; i < pair.second.size(); i++)
            {
                auto& list = pair.second[i];

                list.arrange();

                if (list.entityList.empty())
                {
                    continue;
                }

                if (list.isDirty())
                {
                    list.clean();
                    list.generate();

                    m_pBufferManager->accept(list, type, i);

                    list.setReady();
                }
            }
        }
    }

    void GraphicsScene::beforeDraw()
    {
        clear();
        bindGlobal();
    }

    void GraphicsScene::clear()
    {
        glDepthMask(GL_TRUE);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClearDepth(1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void GraphicsScene::bindGlobal()
    {
        NormalGlobalInfo info;

        if (m_pCamera == nullptr)
        {
            info.vMatrix = info.pMatrix = info.vpMatrix = GraphicsCamera::defaultVPMatrix();
        }
        else
        {
            info.vMatrix = m_pCamera->vMatrix();
            info.pMatrix = m_pCamera->pMatrix();
            info.vpMatrix = m_pCamera->vPMatrix();
        }

        auto pWindow = GraphicsWindow::currentWindow();
        assert(pWindow != nullptr);
        auto size = pWindow->size();
        info.viewport = { size.x, size.y };

        auto pUbo = static_cast<UBOInfo*>(m_globalUbo.getOrCreate());
        assert(pUbo != nullptr);
        pUbo->bind();
        pUbo->setData(sizeof(info), &info);
        pUbo->bind(0);

        glViewport(0, 0, size.x, size.y);
    }

    void GraphicsScene::afterDraw()
    {
        unbind();

#if defined(_DEBUG) || defined(DEBUG)
        assert(glGetError() == 0);
#endif
    }

    void GraphicsScene::unbind()
    {
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
    }

} // namespace FX
