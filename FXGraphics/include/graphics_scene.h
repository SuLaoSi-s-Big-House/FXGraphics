#ifndef _GRAPHICS_SCENE_H_
#define _GRAPHICS_SCENE_H_

#include "basic_macro.h"
#include "graphics_entity_manager.h"

namespace FX {

    class GraphicsScene {
    public:
        GraphicsScene(void);
        virtual ~GraphicsScene(void);

        DELETE_COPY_AND_MOVE_CONSTRUCT(GraphicsScene);

    public:
        bool addEntity(GraphicsEntity* pEntity);
        bool removeEntity(GraphicsEntity* pEntity);
        bool dirtyEntity(GraphicsEntity* pEntity, DirtyType type);

        virtual void draw(void);

    protected:
        GraphicsEntityManager* m_pEntityManager = nullptr;
    };

} // namespace FX

#endif // _GRAPHICS_SCENE_H_
