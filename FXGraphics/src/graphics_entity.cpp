#include "graphics_entity.h"

#include <assert.h>
#include "graphics_entity_manager.h"
#include "graphics_texture_manager.h"

namespace FX {

    namespace {
        inline DirtyType compare(const TextureKey& left, const TextureKey& right)
        {
            DirtyType ret = 0;
            auto& manager = GraphicsTextureManager::instance();

            for (unsigned int i = 0; i < TextureSlotNum; i++)
            {
                auto imageHandle1 = left.handle(i);
                auto imageHandle2 = right.handle(i);

                if (imageHandle1 != imageHandle2)
                {
                    ret |= ImageDirty;
                    auto textureHandle1 = manager.query(imageHandle1).textureHandle;
                    auto textureHandle2 = manager.query(imageHandle2).textureHandle;
                    if (textureHandle1 != textureHandle2)
                    {
                        ret |= TextureDirty;
                        return ret;
                    }
                }
            }

            return ret;
        }
    } // namespace

    bool isFontType(EntityType type)
    {
        return type == ScreenTextID;
    }

    bool isTextureType(EntityType type)
    {
        return type > NormalTextureStartID && type < NormalTextureEndID;
    }

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

    void GraphicsEntity::setProfile(const EntityProfile& profile)
    {
        DirtyType dirtyType = 0;

        if (m_profile.matrix != profile.matrix)
        {
            dirtyType |= MatrixDirty;
        }
        if (m_profile.color != profile.color)
        {
            dirtyType |= ColorDirty;
        }
        if (m_profile.color.a != profile.color.a)
        {
            dirtyType |= TransparencyDirty;
        }
        if (m_profile.visible != profile.visible)
        {
            dirtyType |= VisibleDirty;
        }
        if ((m_profile.font == profile.font) == false)
        {
            dirtyType |= DataDirty;
            dirtyType |= FontDirty;
        }
        dirtyType |= compare(m_profile.texture, profile.texture);

        m_profile = profile;

        setDirty(dirtyType);
    }

    const EntityProfile& GraphicsEntity::profile()
    {
        return m_profile;
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
        if (type == NormalTextureStartID || type == NormalTextureEndID)
        {
            assert(0);
            return false;
        }

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
        {ScreenTextID, PrimitiveMode::kTriangles},
        {NormalTextureFaceID_C, PrimitiveMode::kTriangles},
        {NormalTextureFaceID_CN, PrimitiveMode::kTriangles},
        {NormalTextureFaceID_CO, PrimitiveMode::kTriangles},
        {NormalTextureFaceID_CNO, PrimitiveMode::kTriangles}
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
