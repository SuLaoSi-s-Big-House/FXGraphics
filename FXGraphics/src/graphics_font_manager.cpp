#include "graphics_font_manager.h"

#include <unordered_map>
#include <map>
#include <memory>
#include <fstream>
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "basic_log.h"
#include "basic_platform.h"
#include "graphics_entity.h"
#include "graphics_texture.h"

namespace FX {

    namespace {

        constexpr unsigned int TEXT_TEXTURE_SIZE = 1024u;

        struct FontManager {
            FontManager::~FontManager();

            std::string load(const std::string& path);
            const stbtt_fontinfo* find(const std::string& name) const;

            std::unordered_map<std::string, stbtt_fontinfo> fontFiles;
        };

        struct TextData {
            int code = 0;
            stbtt_bakedchar data;
        };

        struct FontData {
            std::vector<std::vector<TextData>> textList;
            std::unordered_map<int, vec2i> textMap;
            std::unique_ptr<GraphicsTexture> pTexture;
            int lastRow = 0;
            float ascent = 0;
            float descent = 0;
            float lineGap = 0;
        };

        struct FontCompare {
            bool operator()(const Font& left, const Font& right) const
            {
                return left.name == right.name ? left.size < right.size : left.name < right.name;
            }
        };

        struct TextManager {
            void generate(const Font& font, const stbtt_fontinfo* pFontFile, const std::vector<int>& codes);

            std::map<Font, FontData, FontCompare> textContainer;
        };

        // 从ttf文件中读取字体名称
        inline std::string findNameFromTTFFile(const stbtt_fontinfo& fontFile)
        {
            assert(fontFile.data != nullptr);

            constexpr int FULL_NAME_ID = 4;

            std::string fontName;
            int length = 0;
            const char* pName = nullptr;
#ifdef FX_WINDOWS
            // 在Windows中，platform ID为3
            // 随后查找encoding ID，先查找full Unicode(10)，如果没有则查找Unicode BMP(1)
            // 随后确认language ID，这里始终使用English(1033)
            // 最后读取full name(4)字段的值作为字体名称
            pName = stbtt_GetFontNameString(&fontFile, &length, STBTT_PLATFORM_ID_MICROSOFT, STBTT_MS_EID_UNICODE_FULL,
                STBTT_MS_LANG_ENGLISH, FULL_NAME_ID);

            if (pName == nullptr || length == 0)
            {
                pName = stbtt_GetFontNameString(&fontFile, &length, STBTT_PLATFORM_ID_MICROSOFT, STBTT_MS_EID_UNICODE_BMP,
                    STBTT_MS_LANG_ENGLISH, FULL_NAME_ID);
            }
#else
#error "New platform to implement!"
#endif

            if (pName == nullptr || length == 0)
            {
                BasicLog::out(BasicLog::kWarn, "Cannot get font name, please check font file.");
                return fontName;
            }

            fontName.reserve(length);
            for (int i = 0; i < length; i++)
            {
                // stb接口返回的字体名称一定是宽字符且是大端序
                // 由于前面始终查找English字符集中的字体名称，这里认为pName中一定只有ascii字符，因此删除0值即可
                if (pName[i] != '\0')
                {
                    fontName.push_back(pName[i]);
                }
            }

            return fontName;
        }

        // 将utf-8字符串转为unicode
        // unicode是在ttf文件中查找字符的键值
        inline std::vector<int> utf8ToUnicode(const std::string& texts)
        {
            assert(texts.empty() == false);

            std::vector<int> codes;
            codes.reserve(texts.length());

            for (int i = 0; i < texts.length();)
            {
                int add = 0;
                unsigned char first = texts[i];
                if (first < 0x80)
                {
                    codes.push_back(first);
                    add = 1;
                }
                else if ((first & 0xE0) == 0xC0)
                {
                    if (texts.length() > i + 1)
                    {
                        codes.push_back(((first & 0x1F) << 6) | (texts[i + 1] & 0x3F));
                        add = 2;
                    }
                }
                else if ((first & 0xF0) == 0xE0)
                {
                    if (texts.length() > i + 2)
                    {
                        codes.push_back(((first & 0x0F) << 12) | ((texts[i + 1] & 0x3F) << 6) | (texts[i + 2] & 0x3F));
                        add = 3;
                    }
                }
                else if ((first & 0xF8) == 0xF0)
                {
                    if (texts.length() > i + 3)
                    {
                        codes.push_back(((first & 0x07) << 18) | ((texts[i + 1] & 0x3F) << 12) | ((texts[i + 2] & 0x3F) << 6) | (texts[i + 3] & 0x3F));
                        add = 4;
                    }
                }

                if (add == 0)
                {
                    BasicLog::out(BasicLog::kWarn, "Something goes wrong when translating utf-8 string to unicode.");
                    break;
                }
                i += add;
            }

            return codes;
        }

        inline void collectNew(const std::unordered_map<int, vec2i>& textMap, std::vector<int>& codes)
        {
            if (codes.empty())
            {
                return;
            }

            std::set<int> newCodes;
            for (auto i : codes)
            {
                if (i != '\n' && textMap.count(i) == 0)
                {
                    newCodes.insert(i);
                }
            }

            codes = std::vector<int>(newCodes.begin(), newCodes.end());
        }

        inline void initFontData(const Font& font, const stbtt_fontinfo* pFontFile, FontData& fontData)
        {
            assert(font.valid());
            assert(pFontFile != nullptr);
            assert(fontData.textList.empty() && fontData.textMap.empty());
            assert(fontData.pTexture == nullptr);

            fontData.pTexture.reset(new GraphicsTexture());
            fontData.pTexture->setSoftFilter(false);
            fontData.pTexture->setUseMipmap(false);
            fontData.pTexture->setTransparent(true);

            BasicImage<> image;
            image.setData(TEXT_TEXTURE_SIZE, TEXT_TEXTURE_SIZE, 1, nullptr);
            fontData.pTexture->pushImage(image);
            fontData.lastRow = 0;    // lastRow = 0标明是新创建的image

            fontData.textList.emplace_back(std::vector<TextData>());

            float scale = stbtt_ScaleForPixelHeight(pFontFile, static_cast<float>(font.size));
            int ascent = 0;
            int descent = 0;
            int lineGap = 0;
            stbtt_GetFontVMetrics(pFontFile, &ascent, &descent, &lineGap);
            fontData.lineGap = (lineGap + ascent - descent) * scale;
            fontData.ascent = ascent * scale;
            fontData.descent = descent * scale;
        }

        int bakeFontImage(const Font& font, const stbtt_fontinfo& fontFile, unsigned char* pImage, int width, int height,
            const int* pCharacter, int num, std::vector<stbtt_bakedchar>& textData)
        {
            assert(pImage != nullptr);
            assert(pCharacter != nullptr);
            assert(font.valid());
            assert(width > 0);
            assert(height > 0);
            assert(num > 0);

            vec3i pos = { 1, 1, 1 };    // 表示下一个字符的起始坐标
            float scale = stbtt_ScaleForPixelHeight(&fontFile, static_cast<float>(font.size));
            memset(pImage, 0, width * height * sizeof(unsigned char));

            for (int i = 0; i < num; i++)
            {
                // 找到字符在表中的索引，返回0表示没找到
                int index = stbtt_FindGlyphIndex(&fontFile, pCharacter[i]);
                if (index == 0)
                {
                    BasicLog::out(BasicLog::kWarn, "Cannot find character ", pCharacter[i], "(unicode) in font ", font.name, ", please check.");
                    continue;
                }

                // 获取字符左右需要空出的空间，以font point为单位
                int advance = 0;
                int leftBearing = 0;
                stbtt_GetGlyphHMetrics(&fontFile, index, &advance, &leftBearing);

                // 获取字符的最小外包围盒，以像素为单位
                // 存储顺序为x0 x1 y0 y1
                vec4i box;
                stbtt_GetGlyphBitmapBox(&fontFile, index, scale, scale, &box.x, &box.z, &box.y, &box.w);
                vec4i size = { box.y - box.x, box.w - box.z };

                // 判断是否需要换行
                if (pos.x + size.x + 1 >= width)
                {
                    pos = { 1, pos.z, pos.z };
                }

                // 判断图片是否还能容纳下新的字符
                if (pos.y + size.y + 1 >= height)
                {
                    return -i;
                }

                // 生成字符数据
                stbtt_MakeGlyphBitmap(&fontFile, pImage + pos.x + pos.y * width, size.x, size.y, width, scale, scale, index);
                stbtt_bakedchar data;
                data.x0 = static_cast<unsigned short>(pos.x);
                data.y0 = static_cast<unsigned short>(pos.y);
                data.x1 = static_cast<unsigned short>(pos.x + size.x);
                data.y1 = static_cast<unsigned short>(pos.y + size.y);
                data.xadvance = scale * advance;
                data.xoff = static_cast<float>(box.x);
                data.yoff = static_cast<float>(box.z);
                textData.emplace_back(std::move(data));

                pos.x += size.x + 1;
                pos.z = std::max(pos.z, pos.y + size.y + 1);
            }

            return pos.z;
        }

        FontManager::~FontManager()
        {
            for (auto& pair : fontFiles)
            {
                assert(pair.second.data != nullptr);
                delete[] pair.second.data;
            }
        }

        std::string FontManager::load(const std::string& path)
        {
            std::string fontName;

            if (path.empty())
            {
                return fontName;
            }

            std::ifstream file;
            file.open(path, std::ios::binary | std::ios::in);

            if (file.is_open() == false)
            {
                BasicLog::out(BasicLog::kWarn, "Cannot find font file, please check file path.");
                return fontName;
            }

            auto start = file.tellg();
            file.seekg(0, std::ios::end);
            auto end = file.tellg();
            auto size = std::size_t(end - start);

            if (size == 0)
            {
                BasicLog::out(BasicLog::kWarn, "Empty file when loading font, ignored.");
                return fontName;
            }

            // new pFileData用于存放字体文件的内容，将直接存入stbtt_fontinfo.data
            auto pFileData = new unsigned char[size];
            file.seekg(0, std::ios::beg);
            file.read((char*)pFileData, size);
            file.close();

            stbtt_fontinfo fontInfo;
            memset(&fontInfo, 0, sizeof(fontInfo));

            if (stbtt_InitFont(&fontInfo, pFileData, 0) == 0)
            {
                BasicLog::out(BasicLog::kWarn, "Cannot load font, please check font file.");
                delete[] pFileData;
                return fontName;
            }

            fontName = findNameFromTTFFile(fontInfo);

            if (fontName.empty())
            {
                delete[] pFileData;
                return fontName;
            }

            auto itr = fontFiles.find(fontName);
            if (itr == fontFiles.end())
            {
                fontFiles.insert({ fontName, fontInfo });
            }
            else
            {
                delete[] pFileData;    // 如果发现这个字体已经被记录，则立刻释放内存，否则会在析构函数中释放
            }

            return fontName;
        }

        const stbtt_fontinfo* FontManager::find(const std::string& name) const
        {
            auto itr = fontFiles.find(name);
            return itr == fontFiles.end() ? nullptr : &itr->second;
        }

        void TextManager::generate(const Font& font, const stbtt_fontinfo* pFontFile, const std::vector<int>& codes)
        {
            assert(font.valid());
            assert(pFontFile != nullptr);
            assert(codes.empty() == false);

            auto& fontData = textContainer[font];

            if (fontData.textList.empty() && fontData.textMap.empty())
            {
                initFontData(font, pFontFile, fontData);
            }

#if defined(_DEBUG) || defined(DEBUG)
            // 确保都是需要生成的新的unicode
            for (auto i : codes)
            {
                assert(fontData.textMap.count(i) == 0);
            }
#endif

            // 遍历生成所有字符，i为已经生成的数量
            for (int i = 0; i < codes.size();)
            {
                // n为还未处理的字符的个数
                int n = static_cast<int>(codes.size() - i);

                // 总是拿取最后一张image继续生成
                // 最后一张image可能是刚刚创建的空的image，也有可能是上次填充了一半的image，根据lastRow区分
                assert(fontData.pTexture != nullptr);
                auto imageList = fontData.pTexture->imageList();
                assert(imageList.empty() == false);
                auto pImage = static_cast<unsigned char*>(imageList.back());
                assert(pImage);

                auto lastRow = fontData.lastRow;
                std::vector<stbtt_bakedchar> textData;
                auto ret = bakeFontImage(font, *pFontFile, pImage + lastRow * TEXT_TEXTURE_SIZE, TEXT_TEXTURE_SIZE,
                    TEXT_TEXTURE_SIZE - lastRow, codes.data() + i, n, textData);    // 注意根据lastRow进行偏移

                if (textData.empty() == false)
                {
                    // 将这一轮生成的字符数据存入数组
                    assert(fontData.textList.empty() == false);
                    auto& list = fontData.textList.back();
                    auto& map = fontData.textMap;

                    for (int j = 0; j < textData.size(); j++)
                    {
                        textData[j].y0 += fontData.lastRow;
                        textData[j].y1 += fontData.lastRow;
                        int code = codes[i + j];
                        list.emplace_back(TextData{ code, textData[j] });
                        map.insert({ code, { static_cast<int>(imageList.size() - 1), static_cast<int>(list.size() - 1) } });
                    }

                    fontData.pTexture->markImageDirty(static_cast<unsigned int>(imageList.size() - 1));
                }

                // 如果ret > 0，表示这一轮已经生成全部字符
                if (ret > 0)
                {
                    assert(textData.size() == n);
                    fontData.lastRow += ret;
                    return;
                }
                else    // 如果ret <= 0，需要添加一张image并继续生成
                {
                    assert(textData.size() < n);
                    i += static_cast<int>(textData.size());
                    BasicImage<> image;
                    image.setData(TEXT_TEXTURE_SIZE, TEXT_TEXTURE_SIZE, 1, nullptr);
                    fontData.pTexture->pushImage(image);
                    fontData.lastRow = 0;
                    fontData.textList.emplace_back(std::vector<TextData>());
                }
            }
        }

    }  // namespace

    class GraphicsFontManagerImpl {
    public:
        friend class GraphicsFontManager;

        void prepare(const Font& font, const std::string& texts);
        FontInfo queryFont(const Font& font) const;
        StringVertex queryStringVertex(const Font& font, const std::string& texts);

    private:
        FontManager m_fontManager;
        TextManager m_textManager;
    };

    GraphicsFontManager& GraphicsFontManager::instance()
    {
        static GraphicsFontManager instance;
        return instance;
    }

    GraphicsFontManager::GraphicsFontManager()
    {
        m_pImpl = new GraphicsFontManagerImpl;
    }

    GraphicsFontManager::~GraphicsFontManager()
    {
        delete m_pImpl;
    }

    std::string GraphicsFontManager::loadFontFile(const std::string& path)
    {
        return m_pImpl->m_fontManager.load(path);
    }

    void GraphicsFontManager::prepare(const Font& font, const std::string& texts)
    {
        m_pImpl->prepare(font, texts);
    }

    FontInfo GraphicsFontManager::queryFont(const Font& font) const
    {
        return m_pImpl->queryFont(font);
    }

    StringVertex GraphicsFontManager::queryStringVertex(const Font& font, const std::string& texts)
    {
        return m_pImpl->queryStringVertex(font, texts);
    }

    void GraphicsFontManagerImpl::prepare(const Font& font, const std::string& texts)
    {
        if (texts.empty())
        {
            return;
        }

        if (font.valid() == false)
        {
            BasicLog::out(BasicLog::kWarn, "Trying to load texts with invalid font, discard.");
            return;
        }

        auto pFontFile = m_fontManager.find(font.name);
        if (pFontFile == nullptr)
        {
            BasicLog::out(BasicLog::kWarn, "You need to load font file first before loading texts.");
            return;
        }

        auto codes = utf8ToUnicode(texts);
        if (codes.empty())
        {
            BasicLog::out(BasicLog::kWarn, "Cannot get unicode ID from these texts.");
            return;
        }

        auto& textMap = m_textManager.textContainer[font].textMap;
        collectNew(textMap, codes);

        m_textManager.generate(font, pFontFile, codes);
    }

    FontInfo GraphicsFontManagerImpl::queryFont(const Font& font) const
    {
        FontInfo ret;

        auto itr = m_textManager.textContainer.find(font);
        if (itr == m_textManager.textContainer.end())
        {
            BasicLog::out(BasicLog::kWarn, "Font [", font.name, "] dose not exist or not loaded.");
            return ret;
        }

        ret.pTexture = itr->second.pTexture.get();
        return ret;
    }

    StringVertex GraphicsFontManagerImpl::queryStringVertex(const Font& font, const std::string& texts)
    {
        StringVertex ret;

        if (texts.empty())
        {
            return ret;
        }

        if (font.valid() == false)
        {
            BasicLog::out(BasicLog::kWarn, "Trying to load texts with invalid font, discard.");
            return ret;
        }

        auto pFontFile = m_fontManager.find(font.name);
        if (pFontFile == nullptr)
        {
            BasicLog::out(BasicLog::kWarn, "You need to load font file first before loading texts.");
            return ret;
        }

        auto codes = utf8ToUnicode(texts);
        if (codes.empty())
        {
            BasicLog::out(BasicLog::kWarn, "Cannot get unicode ID from these texts.");
            return ret;
        }

        std::vector<int> newCodes = codes;
        auto& fontData = m_textManager.textContainer[font];
        collectNew(fontData.textMap, newCodes);

        if (newCodes.empty() == false)
        {
            m_textManager.generate(font, pFontFile, newCodes);
        }

        // 现在已经字符的生成，开始组织顶点
        ret.vertex.reserve(codes.size() * 12);
        ret.normal.reserve(codes.size() * 12);
        ret.uv.reserve(codes.size() * 12);
        ret.index.reserve(codes.size() * 6);
        vec2f advance = { 0, fontData.ascent };

        for (auto code : codes)
        {
            // 如果是换行符，更新advance
            if (code == '\n')
            {
                advance = { 0, advance.y + fontData.lineGap };
                continue;
            }

            vec2i offset = { static_cast<int>(advance.x + 0.5f), static_cast<int>(advance.y + 0.5f) };    // 取整，让字符对齐像素

            auto itr = fontData.textMap.find(code);
            assert(itr != fontData.textMap.end());
            assert(fontData.textList.size() > itr->second.x);
            assert(fontData.textList[itr->second.x].size() > itr->second.y);
            assert(fontData.textList[itr->second.x][itr->second.y].code == code);

            assert(ret.vertex.size() % 12 == 0);
            unsigned int i = static_cast<unsigned int>(ret.vertex.size()) / 3;

            // 索引，总是最后四个点
            ret.index.push_back(i); ret.index.push_back(i + 2); ret.index.push_back(i + 1);
            ret.index.push_back(i + 1); ret.index.push_back(i + 2); ret.index.push_back(i + 3);

            auto& textData = fontData.textList[itr->second.x][itr->second.y].data;

            ret.vertex.push_back(offset.x + textData.xoff);
            ret.vertex.push_back(offset.y + textData.yoff);
            ret.vertex.push_back(0);
            ret.vertex.push_back(offset.x + textData.xoff + textData.x1 - textData.x0);
            ret.vertex.push_back(offset.y + textData.yoff);
            ret.vertex.push_back(0);
            ret.vertex.push_back(offset.x + textData.xoff);
            ret.vertex.push_back(offset.y + textData.yoff + textData.y1 - textData.y0);
            ret.vertex.push_back(0);
            ret.vertex.push_back(offset.x + textData.xoff + textData.x1 - textData.x0);
            ret.vertex.push_back(offset.y + textData.yoff + textData.y1 - textData.y0);
            ret.vertex.push_back(0);

            // 法向（无意义值）
            ret.normal.push_back(0); ret.normal.push_back(0); ret.normal.push_back(-1);
            ret.normal.push_back(0); ret.normal.push_back(0); ret.normal.push_back(-1);
            ret.normal.push_back(0); ret.normal.push_back(0); ret.normal.push_back(-1);
            ret.normal.push_back(0); ret.normal.push_back(0); ret.normal.push_back(-1);

            ret.uv.push_back(static_cast<float>(textData.x0) / TEXT_TEXTURE_SIZE);
            ret.uv.push_back(static_cast<float>(textData.y0) / TEXT_TEXTURE_SIZE);
            ret.uv.push_back(static_cast<float>(itr->second.x));
            ret.uv.push_back(static_cast<float>(textData.x1) / TEXT_TEXTURE_SIZE);
            ret.uv.push_back(static_cast<float>(textData.y0) / TEXT_TEXTURE_SIZE);
            ret.uv.push_back(static_cast<float>(itr->second.x));
            ret.uv.push_back(static_cast<float>(textData.x0) / TEXT_TEXTURE_SIZE);
            ret.uv.push_back(static_cast<float>(textData.y1) / TEXT_TEXTURE_SIZE);
            ret.uv.push_back(static_cast<float>(itr->second.x));
            ret.uv.push_back(static_cast<float>(textData.x1) / TEXT_TEXTURE_SIZE);
            ret.uv.push_back(static_cast<float>(textData.y1) / TEXT_TEXTURE_SIZE);
            ret.uv.push_back(static_cast<float>(itr->second.x));

            advance.x += textData.xadvance;
        }

        return ret;
    }

} // namespace FX
