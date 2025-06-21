#ifndef _GRAPHICS_BUFFER_MANAGER_H_
#define _GRAPHICS_BUFFER_MANAGER_H_

#include <vector>
#include "basic_macro.h"
#include "graphics_buffer.h"

namespace FX {

    using EntityType = unsigned short;

    class GraphicsScene;
    struct EntityList;

    class GraphicsBufferManager {
    protected:
        friend class GraphicsScene;

        explicit GraphicsBufferManager(GraphicsScene* pScene) : m_pScene(pScene) {}
        virtual ~GraphicsBufferManager(void);

        void accept(const EntityList& list, EntityType type, int index);

        void generate(const EntityList& list, EntityType type, int index);

        DELETE_COPY_AND_MOVE_CONSTRUCT(GraphicsBufferManager);

    public:

    protected:
        struct BufferSet {
            GraphicsVAO* pVao = nullptr;
            std::vector<GraphicsVBO*> pVbos;
            GraphicsEBO* pEbo = nullptr;
            GraphicsSSBO* pSsbo = nullptr;
            std::vector<GraphicsDIBO*> pDibos;
            bool init = false;
        };
         
        using BufferGroup = std::vector<BufferSet>;
        using BufferContainer = std::map<EntityType, BufferGroup>;

        BufferContainer m_container;
        GraphicsScene* m_pScene = nullptr;
    };

} // namespace FX

#endif // _GRAPHICS_BUFFER_MANAGER_H_
