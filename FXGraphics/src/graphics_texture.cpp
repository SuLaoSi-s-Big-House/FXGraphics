#include "graphics_texture.h"

#include "glad.h"
#include "basic_log.h"

namespace FX {

    namespace {

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

        constexpr inline unsigned int internal(GraphicsTexture::Format format, unsigned int dataType)
        {
            if (dataType == GL_UNSIGNED_BYTE)
            {
                switch (format)
                {
                    case GraphicsTexture::Format::kR: return GL_R8;
                    case GraphicsTexture::Format::kRGB: return GL_RGB8;
                    case GraphicsTexture::Format::kRGBA: return GL_RGBA8;
                    default: return GL_RGBA8;
                }
            }

            assert(0);
            return GL_RGBA8;
        }

    }  // namespace

    unsigned int GraphicsTextureBase::width() const
    {
        return m_width;
    }

    unsigned int GraphicsTextureBase::height() const
    {
        return m_height;
    }

    GraphicsTextureBase::Format GraphicsTextureBase::format() const
    {
        return m_format;
    }

    void GraphicsTextureBase::setSoftFilter(bool soft)
    {
        if (m_softFilter != soft)
        {
            m_softFilter = soft;

            for (auto& itr : m_itemList)
            {
                static_cast<TextureInfo*>(itr.second)->markParamDirty();
            }
        }
    }

    void GraphicsTextureBase::setUseMipmap(bool use)
    {
        if (m_useMipmap != use)
        {
            m_useMipmap = use;

            for (auto& itr : m_itemList)
            {
                static_cast<TextureInfo*>(itr.second)->markParamDirty();
            }
        }
    }

    void GraphicsTextureBase::setTransparent(bool transparent)
    {
        m_transparent = transparent;
    }

    bool GraphicsTextureBase::softFilter() const
    {
        return m_softFilter;
    }

    bool GraphicsTextureBase::useMipmap() const
    {
        return m_useMipmap;
    }

    bool GraphicsTextureBase::isTransparent() const
    {
        return m_transparent;
    }

    ItemInfo* GraphicsTextureBase::create() const
    {
        auto pTexture = new TextureInfo(this);
        for (unsigned int i = 0; i < depth(); i++)
        {
            pTexture->markImageDirty(i);
        }
        return pTexture;
    }

    void GraphicsTexture::pushImage(const BasicImage<>& image)
    {
        if (image.valid() == false)
        {
            BasicLog::out(BasicLog::kWarn, "Tring to add an invalid image to texture, discard.");
            return;
        }
        assert(image.data() != nullptr);

        if (m_imageList.empty() == false && m_width > 0 && m_height > 0)
        {
            if (m_width == image.width() && m_height == image.height() && translate(m_format) == image.channels())
            {
                markImageDirty(static_cast<unsigned int>(m_imageList.size()));
                m_imageList.emplace_back(ImagePtr<>(image));
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
            m_imageList.emplace_back(ImagePtr<>(image));
            markImageDirty(0);
        }
    }

    void GraphicsTexture::popImage()
    {
        if (m_imageList.empty() == false)
        {
            m_imageList.pop_back();

            for (auto& itr : m_itemList)
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

    void GraphicsTexture::setImage(const BasicImage<>& image, unsigned int index)
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

        if (m_width == image.width() && m_height == image.height() && translate(m_format) == image.channels())
        {
            m_imageList[index] = ImagePtr<>(image);
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

        for (auto& itr : m_itemList)
        {
            static_cast<TextureInfo*>(itr.second)->markImageDirty(index);
        }
    }

    unsigned int GraphicsTexture::depth() const
    {
        return static_cast<unsigned int>(m_imageList.size());
    }

    unsigned int GraphicsTexture::dataType() const
    {
        return GL_UNSIGNED_BYTE;
    }

    std::vector<void*> GraphicsTexture::imageList() const
    {
        std::vector<void*> ret;
        ret.resize(m_imageList.size());
        for (int i = 0; i < m_imageList.size(); i++)
        {
            ret[i] = m_imageList[i].pData;
        }
        return ret;
    }

    TextureInfo::TextureInfo(const GraphicsTextureBase* pOwner)
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
        glBindTexture((GLenum)m_pOwner->type(), m_handle);

        if (m_dirtyList.empty())
        {
            if (m_paramDirty)
            {
                updateParam();
                m_paramDirty = false;
            }
            return;
        }

        auto pOwner = static_cast<const GraphicsTextureBase*>(m_pOwner);
        assert(pOwner);
        auto target = static_cast<GLenum>(m_pOwner->type());
        auto imageList = pOwner->imageList();
        assert(imageList.empty() == false);
        auto max = *m_dirtyList.rbegin();

        // 从未创建过texture
        if (m_textureDepth == 0)
        {
            glTexStorage3D(target, 1, internal(pOwner->format(), pOwner->dataType()), pOwner->width(), pOwner->height(), pOwner->depth());

            for (int i = 0; i < imageList.size(); i++)
            {
                glTexSubImage3D(target, 0, 0, 0, i, pOwner->width(), pOwner->height(), 1, (GLenum)pOwner->format(),
                    pOwner->dataType(), imageList[i]);
            }

            updateParam();

            m_textureDepth = m_dataDepth = pOwner->depth();
        }
        else if (max + 1 > m_textureDepth)    // 需要扩容和拷贝
        {
            unsigned int newTexture;
            glGenTextures(1, &newTexture);
            glBindTexture(target, newTexture);

            auto depth = m_textureDepth > 30 ? max + 6 : max + 3;
            glTexStorage3D(target, 1, internal(pOwner->format(), pOwner->dataType()), pOwner->width(), pOwner->height(), depth);

            for (unsigned int i = 0; i < m_dataDepth; i++)
            {
                if (m_dirtyList.count(i) == 0)
                {
                    glCopyImageSubData(m_handle, target, 0, 0, 0, i, newTexture, target, 0, 0, 0, i, pOwner->width(), pOwner->height(), 1);
                }
            }

            for (auto i : m_dirtyList)
            {
                glTexSubImage3D(target, 0, 0, 0, i, pOwner->width(), pOwner->height(), 1, (GLenum)pOwner->format(),
                    pOwner->dataType(), imageList[i]);
            }

            glDeleteTextures(1, &m_handle);

            updateParam();

            m_handle = newTexture;
            m_textureDepth = depth;
            m_dataDepth = max + 1;
        }
        else    // 不需要扩容，直接更新
        {
            for (auto i : m_dirtyList)
            {
                glTexSubImage3D(target, 0, 0, 0, i, pOwner->width(), pOwner->height(), 1, (GLenum)pOwner->format(),
                    pOwner->dataType(), imageList[i]);
            }

            if (m_paramDirty)
            {
                updateParam();
            }

            m_dataDepth = max + 1;
        }

        if (pOwner->useMipmap())
        {
            glGenerateMipmap(target);
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
        assert(index < static_cast<const GraphicsTextureBase*>(m_pOwner)->depth());
        m_dirtyList.insert(index);
    }

    void TextureInfo::eraseImageDirty(unsigned int index)
    {
        m_dirtyList.erase(index);
    }

    void TextureInfo::updateParam() const
    {
        assert(m_paramDirty);
        auto pOwner = static_cast<const GraphicsTextureBase*>(m_pOwner);
        auto target = static_cast<GLenum>(m_pOwner->type());

        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);

        auto maxFilter = pOwner->softFilter() ? GL_LINEAR : GL_NEAREST;
        auto minFilter = pOwner->softFilter() ? pOwner->useMipmap() ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR :
            pOwner->useMipmap() ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;

        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, maxFilter);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
    }

} // namespace FX
