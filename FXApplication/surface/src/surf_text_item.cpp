#include "surf_text_item.h"

#include "graphics_font_manager.h"

namespace FX {

    SurfTextEntity::SurfTextEntity(const Font& font, const std::string& str, const vec2i& pos)
        : SurfEntity(ScreenTextID), m_texts(str)
    {
        m_profile.font = font;
        m_position = pos;
    }

    void SurfTextEntity::setText(const std::string& str)
    {
        if (m_texts != str)
        {
            m_texts = str;
            setDirty(DataDirty);
        }
    }

    const std::string& SurfTextEntity::text() const
    {
        return m_texts;
    }

    void SurfTextEntity::generate()
    {
        auto vertex = GraphicsFontManager::instance().queryStringVertex(m_profile.font, m_texts);
        m_vertex = std::move(vertex.vertex);
        m_normal = std::move(vertex.normal);
        m_uv = std::move(vertex.uv);
        m_index = std::move(vertex.index);
    }

} // namespace FX
