#ifndef _SURF_TEXT_ITEM_H_
#define _SURF_TEXT_ITEM_H_

#include "surf_item.h"

namespace FX {

    class SurfTextItem : public SurfItem {

    };


    class SurfTextEntity : public SurfEntity {
    public:
        SurfTextEntity(void) : SurfEntity(ScreenTextID) {}
        SurfTextEntity(const Font& font, const std::string& str, const vec2i& pos);

        void setText(const std::string& str);
        const std::string& text(void) const;

        void generate(void) override;

    protected:
        std::string m_texts;
    };

} // namespace FX

#endif // _SURF_TEXT_ITEM_H_
