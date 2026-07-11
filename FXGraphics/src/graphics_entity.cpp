#include "graphics_entity.h"

#include <assert.h>
#include "graphics_entity_manager.h"

namespace FX {

    namespace {

        template<typename T>
        inline DirtyType compare(const T& left, const T& right, DirtyType type)
        {
            return (left == right) ? 0 : type;
        }

        template<typename T, typename... Args>
        inline DirtyType compare(const T& left, const T& right, DirtyType type, const Args&... args)
        {
            DirtyType ret = (left == right) ? 0 : type;
            return ret | compare(args...);
        }

        inline DirtyType compare(const Material& left, const Material& right)
        {
            return compare(left.baseColor, right.baseColor, MaterialDirty,
                left.baseColor.a, right.baseColor.a, TransparencyDirty | MaterialDirty,
                left.visible, right.visible, VisibleDirty | MaterialDirty,
                left.metallic, right.metallic, MaterialDirty,
                left.roughness, right.roughness, MaterialDirty,
                left.font, right.font, FontDirty);
        }

    }  // namespace

    bool GroupPos::valid() const
    {
        return first >= 0 && second >= 0;
    }

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

        if (m_materialHandle != DefaultMaterialHandle)
        {
            GraphicsMaterialManager::instance().unref(m_materialHandle);
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

    void GraphicsEntity::setMatrix(const glm::mat4& matrix)
    {
        if (m_matrix != matrix)
        {
            m_matrix = matrix;
            setDirty(MatrixDirty);
        }
    }

    const glm::mat4& GraphicsEntity::matrix()
    {
        return m_matrix;
    }

    void GraphicsEntity::setMaterial(const Material& material)
    {
        auto& oldMaterial = GraphicsMaterialManager::instance().get(m_materialHandle);
        auto dirtyType = compare(oldMaterial, material);

        if (dirtyType > 0)
        {
            GraphicsMaterialManager::instance().unref(m_materialHandle);
            m_materialHandle = GraphicsMaterialManager::instance().ref(material);

            setDirty(dirtyType);
        }
    }

    const Material& GraphicsEntity::material()
    {
        return GraphicsMaterialManager::instance().get(m_materialHandle);
    }

    MaterialHandle GraphicsEntity::materialHandle() const
    {
        return m_materialHandle;
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

        // 现在的置脏可能会导致删除加入，所以临时拷贝一份
        std::unordered_map<GraphicsEntityManager*, GroupPos> managerList = m_managerList;
        for (auto& pair : managerList)
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
        {NormalPointID, PrimitiveMode::kPoints},
        {ScreenTextID, PrimitiveMode::kTriangles}
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
