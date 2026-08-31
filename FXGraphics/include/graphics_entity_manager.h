#ifndef _GRAPHICS_ENTITY_MANAGER_H_
#define _GRAPHICS_ENTITY_MANAGER_H_

#include <vector>
#include <set>
#include <unordered_map>

#include "basic_macro.h"
#include "graphics_entity.h"

namespace FX {

    class GraphicsScene;

    struct EntityList {
        using DirtyList = std::set<int, std::less<int>>;

        void setRebuildStart(int start);

        void arrange(void);
        void clean(void);
        void generate(void);

        bool isDirty(void) const;
        void setReady(void);

        std::vector<GraphicsEntity*> entityList;

        std::vector<unsigned int> pointSum;
        std::vector<unsigned int> indexSum;

        // 最近一次generate时，平均每个活实体的顶点数/索引数（分母为活实体数，分子含已删除
        // 实体的残留数据，因此略微偏大，对容量估算而言是保守方向）。
        // 用于shouldAcceptEntity估算尚未generate的实体尺寸。
        unsigned int pointAvg = 0;
        unsigned int indexAvg = 0;

        DirtyList matrixList;
        DirtyList profileList;
        DirtyList commandList;
        int rebuildStart = -1;

        unsigned int invalidNum = 0;

        GraphicsEntityManager* pOwner = nullptr;
        int index = 0;

        Font font;           // 当且仅当文字类实体时有意义
        TextureKey texture;  // 当且仅当纹理类实体时有意义
    };

    // 此文件定义了GraphicsEntityManager
    // GraphicsEntityManager由GraphicsScene创建，是GraphicsScene管理GraphicsEntity的功能的拆分

    // For users: 用户可以从GraphicsEntityManager派生，以实现特定的管理能力。

    class GraphicsEntityManager {
    protected:
        friend class GraphicsScene;

        explicit GraphicsEntityManager(GraphicsScene* pScene) : m_pScene(pScene) {}
        virtual ~GraphicsEntityManager(void);

        DELETE_COPY_AND_MOVE_CONSTRUCT(GraphicsEntityManager);

    public:
        bool addEntity(GraphicsEntity* pEntity);
        bool removeEntity(GraphicsEntity* pEntity);

        bool dirtyEntity(GraphicsEntity* pEntity, DirtyType type);

    protected:
        EntityList& findBestListForEntity(GraphicsEntity* pEntity);

    protected:
        using EntityGroup = std::vector<EntityList>;
        using EntityContainer = std::unordered_map<EntityType, EntityGroup>;

        EntityContainer m_container;
        GraphicsScene* m_pScene = nullptr;
    };

} // namespace FX

#endif // _GRAPHICS_ENTITY_MANAGER_H_
