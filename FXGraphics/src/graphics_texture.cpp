#include "graphics_texture.h"

#include <assert.h>
#include "glad.h"
#include "basic_log.h"

namespace FX {

    namespace {

        constexpr inline bool equal(GraphicsTexture::Format format, unsigned char channels)
        {
            return (format == GraphicsTexture::Format::kR && channels == 1) ||
                (format == GraphicsTexture::Format::kRGB && channels == 3) ||
                (format == GraphicsTexture::Format::kRGBA && channels == 4) ||
                (format == GraphicsTexture::Format::kDepth && channels == 1) ||
                (format == GraphicsTexture::Format::kDepthStencil && channels == 1);
        }

        constexpr inline GraphicsTexture::Format translate(unsigned char channels)
        {
            switch (channels)
            {
                case 1: return GraphicsTexture::Format::kR;
                case 3: return GraphicsTexture::Format::kRGB;
                case 4: return GraphicsTexture::Format::kRGBA;
                default: return GraphicsTexture::Format::kRGBA;
            }
        }

        constexpr inline unsigned char translate(GraphicsTexture::Format format)
        {
            switch (format)
            {
                case GraphicsTexture::Format::kR: return 1;
                case GraphicsTexture::Format::kRGB: return 3;
                case GraphicsTexture::Format::kRGBA: return 4;
                case GraphicsTexture::Format::kDepth: return 1;
                case GraphicsTexture::Format::kDepthStencil: return 1;
                default: return 4;
            }
        }

    }  // namespace

    GraphicsTexture::~GraphicsTexture()
    {
        for (auto ptr : m_imageList)
        {
            if (ptr != nullptr)
            {
                delete[] ptr;
            }
        }
    }

    void GraphicsTexture::pushImage(const BasicImage& image)
    {
        if (image.valid() == false)
        {
            BasicLog::out(BasicLog::kWarn, "Tring to add an invalid image to texture, discard.");
            return;
        }
        assert(image.data() != nullptr);

        if (m_imageList.empty() == false && m_width > 0 && m_height > 0)
        {
            if (m_width == image.width() && m_height == image.height() && equal(m_format, image.channels()))
            {
                auto pData = new unsigned char[m_width * m_height * image.channels()];
                memcpy(pData, image.data(), sizeof(unsigned char) * m_width * m_height * image.channels());
                markImageDirty(static_cast<unsigned int>(m_imageList.size()));
                m_imageList.push_back(pData);
            }
            else
            {
                BasicLog::out(BasicLog::kWarn, "Different size in texture and image, cannot add this image.");
            }
        }
        else
        {
            m_width = image.width();
            m_height = image.height();
            m_format = translate(image.channels());
            auto pData = new unsigned char[m_width * m_height * image.channels()];
            memcpy(pData, image.data(), sizeof(unsigned char) * m_width * m_height * image.channels());
            m_imageList.push_back(pData);
            markImageDirty(0);
        }
    }

    void GraphicsTexture::popImage()
    {
        if (m_imageList.empty() == false)
        {
            assert(m_imageList.back() != nullptr);
            delete[] m_imageList.back();
            m_imageList.pop_back();

            for (auto&& itr : m_itemList)
            {
                static_cast<TextureInfo*>(itr.second)->eraseImageDirty(static_cast<unsigned int>(m_imageList.size()));
            }

            if (m_imageList.empty())
            {
                m_width = 0;
                m_height = 0;
                m_format = Format::kRGBA;
            }
        }
    }

    void GraphicsTexture::setImage(const BasicImage& image, unsigned int index)
    {
        if (index >= m_imageList.size())
        {
            BasicLog::out(BasicLog::kWarn, "Out of index, cannot set image.");
            return;
        }

        if (image.valid() == false)
        {
            BasicLog::out(BasicLog::kWarn, "Tring to set an invalid image to texture, discard.");
            return;
        }

        if (m_width == image.width() && m_height == image.height() && equal(m_format, image.channels()))
        {
            memcpy(m_imageList[index], image.data(), sizeof(unsigned char) * m_width * m_height * image.channels());
            markImageDirty(index);
        }
        else
        {
            BasicLog::out(BasicLog::kWarn, "Different size in texture and image, cannot set this image.");
        }
    }

    void GraphicsTexture::setImage(unsigned int width, unsigned int height, Format format, const unsigned char* pData,
        unsigned int index)
    {
        if (index >= m_imageList.size())
        {
            BasicLog::out(BasicLog::kWarn, "Out of index, cannot set image.");
            return;
        }

        if (width == 0 || height == 0 || pData == nullptr)
        {
            BasicLog::out(BasicLog::kWarn, "Tring to set an invalid image to texture, discard.");
            return;
        }

        if (m_width == width && m_height == height && m_format == format)
        {
            memcpy(m_imageList[index], pData, sizeof(unsigned char) * width * height * translate(format));
            markImageDirty(index);
        }
        else
        {
            BasicLog::out(BasicLog::kWarn, "Different size in texture and image, cannot set this image.");
        }
    }

    void GraphicsTexture::markImageDirty(unsigned int index)
    {
        if (index >= m_imageList.size())
        {
            BasicLog::out(BasicLog::kWarn, "Out of index, cannot mark image dirty.");
            return;
        }

        for (auto&& itr : m_itemList)
        {
            static_cast<TextureInfo*>(itr.second)->markImageDirty(index);
        }
    }

    unsigned int GraphicsTexture::width() const
    {
        return m_width;
    }

    unsigned int GraphicsTexture::height() const
    {
        return m_height;
    }

    unsigned int GraphicsTexture::depth() const
    {
        return static_cast<unsigned int>(m_imageList.size());
    }

    GraphicsTexture::Format GraphicsTexture::format() const
    {
        return m_format;
    }

    void GraphicsTexture::setLinearFilter(bool linear)
    {
        if (m_linearFilter != linear)
        {
            m_linearFilter = linear;

            for (auto&& itr : m_itemList)
            {
                static_cast<TextureInfo*>(itr.second)->markParamDirty();
            }
        }
    }

    void GraphicsTexture::setUseMipmap(bool use)
    {
        if (m_useMipmap != use)
        {
            m_useMipmap = use;

            for (auto&& itr : m_itemList)
            {
                static_cast<TextureInfo*>(itr.second)->markParamDirty();
            }
        }
    }

    bool GraphicsTexture::linearFilter() const
    {
        return m_linearFilter;
    }

    bool GraphicsTexture::useMipmap() const
    {
        return m_useMipmap;
    }

    const std::vector<unsigned char*>& GraphicsTexture::imageList() const
    {
        return m_imageList;
    }

    ItemInfo* GraphicsTexture::create() const
    {
        auto pTexture = new TextureInfo(this);
        for (unsigned int i = 0; i < m_imageList.size(); i++)
        {
            pTexture->markImageDirty(i);
        }
        return pTexture;
    }

    TextureInfo::TextureInfo(const GraphicsTexture* pOwner)
        : ItemInfo(pOwner)
    {
        assert(m_type == GPUItemType::kTexture2dArray);
        glGenTextures(1, &m_handle);
        assert(m_handle != 0);
    }

    TextureInfo::~TextureInfo()
    {
        glDeleteTextures(1, &m_handle);
    }

    void TextureInfo::bind(TextureUnit unit)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_handle);

        if (m_dirtyList.empty() && m_paramDirty == false)
        {
            return;
        }

        auto pOwner = static_cast<const GraphicsTexture*>(m_pOwner);
        assert(pOwner);
        auto max = *m_dirtyList.rbegin();

        if (max + 1 > m_textureDepth)
        {
            if (m_textureDepth == 0)
            {
                glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R8, pOwner->width(), pOwner->height(), max + 1);

                auto& imageList = pOwner->imageList();

                for (int i = 0; i < imageList.size(); i++)
                {
                    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, pOwner->width(), pOwner->height(), 1, GL_RED,
                        GL_UNSIGNED_BYTE, imageList[i]);
                }

                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

                auto maxFilter = pOwner->linearFilter() ? GL_LINEAR : GL_NEAREST;
                auto minFilter = pOwner->linearFilter() ? pOwner->useMipmap() ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR :
                    pOwner->useMipmap() ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;

                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, maxFilter);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, minFilter);

                m_textureDepth = m_dataDepth = max + 1;
            }
            else
            {
                unsigned int newTexture;
                glGenTextures(1, &newTexture);
                glBindTexture(GL_TEXTURE_2D_ARRAY, newTexture);

                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

                auto maxFilter = pOwner->linearFilter() ? GL_LINEAR : GL_NEAREST;
                auto minFilter = pOwner->linearFilter() ? pOwner->useMipmap() ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR :
                    pOwner->useMipmap() ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;

                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, maxFilter);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, minFilter);

                int depth = m_textureDepth > 30 ? max + 6 : max + 3;
                glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, (GLenum)pOwner->format(), pOwner->width(), pOwner->height(), depth);

                for (unsigned int i = 0; i < m_dataDepth; i++)
                {
                    glCopyImageSubData(m_handle, GL_TEXTURE_2D_ARRAY, 0, 0, 0, i,
                        newTexture, GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, pOwner->width(), pOwner->height(), 1);
                }

                glDeleteTextures(1, &m_handle);

                m_handle = newTexture;
                m_dataDepth = max + 1;
                m_textureDepth = depth;
            }
        }
        else
        {
            auto& imageList = pOwner->imageList();

            for (auto i : m_dirtyList)
            {
                glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, pOwner->width(), pOwner->height(), 1, (GLenum)pOwner->format(),
                    GL_UNSIGNED_BYTE, imageList[i]);
            }

            if (m_paramDirty)
            {
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

                auto maxFilter = pOwner->linearFilter() ? GL_LINEAR : GL_NEAREST;
                auto minFilter = pOwner->linearFilter() ? pOwner->useMipmap() ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR :
                    pOwner->useMipmap() ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;

                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, maxFilter);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, minFilter);
            }

            m_dataDepth = max + 1;
        }

        if (pOwner->useMipmap())
        {
            glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
        }

        m_dirtyList.clear();
        m_paramDirty = false;
    }

    void TextureInfo::markParamDirty()
    {
        m_paramDirty = true;
    }

    void TextureInfo::markImageDirty(unsigned int index)
    {
        assert(index < static_cast<const GraphicsTexture*>(m_pOwner)->imageList().size());
        m_dirtyList.insert(index);
    }

    void TextureInfo::eraseImageDirty(unsigned int index)
    {
        m_dirtyList.erase(index);
    }

} // namespace FX
