#include "surf_text_item.h"

#include "graphics_font_manager.h"
#include "graphics_material_manager.h"

namespace FX {

    SurfTextEntity::SurfTextEntity(const Font& font, const std::string& str, const vec2i& pos)
        : SurfEntity(ScreenTextID), m_texts(str)
    {
        Material material = GraphicsMaterialManager::instance().get(m_materialHandle);
        material.font = font;
        GraphicsMaterialManager::instance().unref(m_materialHandle);
        m_materialHandle = GraphicsMaterialManager::instance().ref(material);
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
        auto vertex = GraphicsFontManager::instance().queryStringVertex(material().font, m_texts);
        m_vertex = std::move(vertex.vertex);
        m_normal = std::move(vertex.normal);
        m_uv = std::move(vertex.uv);
        m_index = std::move(vertex.index);
    }

} // namespace FX
