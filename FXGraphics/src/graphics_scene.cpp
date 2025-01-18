#include "graphics_scene.h"

#include "basic_log.h"

namespace FX {

    GraphicsScene::GraphicsScene()
    {
        if (m_pEntityManager == nullptr)
        {
            m_pEntityManager = new GraphicsEntityManager(this);
        }
    }

    GraphicsScene::~GraphicsScene()
    {
        if (m_pEntityManager != nullptr)
        {
            delete m_pEntityManager;
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
