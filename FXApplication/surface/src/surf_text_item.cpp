#include "surf_text_item.h"

namespace FX {

    TextEntity::TextEntity(const Font& font, const std::string& str, const vec2i& pos)
        : GraphicsEntity(NormalTextID), m_texts(str), m_position(pos)
    {
        m_profile.font = font;
    }

    void TextEntity::setText(const std::string& str)
    {
        if (m_texts != str)
        {
            m_texts = str;
            setDirty(DataDirty);
        }
    }

    const std::string& TextEntity::text() const
    {
        return m_texts;
    }

    void TextEntity::setPosition(const vec2i& pos)
    {
        if (m_position != pos)
        {
            m_position = pos;
            setDirty(MatrixDirty);
        }
    }

    const vec2i& TextEntity::position() const
    {
        return m_position;
    }

    void TextEntity::generate()
    {
        auto vertex = GraphicsFontManager::instance().generate(m_profile.font, m_texts);
        m_vertex = vertex.vertex;
        m_normal = vertex.normal;
        m_uv = vertex.uv;
        m_index = vertex.index;
    }

} // namespace FX
