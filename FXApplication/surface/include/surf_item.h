#ifndef _SURF_ITEM_H_
#define _SURF_ITEM_H_

#include "graphics_entity.h"

namespace FX {

    class Item {

    };

    class SurfItem : public Item {

    };


    class EntityBase : public GraphicsEntity {
    public:
        explicit EntityBase(EntityType type) : GraphicsEntity(type) {}
    };


    class SurfEntity : public EntityBase {
    public:
        explicit SurfEntity(EntityType type) : EntityBase(type) {}

        // 设置实体在屏幕上的位置，以左上角为原点，以像素为单位
        void setPosition(const vec2i& pos);
        const vec2i& position(void) const;

        // 设置实体的深度，depth应该在[-1, 1]范围内
        void setDepth(float depth);
        float depth(void) const;

        const EntityProfile& profile(void) override;

    protected:
        vec2i m_position;
        float m_depth = 1e-6f - 1;
    };

} // namespace FX

#endif // _SURF_ITEM_H_
