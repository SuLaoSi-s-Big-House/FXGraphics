#ifndef _GRAPHICS_FONT_MANAGER_H_
#define _GRAPHICS_FONT_MANAGER_H_

#include <map>
#include <unordered_map>
#include <vector>
#include <memory>
#include "stb_truetype.h"

#include "basic_vector.h"
#include "graphics_entity.h"
#include "graphics_texture.h"

namespace FX {

    struct FontInfo {
        const GraphicsTexture* pTexture = nullptr;
        float lineGap = 0;
        float ascent = 0;
    };

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

        void generate(const Font& font, const std::string& texts);

        FontInfo queryFont(const Font& font) const;
        TextVertex queryStringVertex(const Font& font, const std::string& texts);

    private:
        GraphicsFontManager(void) = default;
        ~GraphicsFontManager(void);

        void generate(const Font& font, const std::vector<int>& codes);

    private:
        struct TextData {
            int code = 0;
            stbtt_bakedchar data;
        };

        struct FontData {
            std::unordered_map<int, vec2i> textMap;
            std::vector<std::vector<TextData>> textArray;
            std::unique_ptr<GraphicsTexture> pTexture;
            int lastRow = 0;
            float lineGap = 0;
            float ascent = 0;
        };

        struct FontCompare {
            bool operator()(const Font& left, const Font& right) const;
        };

        std::map<Font, FontData, FontCompare> m_fontMap;
        std::unordered_map<std::string, stbtt_fontinfo> m_fontFiles;
    };

} // namespace FX

#endif // _GRAPHICS_FONT_MANAGER_H_
