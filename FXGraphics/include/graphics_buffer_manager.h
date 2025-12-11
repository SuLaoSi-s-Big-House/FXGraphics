#ifndef _GRAPHICS_BUFFER_MANAGER_H_
#define _GRAPHICS_BUFFER_MANAGER_H_

#include <vector>
#include <memory>
#include "basic_macro.h"
#include "graphics_buffer.h"

namespace FX {

    using EntityType = unsigned int;

    class GraphicsScene;
    struct EntityList;

    struct BufferSet {
        std::unique_ptr<GraphicsVAO> pVao;
        std::vector<std::unique_ptr<GraphicsVBO>> pVbos;
        std::unique_ptr<GraphicsEBO> pEbo;
        std::unique_ptr<GraphicsSSBO> pSsbo;
        std::vector<std::unique_ptr<GraphicsDIBO>> pDibos;
    };

    constexpr unsigned int NormalOpaqueCommand = 0;
    constexpr unsigned int NormalTransCommand = 1;
    constexpr unsigned int NormalCommandNum = 2;

    // 此文件定义了GraphicsBufferManager
    // GraphicsBufferManager由GraphicsScene创建，帮助GraphicsScene管理绘制过程中创建的buffer。

    // For users: 用户可以从GraphicsBufferManager派生，以修改绘制中所需的buffer。

    class GraphicsBufferManager {
    protected:
        friend class GraphicsScene;

        explicit GraphicsBufferManager(GraphicsScene* pScene) : m_pScene(pScene) {}
        virtual ~GraphicsBufferManager(void) = default;

        virtual void accept(const EntityList& list, EntityType type, int index);

        virtual const BufferSet& generate(const EntityList& list, EntityType type, int index);

        DELETE_COPY_AND_MOVE_CONSTRUCT(GraphicsBufferManager);

    protected:
        using BufferGroup = std::vector<BufferSet>;
        using BufferContainer = std::unordered_map<EntityType, BufferGroup>;

        BufferContainer m_container;
        GraphicsScene* m_pScene = nullptr;
    };

} // namespace FX

#endif // _GRAPHICS_BUFFER_MANAGER_H_
