#include "graphics_scene.h"

#include "basic_log.h"
#include "graphics_printer.h"

namespace FX {

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

    void GraphicsScene::draw()
    {
        for (auto&& itr : m_pEntityManager->m_container)
        {
            auto& group = itr.second;

            for (unsigned int i = 0; i < group.size(); i++)
            {
                auto& list = group[i];

                list.arrange();

                if (list.isDirty())
                {
                    list.clean();
                    list.generate();
                    list.setReady();
                }
            }
        }
    }

} // namespace FX
