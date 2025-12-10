#ifndef _GRAPHICS_SCENE_H_
#define _GRAPHICS_SCENE_H_

#include "basic_macro.h"
#include "graphics_buffer.h"
#include "graphics_entity_manager.h"
#include "graphics_buffer_manager.h"

namespace FX {

    class GraphicsPrinter;

    // 此文件定义了GraphicsScene
    // GraphicsScene管理GraphicsEntity GraphicsPrinter等与绘制相关的资源，并实现绘制的主要流程。
    
    class GraphicsScene {
    public:
        GraphicsScene(void);
        virtual ~GraphicsScene(void);

        DELETE_COPY_AND_MOVE_CONSTRUCT(GraphicsScene);

    public:
        bool addEntity(GraphicsEntity* pEntity);
        bool removeEntity(GraphicsEntity* pEntity);
        bool dirtyEntity(GraphicsEntity* pEntity, DirtyType type);

        bool addPrinter(GraphicsPrinter* pPrinter, EntityType type);
        bool removePrinter(GraphicsPrinter* pPrinter, EntityType type);
        bool removePrinter(GraphicsPrinter* pPrinter);

        virtual void draw(void);

    protected:
        using PrinterManager = std::unordered_map<EntityType, GraphicsPrinter*>;

        GraphicsEntityManager* m_pEntityManager = nullptr;
        GraphicsBufferManager* m_pBufferManager = nullptr;
        PrinterManager m_printerManager;
    };

} // namespace FX

#endif // _GRAPHICS_SCENE_H_
