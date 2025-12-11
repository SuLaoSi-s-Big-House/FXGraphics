#ifndef _GRAPHICS_TEXTURE_H_
#define _GRAPHICS_TEXTURE_H_

#include <vector>
#include <set>

#include "basic_image.h"
#include "graphics_gpu_item.h"

namespace FX {

    using TextureUnit = unsigned char;
    constexpr TextureUnit TextTextureUnit = 4;

    class GraphicsTexture : public GraphicsGPUItem {
    public:
        enum class Format : unsigned int {
            kR = 0x1903,
            kRGB = 0x1907,
            kRGBA = 0x1908,
            kDepth = 0x1902,
            kDepthStencil = 0x84F9
        };

        GraphicsTexture(void) : GraphicsGPUItem(GPUItemType::kTexture2dArray) {}
        ~GraphicsTexture(void) override;

        void pushImage(const BasicImage& image);
        void popImage(void);
        void setImage(const BasicImage& image, unsigned int index);
        void setImage(unsigned int width, unsigned int height, Format format, const unsigned char* pData, unsigned int index);
        void markImageDirty(unsigned int index);

        unsigned int width(void) const;
        unsigned int height(void) const;
        unsigned int depth(void) const;
        Format format(void) const;

        void setLinearFilter(bool linear);
        void setUseMipmap(bool use);
        void setTransparent(bool isTransparent);
        bool linearFilter(void) const;
        bool useMipmap(void) const;
        bool isTransparent(void) const;

        const std::vector<unsigned char*>& imageList(void) const;

    protected:
        ItemInfo* create(void) const override;

    protected:
        std::vector<unsigned char*> m_imageList;
        unsigned int m_width = 0;
        unsigned int m_height = 0;
        Format m_format = Format::kRGBA;
        bool m_useMipmap = true;
        bool m_linearFilter = true;
        bool m_isTransparent = false;
    };


    class TextureInfo : public ItemInfo {
    protected:
        friend class GraphicsTexture;

        explicit TextureInfo(const GraphicsTexture* pOwner);
        ~TextureInfo(void) override;

    public:
        void bind(TextureUnit unit = 0);

    protected:
        void markParamDirty(void);
        void markImageDirty(unsigned int index);
        void eraseImageDirty(unsigned int index);

    protected:
        std::set<unsigned int, std::less<unsigned int>> m_dirtyList;
        unsigned int m_textureDepth = 0;
        unsigned int m_dataDepth = 0;
        bool m_paramDirty = true;
    };

} // namespace FX

#endif // _GRAPHICS_TEXTURE_H_
