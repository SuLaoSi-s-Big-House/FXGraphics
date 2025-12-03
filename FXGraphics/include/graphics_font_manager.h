#ifndef _GRAPHICS_FONT_MANAGER_H_
#define _GRAPHICS_FONT_MANAGER_H_

#include <map>
#include <vector>
#include <memory>
#include "stb_truetype.h"

#include "basic_vector.h"
#include "graphics_entity.h"
#include "graphics_texture.h"

namespace FX {

    struct TextVertex {
        std::vector<float> vertex;
        std::vector<float> normal;
        std::vector<float> uv;
        std::vector<unsigned int> index;
    };

    class GraphicsFontManager {
    public:
        static GraphicsFontManager& instance(void);

        std::string loadFontFile(const std::string& path);

        TextVertex generate(const Font& font, const std::string& texts);

        GraphicsTexture* getTexture(const Font& font) const;

    private:
        GraphicsFontManager(void) = default;
        ~GraphicsFontManager(void);

    private:
        struct TextData {
            int code = 0;
            stbtt_bakedchar data;
        };

        struct FontData {
            std::map<int, vec2i> textMap;
            std::vector<std::vector<TextData>> textArray;
            std::unique_ptr<GraphicsTexture> pTexture;
            int lastRow = 0;
            int lineGap = 0;
        };

        struct FontCompare {
            bool operator()(const Font& left, const Font& right) const;
        };

        std::map<Font, FontData, FontCompare> m_fontMap;
        std::map<std::string, stbtt_fontinfo> m_fontFiles;
    };

} // namespace FX

#endif // _GRAPHICS_FONT_MANAGER_H_
