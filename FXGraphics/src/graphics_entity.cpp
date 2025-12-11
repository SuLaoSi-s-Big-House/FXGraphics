#include "graphics_entity.h"

#include <assert.h>
#include "graphics_entity_manager.h"

namespace FX {

    GraphicsEntity::~GraphicsEntity()
    {
        if (!m_managerList.empty())
        {
            std::unordered_map<GraphicsEntityManager*, GroupPos> managerList = m_managerList;
            for (auto&& pair : managerList)
            {
                pair.first->removeEntity(this);
            }
        }
    }

    const float* GraphicsEntity::vertex() const
    {
        return m_vertex.data();
    }

    const float* GraphicsEntity::normal() const
    {
        return m_normal.data();
    }

    const float* GraphicsEntity::uv() const
    {
        return m_uv.data();
    }

    unsigned int GraphicsEntity::pointNum() const
    {
        return static_cast<unsigned int>(m_vertex.size() / 3);
    }

    const unsigned int* GraphicsEntity::index() const
    {
        return m_index.data();
    }

    unsigned int GraphicsEntity::indexNum() const
    {
        return static_cast<unsigned int>(m_index.size());
    }

    void* GraphicsEntity::owner() const
    {
        return nullptr;
    }

    EntityType GraphicsEntity::type() const
    {
        return m_type;
    }

    void GraphicsEntity::setDirty(DirtyType type)
    {
        if (type & DataDirty)
        {
            m_dataDirty = true;
        }

        for (auto&& pair : m_managerList)
        {
            pair.first->dirtyEntity(this, type);
        }
    }

    bool GraphicsEntity::registerType(EntityType type, PrimitiveMode mode)
    {
        if (s_entityTypeMap.count(type) > 0)
        {
            return false;
        }

        s_entityTypeMap.insert({ type, mode });
        return true;
    }

    const std::unordered_map<EntityType, PrimitiveMode>& GraphicsEntity::entityTypeMap()
    {
        return s_entityTypeMap;
    }

    std::unordered_map<EntityType, PrimitiveMode> GraphicsEntity::s_entityTypeMap = {
        {NormalLineID, PrimitiveMode::kLines},
        {NormalFaceID, PrimitiveMode::kTriangles},
        {NormalLineStripID, PrimitiveMode::kLineStrip},
        {NormalFaceStripID, PrimitiveMode::kTriangleStrip},
        {NormalPointID, PrimitiveMode::kPoints}
    };

    bool GraphicsEntity::isDataDirty() const
    {
        return m_dataDirty;
    }

    void GraphicsEntity::setDataReady()
    {
        m_dataDirty = false;
    }

    GroupPos GraphicsEntity::groupPos(GraphicsEntityManager* pManager) const
    {
        auto itr = m_managerList.find(pManager);
        return itr == m_managerList.end() ? GroupPos() : itr->second;
    }

    void GraphicsEntity::setGroupPos(GraphicsEntityManager* pManager, GroupPos pos)
    {
        assert(pos.valid());
        m_managerList[pManager] = pos;
    }

    void GraphicsEntity::eraseGroup(GraphicsEntityManager* pManager)
    {
        m_managerList.erase(pManager);
    }

} // namespace FX
