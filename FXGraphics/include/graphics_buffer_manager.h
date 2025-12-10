#ifndef _GRAPHICS_BUFFER_MANAGER_H_
#define _GRAPHICS_BUFFER_MANAGER_H_

#include <vector>
#include "basic_macro.h"
#include "graphics_buffer.h"

namespace FX {

    using EntityType = unsigned int;

    class GraphicsScene;
    struct EntityList;

    struct BufferSet {
        GraphicsVAO* pVao = nullptr;
        std::vector<GraphicsVBO*> pVbos;
        GraphicsEBO* pEbo = nullptr;
        GraphicsSSBO* pSsbo = nullptr;
        std::vector<GraphicsDIBO*> pDibos;
        bool init = false;
    };

    // 此文件定义了GraphicsBufferManager
    // GraphicsBufferManager由GraphicsScene创建，帮助GraphicsScene管理绘制过程中创建的buffer。

    // For users: 用户可以从GraphicsBufferManager派生，以修改绘制中所需的buffer。

    class GraphicsBufferManager {
    protected:
        friend class GraphicsScene;

        explicit GraphicsBufferManager(GraphicsScene* pScene) : m_pScene(pScene) {}
        virtual ~GraphicsBufferManager(void);

        void accept(const EntityList& list, EntityType type, int index);

        const BufferSet& generate(const EntityList& list, EntityType type, int index);

        DELETE_COPY_AND_MOVE_CONSTRUCT(GraphicsBufferManager);

    protected:
        using BufferGroup = std::vector<BufferSet>;
        using BufferContainer = std::unordered_map<EntityType, BufferGroup>;

        BufferContainer m_container;
        GraphicsScene* m_pScene = nullptr;
    };

} // namespace FX

#endif // _GRAPHICS_BUFFER_MANAGER_H_
