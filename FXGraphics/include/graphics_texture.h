#ifndef _GRAPHICS_TEXTURE_H_
#define _GRAPHICS_TEXTURE_H_

#include <vector>
#include <set>
#include <assert.h>

#include "basic_image.h"
#include "graphics_gpu_item.h"

namespace FX {

    using TextureUnit = unsigned char;
    constexpr TextureUnit TextTextureUnit = 6;

    class GraphicsTextureBase : public GraphicsGPUItem {
    public:
        enum class Format : unsigned int {
            kR = 0x1903,
            kRGB = 0x1907,
            kRGBA = 0x1908,
            kDepth = 0x1902,
            kDepthStencil = 0x84F9
        };

        unsigned int width(void) const;
        unsigned int height(void) const;
        Format format(void) const;

        void setSoftFilter(bool soft);
        void setUseMipmap(bool use);
        void setTransparent(bool transparent);
        bool softFilter(void) const;
        bool useMipmap(void) const;
        bool isTransparent(void) const;

        virtual unsigned int depth(void) const = 0;
        virtual unsigned int dataType(void) const = 0;
        virtual std::vector<void*> imageList(void) const = 0;

    protected:
        GraphicsTextureBase(void) : GraphicsGPUItem(GPUItemType::kTexture2dArray) {}
        ItemInfo* create(void) const override;

        template<typename T = unsigned char>
        struct ImagePtr {
            T* pData = nullptr;
            bool reference = false;

            ImagePtr(const BasicImage<T>& image)
            {
                assert(image.valid());
                if (image.isReference())
                {
                    pData = const_cast<T*>(image.data());
                    reference = true;
                }
                else
                {
                    pData = new T[image.width() * image.height() * image.channels()];
                    memcpy(pData, image.data(), image.width() * image.height() * image.channels() * sizeof(T));
                    reference = false;
                }
            }

            ~ImagePtr()
            {
                if (reference == false && pData != nullptr)
                {
                    delete[] pData;
                }
            }

            ImagePtr(ImagePtr&& other)
            {
                this->pData = other.pData;
                this->reference = other.reference;
                other.pData = nullptr;
            }

            ImagePtr& operator=(ImagePtr&& other)
            {
                if (this == &other)
                {
                    return *this;
                }

                if (reference == false && pData != nullptr)
                {
                    delete[] pData;
                }

                this->pData = other.pData;
                this->reference = other.reference;
                other.pData = nullptr;
                return *this;
            }

            ImagePtr(const ImagePtr& other) = delete;
            ImagePtr& operator=(const ImagePtr& other) = delete;
        };

    protected:
        unsigned int m_width = 0;
        unsigned int m_height = 0;
        Format m_format = Format::kRGBA;
        bool m_useMipmap = true;
        bool m_softFilter = true;
        bool m_transparent = false;
    };


    class GraphicsTexture : public GraphicsTextureBase {
    public:
        GraphicsTexture(void) = default;

        void pushImage(const BasicImage<>& image);
        void popImage(void);
        void setImage(const BasicImage<>& image, unsigned int index);
        void markImageDirty(unsigned int index);

        unsigned int depth(void) const override;
        unsigned int dataType(void) const override;
        std::vector<void*> imageList(void) const override;

    protected:
        std::vector<ImagePtr<>> m_imageList;
    };


    class TextureInfo : public ItemInfo {
    protected:
        friend class GraphicsTextureBase;

        explicit TextureInfo(const GraphicsTextureBase* pOwner);
        ~TextureInfo(void) override;

    public:
        void bind(TextureUnit unit = 0);

        void markParamDirty(void);
        void markImageDirty(unsigned int index);
        void eraseImageDirty(unsigned int index);

    protected:
        void updateParam(void) const;

    protected:
        std::set<unsigned int, std::less<unsigned int>> m_dirtyList;
        unsigned int m_textureDepth = 0;
        unsigned int m_dataDepth = 0;
        bool m_paramDirty = true;
    };

} // namespace FX

#endif // _GRAPHICS_TEXTURE_H_
