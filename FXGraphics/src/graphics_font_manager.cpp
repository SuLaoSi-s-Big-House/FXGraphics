#include "graphics_font_manager.h"

#include <set>
#include <fstream>
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "basic_log.h"
#include "basic_platform.h"

namespace FX {

    namespace {

        constexpr int FULL_NAME_ID = 4;
        constexpr unsigned int TEXT_TEXTURE_SIZE = 1024u;

        inline std::vector<int> utf8ToUnicode(const std::string& str)
        {
            std::vector<int> codes;
            codes.reserve(str.length());

            for (int i = 0; i < str.length();)
            {
                unsigned char first = str[i];
                if (first < 0x80)
                {
                    codes.push_back(first);
                    i += 1;
                }
                else if ((first & 0xE0) == 0xC0)
                {
                    if (str.length() > i + 1)
                    {
                        codes.push_back(((first & 0x1F) << 6) | (str[i + 1] & 0x3F));
                        i += 2;
                    }
                    else
                    {
                        BasicLog::out(BasicLog::kWarn, "Something goes wrong when translating utf-8 string to unicode.");
                        break;
                    }
                }
                else if ((first & 0xF0) == 0xE0)
                {
                    if (str.length() > i + 2)
                    {
                        codes.push_back(((first & 0x0F) << 12) | ((str[i + 1] & 0x3F) << 6) | (str[i + 2] & 0x3F));
                        i += 3;
                    }
                    else
                    {
                        BasicLog::out(BasicLog::kWarn, "Something goes wrong when translating utf-8 string to unicode.");
                        break;
                    }
                }
                else if ((first & 0xF8) == 0xF0)
                {
                    if (str.length() > i + 3)
                    {
                        codes.push_back(((first & 0x07) << 18) | ((str[i + 1] & 0x3F) << 12) | ((str[i + 2] & 0x3F) << 6) | (str[i + 3] & 0x3F));
                        i += 4;
                    }
                    else
                    {
                        BasicLog::out(BasicLog::kWarn, "Something goes wrong when translating utf-8 string to unicode.");
                        break;
                    }
                }
                else
                {
                    BasicLog::out(BasicLog::kWarn, "Something goes wrong when translating utf-8 string to unicode.");
                    break;
                }
            }

            return codes;
        }

        inline std::vector<int> collectNew(const std::unordered_map<int, vec2i>& textMap, const std::vector<int>& codes)
        {
            std::vector<int> ret;

            if (codes.empty())
            {
                return ret;
            }

            std::set<int> texts;
            for (auto i : codes)
            {
                if (i != '\n' && textMap.count(i) == 0)
                {
                    texts.insert(i);
                }
            }

            ret = std::vector<int>(texts.begin(), texts.end());
            return ret;
        }

        // 修改自stbtt_BakeFontBitmap
        int bakeFontBitmap(const stbtt_fontinfo& info, int fontSize, unsigned char* pImage, int width, int height,
            const int* pCharacter, int num, std::vector<stbtt_bakedchar>& textData)
        {
            assert(pImage != nullptr);
            assert(pCharacter != nullptr);
            assert(fontSize > 0);
            assert(width > 0);
            assert(height > 0);
            assert(num > 0);

            int x = 1;
            int y = 1;
            int bottom_y = 1;
            memset(pImage, 0, width * height * sizeof(unsigned char));

            float scale = stbtt_ScaleForPixelHeight(&info, (float)fontSize);

            for (int i = 0; i < num; i++)
            {
                int g = stbtt_FindGlyphIndex(&info, pCharacter[i]);
                int advance = 0;
                int lsb = 0;
                stbtt_GetGlyphHMetrics(&info, g, &advance, &lsb);
                int x0 = 0;
                int y0 = 0;
                int x1 = 0;
                int y1 = 0;
                stbtt_GetGlyphBitmapBox(&info, g, scale, scale, &x0, &y0, &x1, &y1);
                int gw = x1 - x0;
                int gh = y1 - y0;
                if (x + gw + 1 >= width)
                    y = bottom_y, x = 1; // advance to next row
                if (y + gh + 1 >= height) // check if it fits vertically AFTER potentially moving to next row
                    return -i;
                assert(x + gw < width);
                assert(y + gh < height);
                stbtt_MakeGlyphBitmap(&info, pImage + x + y * width, gw, gh, width, scale, scale, g);
                stbtt_bakedchar data;
                data.x0 = (stbtt_int16)x;
                data.y0 = (stbtt_int16)y;
                data.x1 = (stbtt_int16)(x + gw);
                data.y1 = (stbtt_int16)(y + gh);
                data.xadvance = scale * advance;
                data.xoff = (float)x0;
                data.yoff = (float)y0;
                textData.emplace_back(std::move(data));
                x = x + gw + 1;
                if (y + gh + 1 > bottom_y)
                    bottom_y = y + gh + 1;
            }
            return bottom_y;
        }

    }  // namespace

    bool GraphicsFontManager::FontCompare::operator()(const Font& left, const Font& right) const
    {
        return left.name == right.name ? left.size < right.size : left.name < right.name;
    }

    GraphicsFontManager::~GraphicsFontManager()
    {
        for (auto& pair : m_fontFiles)
        {
            assert(pair.second.data != nullptr);
            delete[] pair.second.data;
        }
    }

    GraphicsFontManager& GraphicsFontManager::instance()
    {
        static GraphicsFontManager instance;
        return instance;
    }

    std::string GraphicsFontManager::loadFontFile(const std::string& path)
    {
        std::string fontName;

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
            return fontName;
        }

        int length = 0;
        const char* pName = nullptr;
#ifdef FX_WINDOWS
        // 在Windows中，platform ID为3
        // 随后查找encoding ID，先查找full Unicode(10)，如果没有则查找Unicode BMP(1)
        // 随后确认language ID，这里始终使用English(1033)
        // 最后读取full name(4)字段的值作为字体名称
        pName = stbtt_GetFontNameString(&fontInfo, &length, STBTT_PLATFORM_ID_MICROSOFT, STBTT_MS_EID_UNICODE_FULL,
            STBTT_MS_LANG_ENGLISH, FULL_NAME_ID);

        if (pName == nullptr && length == 0)
        {
            pName = stbtt_GetFontNameString(&fontInfo, &length, STBTT_PLATFORM_ID_MICROSOFT, STBTT_MS_EID_UNICODE_BMP,
                STBTT_MS_LANG_ENGLISH, FULL_NAME_ID);
        }
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
            // 由于前面始终查找English字符集中的字体名称，这里认为pName中一定只有asic字符，因此删除0值即可
            if (pName[i] != '\0')
            {
                fontName.push_back(pName[i]);
            }
        }

        auto itr = m_fontFiles.find(fontName);
        if (itr == m_fontFiles.end())
        {
            m_fontFiles.insert({ fontName, fontInfo });
        }
        else
        {
            delete[] pFileData;    // 如果发现这个字体已经被记录，则立刻释放内存，否则会在析构函数中释放
        }

        return fontName;
    }

    void GraphicsFontManager::generate(const Font& font, const std::string& texts)
    {
        if (texts.empty())
        {
            return;
        }

        if (font.size == 0)
        {
            BasicLog::out(BasicLog::kWarn, "Trying to generating text vertex which height is 0, ignored.");
            return;
        }

        if (m_fontFiles.count(font.name) == 0)
        {
            BasicLog::out(BasicLog::kWarn, "You need to load font file first before generating text vertex.");
            return;
        }

        auto& fontData = m_fontMap[font];

        // 将utf-8字符串的值转换为unicode值
        auto codes = utf8ToUnicode(texts);

        // 收集新出现的字符，并生成数据
        generate(font, collectNew(fontData.textMap, codes));
    }

    FontInfo GraphicsFontManager::queryFont(const Font& font) const
    {
        FontInfo ret;

        // TODO 优化
        auto itr = m_fontMap.find(font);
        if (itr == m_fontMap.end())
        {
            BasicLog::out(BasicLog::kWarn, "Font [", font.name, "] dose not exist or not loaded.");
            return ret;
        }

        ret.pTexture = itr->second.pTexture.get();
        ret.lineGap = itr->second.lineGap;
        ret.ascent = itr->second.ascent;
        return ret;
    }

    TextVertex GraphicsFontManager::queryStringVertex(const Font& font, const std::string& texts)
    {
        TextVertex ret;

        if (texts.empty())
        {
            BasicLog::out(BasicLog::kWarn, "Empty string when generating text vertex, please check input.");
            return ret;
        }

        if (font.size == 0)
        {
            BasicLog::out(BasicLog::kWarn, "Trying to generating text vertex which height is 0, ignored.");
            return ret;
        }

        if (m_fontFiles.count(font.name) == 0)
        {
            BasicLog::out(BasicLog::kWarn, "You need to load font file first before generating text vertex.");
            return ret;
        }

        auto& fontData = m_fontMap[font];

        // 将utf-8字符串的值转换为unicode值
        auto codes = utf8ToUnicode(texts);

        // 收集新出现的字符，并生成数据
        generate(font, collectNew(fontData.textMap, codes));

        ret.vertex.reserve(codes.size() * 12);
        ret.normal.reserve(codes.size() * 12);
        ret.uv.reserve(codes.size() * 12);
        ret.index.reserve(codes.size() * 6);
        vec2f advance = { 0, fontData.ascent };

        // 计算顶点数据，此时所有字符的数据都已生成
        for (auto code : codes)
        {
            // 如果是换行符，更新advance
            if (code == '\n')
            {
                advance.x = 0;
                advance.y += fontData.lineGap;
                continue;
            }

            // 取整，让字符对齐像素
            vec2i offset = { static_cast<int>(advance.x + 0.5f), static_cast<int>(advance.y + 0.5f) };

            auto itr = fontData.textMap.find(code);
            assert(itr != fontData.textMap.end());
            assert(fontData.textArray.size() > itr->second.x);
            assert(fontData.textArray[itr->second.x].size() > itr->second.y);
            assert(fontData.textArray[itr->second.x][itr->second.y].code == code);

            assert(ret.vertex.size() % 12 == 0);
            unsigned int k = static_cast<unsigned int>(ret.vertex.size()) / 3;

            // 索引，总是最后四个点
            ret.index.push_back(k); ret.index.push_back(k + 2); ret.index.push_back(k + 1);
            ret.index.push_back(k + 1); ret.index.push_back(k + 2); ret.index.push_back(k + 3);

            auto& textData = fontData.textArray[itr->second.x][itr->second.y].data;

            // 根据textData计算点坐标
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

            // 法向（无效值）
            ret.normal.push_back(0); ret.normal.push_back(0); ret.normal.push_back(-1);
            ret.normal.push_back(0); ret.normal.push_back(0); ret.normal.push_back(-1);
            ret.normal.push_back(0); ret.normal.push_back(0); ret.normal.push_back(-1);
            ret.normal.push_back(0); ret.normal.push_back(0); ret.normal.push_back(-1);

            // 根据textData计算纹理坐标
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

    void GraphicsFontManager::generate(const Font& font, const std::vector<int>& codes)
    {
        if (codes.empty())
        {
            return;
        }

        auto itr = m_fontFiles.find(font.name);
        assert(itr != m_fontFiles.end());

        auto& fontInfo = itr->second;
        auto& fontData = m_fontMap[font];

        // 如果是该字体是第一次被创建，添加一个初始的image，并计算lineGap
        if (fontData.textMap.empty())
        {
            assert(fontData.pTexture == nullptr);
            fontData.pTexture.reset(new GraphicsTexture());
            fontData.pTexture->setLinearFilter(false);
            fontData.pTexture->setUseMipmap(false);
            fontData.pTexture->setTransparent(true);

            BasicImage image;
            image.setData(TEXT_TEXTURE_SIZE, TEXT_TEXTURE_SIZE, 1, nullptr);
            fontData.pTexture->pushImage(image);
            fontData.lastRow = 0;    // lastRow = 0标明是新创建的image

            fontData.textArray.emplace_back(std::vector<TextData>());

            float scale = stbtt_ScaleForPixelHeight(&fontInfo, (float)font.size);
            int ascent = 0;
            int descent = 0;
            int lineGap = 0;
            stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);
            fontData.lineGap = (lineGap + ascent - descent) * scale;
            fontData.ascent = ascent * scale;
        }

#if defined(_DEBUG) || defined(DEBUG)
        for (auto i : codes)
        {
            assert(fontData.textMap.count(i) == 0);
        }
#endif

        // 遍历生成所有新字符的texture，i为已经生成的数量
        for (int i = 0; i < codes.size();)
        {
            // n为还未处理的字符的个数
            int n = static_cast<int>(codes.size() - i);

            // 总是拿取最后一张image继续生成
            // 最后一张image可能是刚刚创建的空的image，也有可能是上次填充了一半的image，根据lastRow区分
            assert(fontData.pTexture != nullptr);
            auto& imageList = fontData.pTexture->imageList();
            assert(imageList.empty() == false);
            auto pImage = imageList.back();
            assert(pImage);

            std::vector<stbtt_bakedchar> textData;

            auto ret = bakeFontBitmap(fontInfo, font.size, pImage + fontData.lastRow * TEXT_TEXTURE_SIZE,
                TEXT_TEXTURE_SIZE, TEXT_TEXTURE_SIZE - fontData.lastRow,    // 注意根据lastRow偏移指针并计算height
                codes.data() + i, n, textData);

            // 新生成的像素直接根据指针写入texture，这里只需要mark dirty
            fontData.pTexture->markImageDirty(static_cast<int>(imageList.size() - 1));

            // 根据返回值判断剩余字符是否已经全部生成（当前image是否足够存放剩余字符）
            // m表示这一轮实际生成的字符的数量
            int m = ret > 0 ? n : -ret;
            assert(m <= n);
            assert(m == textData.size());

            assert(fontData.textArray.empty() == false);
            auto& list = fontData.textArray.back();
            auto& map = fontData.textMap;

            // 将这一轮生成的m个字符的数据存入数组
            for (int k = 0; k < textData.size(); k++)
            {
                textData[k].y0 += fontData.lastRow;
                textData[k].y1 += fontData.lastRow;
                int code = codes[i + k];
                list.emplace_back(TextData{ code, textData[k] });
                map.insert({ code, { static_cast<int>(imageList.size() - 1), static_cast<int>(list.size() - 1) } });
            }

            if (ret > 0)    // ret > 0表示这一轮已经生成全部字符
            {
                fontData.lastRow += ret;    // 更新lastRow
                break;
            }
            else
            {
                // 还没有生成全部字符
                i += m;
                BasicImage image;
                image.setData(TEXT_TEXTURE_SIZE, TEXT_TEXTURE_SIZE, 1, nullptr);
                fontData.pTexture->pushImage(image);
                fontData.lastRow = 0;    // lastRow = 0标明是新创建的image
                fontData.textArray.emplace_back(std::vector<TextData>());
            }
        }
    }

} // namespace FX
