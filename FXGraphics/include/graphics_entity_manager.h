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

        DirtyList matrixList;
        DirtyList profileList;
        DirtyList commandList;
        int rebuildStart = -1;

        unsigned int invalidNum = 0;

        GraphicsEntityManager* pOwner = nullptr;
        int index = 0;

        Font font;    // 当且仅当文字类实体时有意义
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
        using EntityGroup = std::vector<EntityList>;
        using EntityContainer = std::unordered_map<EntityType, EntityGroup>;

        EntityContainer m_container;
        GraphicsScene* m_pScene = nullptr;
    };

} // namespace FX

#endif // _GRAPHICS_ENTITY_MANAGER_H_
