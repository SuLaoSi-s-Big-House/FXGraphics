#include "graphics_texture_manager.h"

#include <assert.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "basic_log.h"
#include "graphics_window.h"

namespace FX {

    namespace {

        constexpr unsigned int TEXTURE_LEVEL_NUM = 4;
        constexpr unsigned int TEXTURE_LEVEL_SIZE[TEXTURE_LEVEL_NUM] = { 64, 256, 512, 1024 };
        constexpr unsigned int MAX_TEXTURE_DEPTH[TEXTURE_LEVEL_NUM] = { 1024, 1024, 1024, 256 };

        inline unsigned char calculateLevel(unsigned int width, unsigned int height)
        {
            assert(width > 0 && height > 0);
            assert(width <= TEXTURE_LEVEL_SIZE[TEXTURE_LEVEL_NUM - 1] && height <= TEXTURE_LEVEL_SIZE[TEXTURE_LEVEL_NUM - 1]);
            auto size = std::max(width, height);
            unsigned char i = 0;
            while (i < TEXTURE_LEVEL_NUM && size > TEXTURE_LEVEL_SIZE[i])
            {
                i++;
            }
            return i;
        }

        // 通道数转texture格式，调用前需要保证channels为1/3/4
        //constexpr GraphicsTextureBase::Format translate(unsigned char channels)
        //{
        //    switch (channels)
        //    {
        //        case 1: return GraphicsTextureBase::Format::kR;
        //        case 3: return GraphicsTextureBase::Format::kRGB;
        //        case 4: return GraphicsTextureBase::Format::kRGBA;
        //        default: return GraphicsTextureBase::Format::kRGBA;
        //    }
        //}

        inline void fillImage(const BasicImage<>& src, unsigned int size, BasicImage<>& dst)
        {
            assert(src.valid());
            assert(src.width() <= size && src.height() <= size);

            dst.setData(size, size, src.channels(), nullptr);

            auto ch = static_cast<unsigned int>(src.channels());
            auto srcWidth = src.width();
            auto srcHeight = src.height();
            auto pDst = const_cast<unsigned char*>(dst.data());
            auto pSrc = src.data();

            for (unsigned int y = 0; y < srcHeight; y++)
            {
                auto pDstRow = pDst + y * size * ch;
                auto pSrcRow = pSrc + y * srcWidth * ch;

                memcpy(pDstRow, pSrcRow, srcWidth * ch);

                for (unsigned int x = srcWidth; x < size; x++)
                {
                    memcpy(pDstRow + x * ch, pSrcRow + (srcWidth - 1) * ch, ch);
                }
            }

            for (unsigned int y = srcHeight; y < size; y++)
            {
                memcpy(pDst + y * size * ch, pDst + (srcHeight - 1) * size * ch, size * ch);
            }
        }
    }  // namespace

    GraphicsTextureManager& GraphicsTextureManager::instance()
    {
        static GraphicsTextureManager instance;
        return instance;
    }

    ImageHandle GraphicsTextureManager::registerImage(const BasicImage<>& image)
    {
        if (image.valid() == false)
        {
            BasicLog::out(BasicLog::kWarn, "Trying to register an invalid image, discard.");
            return InvalidHandle;
        }

        auto width = image.width();
        auto height = image.height();

        if (width > TEXTURE_LEVEL_SIZE[TEXTURE_LEVEL_NUM - 1] || height > TEXTURE_LEVEL_SIZE[TEXTURE_LEVEL_NUM - 1])
        {
            BasicLog::out(BasicLog::kWarn, "Image is too large, not supported yet.");
            return InvalidHandle;
        }

        auto channels = image.channels();
        if (channels > 4)
        {
            BasicLog::out(BasicLog::kWarn, "Image has too many channels, not supported yet.");
            return InvalidHandle;
        }

        auto level = calculateLevel(width, height);

        // 填充image到标准大小
        BasicImage<> filledImage;
        fillImage(image, TEXTURE_LEVEL_SIZE[level], filledImage);

        // 确定image进入哪个texture
        auto textureHandle = findBestTexture(level, channels);
        assert(m_texturePool.size() > textureHandle);

        auto& textureData = m_texturePool[textureHandle];
        assert(textureData.pTexture != nullptr);
        unsigned int slice = 0;
        
        if (textureData.freeList.empty() == false)
        {
            slice = textureData.freeList.back();
            textureData.freeList.pop_back();
            assert(textureData.pTexture->depth() > slice);
            textureData.pTexture->setImage(filledImage, slice);
            // TODO texture中reference可以为true
        }
        else
        {
            textureData.pTexture->pushImage(filledImage);
            slice = textureData.pTexture->depth() - 1;
        }

        // texture已完成，image入池
        vec2f scale = {
            static_cast<float>(width) / TEXTURE_LEVEL_SIZE[level],
            static_cast<float>(height) / TEXTURE_LEVEL_SIZE[level]
        };

        ImageHandle imageHandle = InvalidHandle;
        if (m_freeList.empty() == false)
        {
            imageHandle = m_freeList.back();
            m_freeList.pop_back();
            assert(m_imagePool.size() > imageHandle);
            assert(m_imagePool[imageHandle].pImage == nullptr);
            m_imagePool[imageHandle].pImage.reset(new BasicImage<>(image));
            m_imagePool[imageHandle].texture = textureHandle;
            m_imagePool[imageHandle].slice = slice;
            m_imagePool[imageHandle].scale = scale;
        }
        else
        {
            imageHandle = static_cast<ImageHandle>(m_imagePool.size());
            m_imagePool.emplace_back(ImageData{ std::make_unique<BasicImage<>>(image), textureHandle, slice, scale });
        }

        return imageHandle;
    }

    ImageHandle GraphicsTextureManager::registerImage(const std::string& path)
    {
        if (path.empty())
        {
            BasicLog::out(BasicLog::kWarn, "Invalid file path when registering image.");
            return InvalidHandle;
        }

        int width = 0;
        int height = 0;
        int channels = 0;
        if (stbi_info(path.c_str(), &width, &height, &channels) == 0)
        {
            BasicLog::out(BasicLog::kWarn, "Invalid image file when registering image.");
            return InvalidHandle;
        }

        auto pData = stbi_load(path.c_str(), &width, &height, &channels, 0);
        if (pData == nullptr)
        {
            BasicLog::out(BasicLog::kWarn, "Cannot load the image file, please check the file.");
            return InvalidHandle;
        }

        BasicImage<> image;
        image.setData(static_cast<unsigned int>(width), static_cast<unsigned int>(height), static_cast<unsigned char>(channels), pData);
        stbi_image_free(pData);

        return registerImage(image);
    }

    std::vector<ImageHandle> GraphicsTextureManager::registerImage(const std::vector<std::string>& paths)
    {
        std::vector<ImageHandle> ret;

        if (paths.empty())
        {
            return ret;
        }

        ret.reserve(paths.size());

        for (auto& path : paths)
        {
            ret.push_back(registerImage(path));
        }

        return ret;
    }

    bool GraphicsTextureManager::unregisterImage(ImageHandle handle)
    {
        if (valid(handle) == false)
        {
            BasicLog::out(BasicLog::kWarn, "Trying to unregister an invalid image handle, discard.");
            return false;
        }

        assert(m_imagePool.size() > handle);

        auto& imageData = m_imagePool[handle];
        assert(imageData.pImage != nullptr);
        assert(imageData.texture != InvalidHandle);

        auto textureHandle = imageData.texture;
        assert(m_texturePool.size() > textureHandle);

        auto& textureData = m_texturePool[textureHandle];
        assert(textureData.pTexture != nullptr);
        assert(textureData.pTexture->depth() > imageData.slice);
        
        textureData.freeList.push_back(imageData.slice);
        // 这里不会立刻将texture中的图片设置为无效值

        imageData.pImage.reset(nullptr);
        imageData.texture = InvalidHandle;
        imageData.slice = 0;
        imageData.scale = { 1.0f, 1.0f };

        m_freeList.push_back(handle);

        return true;
    }

    bool GraphicsTextureManager::valid(ImageHandle handle) const
    {
        if (handle == InvalidHandle)
        {
            BasicLog::out(BasicLog::kWarn, "Cannot query with invalid handle.");
            return false;
        }

        if (handle >= m_imagePool.size() || m_imagePool[handle].pImage == nullptr)
        {
            //assert(m_imagePool[handle].texture == InvalidHandle);
            return false;
        }

        return true;
    }

    TexturePack GraphicsTextureManager::query(ImageHandle handle) const
    {
        TexturePack ret;

        if (handle == InvalidHandle || handle >= m_imagePool.size() || m_imagePool[handle].pImage == nullptr)
        {
            return ret;
        }

        assert(m_imagePool[handle].texture != InvalidHandle);
        ret.textureHandle = m_imagePool[handle].texture;
        ret.scale = m_imagePool[handle].scale;
        ret.slice = m_imagePool[handle].slice;
        return ret;
    }

    void GraphicsTextureManager::bind(const TextureKey& texture)
    {
        if (GraphicsWindow::currentWindow() == nullptr)
        {
            return;
        }

        auto bindTexture = [this](ImageHandle imageHandle, TextureUnit unit)
        {
            if (imageHandle < m_imagePool.size() && m_imagePool[imageHandle].pImage != nullptr)
            {
                auto textureHandle = m_imagePool[imageHandle].texture;
                assert(textureHandle < m_texturePool.size());
                assert(m_texturePool[textureHandle].pTexture != nullptr);
                auto pTexture = static_cast<TextureInfo*>(m_texturePool[textureHandle].pTexture->getOrCreate());
                assert(pTexture);
                pTexture->bind(unit);
            }
        };

        bindTexture(texture.handle(BaseColorTextureSlot), ColorTextureUnit);
        bindTexture(texture.handle(NormalTextureSlot), NormalTextureUnit);
        bindTexture(texture.handle(ORMTextureSlot), ORMTextureUnit);
    }

    GraphicsTextureManager::GraphicsTextureManager()
    {
        addDefaultImage();
    }

    void GraphicsTextureManager::addDefaultImage()
    {
        constexpr unsigned int DEFAULT_IMAGE_SIZE = 64;

        auto pData = new unsigned char[DEFAULT_IMAGE_SIZE * DEFAULT_IMAGE_SIZE * 4];
        auto pLine = new unsigned char[DEFAULT_IMAGE_SIZE * 4];
        memset(pLine, 0, DEFAULT_IMAGE_SIZE * 4 * sizeof(unsigned char));

        for (unsigned int i = 0; i < DEFAULT_IMAGE_SIZE; i++)
        {
            (i / 16) % 2 == 0 ? pLine[4 * i + 3] = 255 : pLine[4 * i] = pLine[4 * i + 2] = pLine[4 * i + 3] = 255;
        }
        for (unsigned int i = 0; i < DEFAULT_IMAGE_SIZE / 4; i++)
        {
            memcpy(pData + DEFAULT_IMAGE_SIZE * 4 * i, pLine, DEFAULT_IMAGE_SIZE * 4 * sizeof(unsigned char));
        }
        for (unsigned int i = DEFAULT_IMAGE_SIZE / 2; i < 3 * DEFAULT_IMAGE_SIZE / 4; i++)
        {
            memcpy(pData + DEFAULT_IMAGE_SIZE * 4 * i, pLine, DEFAULT_IMAGE_SIZE * 4 * sizeof(unsigned char));
        }

        memset(pLine, 0, DEFAULT_IMAGE_SIZE * 4 * sizeof(unsigned char));

        for (unsigned int i = 0; i < DEFAULT_IMAGE_SIZE; i++)
        {
            (i / 16) % 2 != 0 ? pLine[4 * i + 3] = 255 : pLine[4 * i] = pLine[4 * i + 2] = pLine[4 * i + 3] = 255;
        }
        for (unsigned int i = DEFAULT_IMAGE_SIZE / 4; i < DEFAULT_IMAGE_SIZE / 2; i++)
        {
            memcpy(pData + DEFAULT_IMAGE_SIZE * 4 * i, pLine, DEFAULT_IMAGE_SIZE * 4 * sizeof(unsigned char));
        }
        for (unsigned int i = 3 * DEFAULT_IMAGE_SIZE / 4; i < DEFAULT_IMAGE_SIZE; i++)
        {
            memcpy(pData + DEFAULT_IMAGE_SIZE * 4 * i, pLine, DEFAULT_IMAGE_SIZE * 4 * sizeof(unsigned char));
        }

        m_imagePool.reserve(5);
        m_imagePool.emplace_back(ImageData());
        auto& imageData = m_imagePool[0];
        imageData.pImage.reset(new BasicImage<>);
        imageData.pImage->setData(DEFAULT_IMAGE_SIZE, DEFAULT_IMAGE_SIZE, 4, pData);
        imageData.texture = 0;
        imageData.slice = 0;
        imageData.scale = { 1.0f, 1.0f };

        m_texturePool.reserve(5);
        m_texturePool.emplace_back(TextureData());
        auto& textureData = m_texturePool[0];
        textureData.pTexture.reset(new GraphicsTexture);
        textureData.pTexture->pushImage(*(imageData.pImage.get()));
        
        unsigned long long key = (4ull << 16) + calculateLevel(DEFAULT_IMAGE_SIZE, DEFAULT_IMAGE_SIZE);
        m_textureMap[key].push_back(0);

        delete[] pLine;
        delete[] pData;
    }

    TextureHandle GraphicsTextureManager::findBestTexture(unsigned char level, unsigned char channels)
    {
        assert(level < TEXTURE_LEVEL_NUM);
        assert(channels <= 4);

        unsigned long long key = (static_cast<unsigned long long>(channels) << 16) + level;

        for (auto handle : m_textureMap[key])
        {
            assert(handle != InvalidHandle);
            assert(handle < m_texturePool.size());

            auto& textureData = m_texturePool[handle];
            assert(textureData.pTexture != nullptr);

            if (textureData.freeList.empty() == false || textureData.pTexture->depth() < MAX_TEXTURE_DEPTH[level])
            {
                return handle;
            }
        }

        TextureHandle handle = static_cast<TextureHandle>(m_texturePool.size());
        m_texturePool.emplace_back(TextureData{ std::make_unique<GraphicsTexture>(), {} });
        m_textureMap[key].push_back(handle);

        return handle;
    }

    //ResolvedImage GraphicsTextureManager::resolve(ImageHandle handle, TextureSlot slot)
    //{
    //    if (handle != InvalidHandle)
    //    {
    //        auto index = handle - 1;
    //        if (index < m_handleTable.size() && m_handleTable[index].pTexture != nullptr)
    //        {
    //            auto& entry = m_handleTable[index];

    //            ResolvedImage ret;
    //            ret.pTexture = entry.pTexture;
    //            ret.slice = entry.slice;
    //            ret.scaleX = static_cast<float>(entry.image.width()) / entry.pTexture->width();
    //            ret.scaleY = static_cast<float>(entry.image.height()) / entry.pTexture->height();
    //            ret.hasAlpha = entry.hasAlpha;
    //            return ret;
    //        }
    //    }

    //    // 无效或未设置：使用对应slot的中性fallback
    //    ensureFallbackTexture();

    //    ResolvedImage ret;
    //    ret.pTexture = m_pFallbackTexture.get();
    //    ret.slice = slot;
    //    return ret;
    //}

    //void GraphicsTextureManager::ensureFallbackTexture(void)
    //{
    //    if (m_pFallbackTexture != nullptr)
    //    {
    //        return;
    //    }

    //    m_pFallbackTexture.reset(new GraphicsTexture());
    //    m_pFallbackTexture->setUseMipmap(false);

    //    BasicImage<> image;
    //    unsigned char pixel[4] = { 255, 255, 255, 255 };
    //    image.setData(1, 1, 4, pixel);
    //    m_pFallbackTexture->pushImage(image);

    //    pixel[0] = 128; pixel[1] = 128; pixel[2] = 255;    // 中性法向
    //    image.setData(1, 1, 4, pixel);
    //    m_pFallbackTexture->pushImage(image);

    //    pixel[0] = 255; pixel[1] = 255; pixel[2] = 0;    // 中性ORM
    //    image.setData(1, 1, 4, pixel);
    //    m_pFallbackTexture->pushImage(image);
    //}

    //unsigned int GraphicsTextureManager::allocIndex(void)
    //{
    //    if (m_freeList.empty() == false)
    //    {
    //        auto index = m_freeList.back();
    //        m_freeList.pop_back();
    //        return index;
    //    }

    //    m_handleTable.emplace_back();
    //    return static_cast<unsigned int>(m_handleTable.size() - 1);
    //}

    //void GraphicsTextureManager::releaseIndex(unsigned int index)
    //{
    //    assert(index < m_handleTable.size());

    //    auto& entry = m_handleTable[index];
    //    entry.image = BasicImage<>();
    //    entry.pTexture = nullptr;
    //    entry.slice = 0;

    //    m_freeList.push_back(index);
    //}

} // namespace FX
