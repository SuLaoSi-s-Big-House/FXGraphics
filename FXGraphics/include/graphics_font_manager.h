#ifndef _GRAPHICS_FONT_MANAGER_H_
#define _GRAPHICS_FONT_MANAGER_H_

#include <string>
#include <vector>
#include "basic_vector.h"

namespace FX {

    class GraphicsTexture;
    class GraphicsFontManagerImpl;
    struct Font;

    struct FontInfo {
        const GraphicsTexture* pTexture = nullptr;
    };

    struct TextInfo {
        vec4i textBox;
    };

    struct StringVertex {
        std::vector<float> vertex;
        std::vector<float> normal;
        std::vector<float> uv;
        std::vector<unsigned int> index;
    };

    class GraphicsFontManager {
    public:
        static GraphicsFontManager& instance(void);

        std::string loadFontFile(const std::string& path);

        void prepare(const Font& font, const std::string& texts);

        FontInfo queryFont(const Font& font);
        TextInfo queryText(const Font& font, const std::string& text);
        StringVertex queryStringVertex(const Font& font, const std::string& texts);

    private:
        GraphicsFontManager(void);
        ~GraphicsFontManager(void);

    private:
        GraphicsFontManagerImpl* m_pImpl = nullptr;
    };

} // namespace FX

#endif // _GRAPHICS_FONT_MANAGER_H_
