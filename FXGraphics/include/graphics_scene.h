#ifndef _GRAPHICS_SCENE_H_
#define _GRAPHICS_SCENE_H_

#include "basic_macro.h"
#include "graphics_buffer.h"
#include "graphics_entity_manager.h"
#include "graphics_buffer_manager.h"
#include "graphics_highlight_manager.h"

namespace FX {

    class GraphicsPrinter;
    class GraphicsCamera;

    // 此文件定义了GraphicsScene
    // GraphicsScene管理GraphicsEntity GraphicsPrinter等与绘制相关的资源，并实现绘制的主要流程。
    // 将GraphicsEntity加入GraphicsScene以绘制，一个GraphicsScene可以加入多种类型的GraphicsEntity，但不支持同一个GraphicsEntity重复加入。

    // For users: 用户可以从GraphicsScene派生，以自定义绘制流程。
    
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

        void bindCamera(GraphicsCamera* pCamera);
        void unbindCamera(void);

        bool addHighlight(GraphicsEntity* pEntity, HighlightType type);
        bool removeHighlight(GraphicsEntity* pEntity, HighlightType type);
        bool removeHighlight(GraphicsEntity* pEntity);
        bool removeAllHighlight(HighlightType type);
        bool removeAllHighlight(void);

        virtual void draw(void);

    protected:
        void generate(void);
        virtual void clear(void);
        virtual void bindGlobal(void);
        void beforeDraw(void);
        virtual void unbind(void);
        void afterDraw(void);

    protected:
        using PrinterManager = std::unordered_map<EntityType, GraphicsPrinter*>;

        GraphicsEntityManager* m_pEntityManager = nullptr;
        GraphicsBufferManager* m_pBufferManager = nullptr;
        GraphicsHighlightManager* m_pHighlightManager = nullptr;
        PrinterManager m_printerManager;
        GraphicsCamera* m_pCamera = nullptr;
        GraphicsUBO m_globalUbo;
    };

} // namespace FX

#endif // _GRAPHICS_SCENE_H_
