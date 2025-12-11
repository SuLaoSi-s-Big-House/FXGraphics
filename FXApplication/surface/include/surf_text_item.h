#ifndef _SURF_TEXT_ITEM_H_
#define _SURF_TEXT_ITEM_H_

#include <string>

#include "basic_vector.h"
#include "graphics_entity.h"

namespace FX {

    class TextEntity : public GraphicsEntity {
    public:
        TextEntity(void) : GraphicsEntity(ScreenTextID) {}
        TextEntity(const Font& font, const std::string& str, const vec2i& pos);

        void setText(const std::string& str);
        const std::string& text(void) const;

        void setPosition(const vec2i& pos);
        const vec2i& position(void) const;

        EntityProfile profile(void) const override;

        void generate(void) override;

    protected:
        std::string m_texts;
        vec2i m_position;
    };

} // namespace FX

#endif // _SURF_TEXT_ITEM_H_
