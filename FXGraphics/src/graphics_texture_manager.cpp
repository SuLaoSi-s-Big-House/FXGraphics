#include "graphics_texture_manager.h"

#include <assert.h>
#include <string.h>
#include "basic_log.h"
#include "basic_hash.h"
#include "graphics_texture.h"

namespace FX {

    namespace {
        
        static constexpr unsigned int TEXTURE_LEVEL_SIZE[TextureLevelNum] = { 64, 256, 512, 1024 };

        inline unsigned char calculateLevel(unsigned int width, unsigned int height)
        {
            assert(width > 0 && height > 0);
            assert(width <= TEXTURE_LEVEL_SIZE[TextureLevelNum - 1] && height <= TEXTURE_LEVEL_SIZE[TextureLevelNum - 1]);
            auto size = std::max(width, height);
            unsigned char i = 0;
            while (i < TextureLevelNum && size > TEXTURE_LEVEL_SIZE[i])
            {
                i++;
            }
            return i;
        }

        inline bool isSameImage(const BasicImage<>& left, const BasicImage<>& right, unsigned int dataSize)
        {
            assert(left.valid() && right.valid());
            return (left.width() == right.width() && left.height() == right.height() && left.channels() == right.channels() &&
                (memcmp(left.data(), right.data(), dataSize) == 0));
        }

    }  // namespace

    GraphicsTextureManager::GraphicsTextureManager(void)
    {
        m_textureEntries.emplace_back();
        m_imagePool.emplace_back();
    }

    GraphicsTextureManager& GraphicsTextureManager::instance()
    {
        static GraphicsTextureManager instance;
        return instance;
    }

    TextureHandle GraphicsTextureManager::allocTextureHandle(const TextureEntry& entry)
    {
        if (m_textureFreeList.empty())
        {
            m_textureEntries.push_back(entry);
            return static_cast<TextureHandle>(m_textureEntries.size() - 1);
        }

        TextureHandle handle = m_textureFreeList.back();
        m_textureFreeList.pop_back();
        m_textureEntries[handle] = entry;
        return handle;
    }

    void GraphicsTextureManager::freeTextureHandle(TextureHandle handle)
    {
        assert(handle > 0 && handle < m_textureEntries.size());
        m_textureFreeList.push_back(handle);
    }

    ImageHandle GraphicsTextureManager::allocImageHandle(const ImageEntry& entry)
    {
        if (m_imageFreeList.empty())
        {
            m_imagePool.push_back(entry);
            return static_cast<ImageHandle>(m_imagePool.size() - 1);
        }

        ImageHandle handle = m_imageFreeList.back();
        m_imageFreeList.pop_back();
        m_imagePool[handle] = entry;
        return handle;
    }

    void GraphicsTextureManager::freeImageHandle(ImageHandle handle)
    {
        assert(handle > 0 && handle < m_imagePool.size());
        m_imageFreeList.push_back(handle);
    }

    std::pair<TextureHandle, ImageHandle> GraphicsTextureManager::addImage(TextureSlot slot, const BasicImage<>& image)
    {
        std::pair<TextureHandle, ImageHandle> ret = { InvalidHandle, InvalidHandle };

        if (slot >= TextureKey::TextureSlotNum)
        {
            BasicLog::out(BasicLog::kWarn, "Invalid texture slot when setting image to texture manager.");
            return ret;
        }

        if (image.valid() == false)
        {
            BasicLog::out(BasicLog::kWarn, "Trying to add an invalid image to texture manager, discard.");
            return ret;
        }
        
        auto width = image.width();
        auto height = image.height();
        auto channels = image.channels();

        // 当前限制image大小不得超过1024像素，不得超过4个通道
        if (width > TEXTURE_LEVEL_SIZE[TextureLevelNum - 1] || height > TEXTURE_LEVEL_SIZE[TextureLevelNum - 1] || channels > ImageChannelNum)
        {
            BasicLog::out(BasicLog::kWarn, "Image is too large or has too many channels, not supported yet.");
            return ret;
        }

        auto level = calculateLevel(width, height);
        auto dataSize = width * height * channels * sizeof(unsigned char);
        auto hash = xxHash64(image.data(), dataSize);

        auto& imageMap = m_imageMap[level][channels];
        auto itr = imageMap.find(hash);
        if (itr != imageMap.end())
        {
            auto imageHandle = itr->second;
            assert(imageHandle != InvalidHandle);
            assert(m_imagePool.size() > imageHandle);
            auto& storage = m_imagePool[imageHandle];
            assert(storage.refNum > 0);

            if (isSameImage(image, storage.image, dataSize))
            {
                storage.refNum++;
                ret = { storage.textureHandle, imageHandle };
                return ret;
            }
        }



        unsigned int tierSize = TierSizes[tier];
        BasicImage<unsigned char> paddedImage;
        if (image.width() != tierSize || image.height() != tierSize)
        {
            unsigned int paddedSize = tierSize * tierSize * channels;
            unsigned char* paddedData = new unsigned char[paddedSize];
            memset(paddedData, 0, paddedSize);

            unsigned int srcRowBytes = image.width() * channels;
            unsigned int dstRowBytes = tierSize * channels;

            for (unsigned int row = 0; row < image.height(); row++)
            {
                memcpy(paddedData + row * dstRowBytes, image.data() + row * srcRowBytes, srcRowBytes);
            }

            paddedImage.setData(tierSize, tierSize, channels, paddedData, false);
            delete[] paddedData;
        }
        else
        {
            paddedImage.setData(image.width(), image.height(), channels, image.data(), false);
        }

        TextureGroup& group = m_groups[slot][tier][channels];
        unsigned int arrayIndex = 0;
        unsigned int layer = 0;

        if (!group.freeSlots.empty())
        {
            unsigned int packed = group.freeSlots.back();
            group.freeSlots.pop_back();
            arrayIndex = packed >> 12;
            layer = packed & 0xFFF;

            group.arrays[arrayIndex]->setImage(paddedImage, layer);
        }
        else
        {
            if (group.arrays.empty())
            {
                auto tex = new GraphicsTexture();
                tex->setSoftFilter(true);
                tex->setUseMipmap(true);
                group.arrays.push_back(tex);
            }

            GraphicsTexture* lastArray = group.arrays.back();
            if (lastArray->depth() >= MaxLayersPerArray)
            {
                auto tex = new GraphicsTexture();
                tex->setSoftFilter(true);
                tex->setUseMipmap(true);
                group.arrays.push_back(tex);
                lastArray = tex;
            }

            arrayIndex = static_cast<unsigned int>(group.arrays.size() - 1);
            layer = lastArray->depth();
            lastArray->pushImage(paddedImage);
        }

        TextureEntry texEntry;
        texEntry.slot = slot;
        texEntry.tier = tier;
        texEntry.channels = channels;
        texEntry.arrayIndex = arrayIndex;
        texEntry.layer = layer;
        TextureHandle texHandle = allocTextureHandle(texEntry);

        ImageEntry imgEntry;
        imgEntry.image.setData(image.width(), image.height(), channels, image.data(), false);
        imgEntry.hash = hash;
        imgEntry.refCount = 1;
        imgEntry.tier = tier;
        imgEntry.texHandle = texHandle;
        ImageHandle imgHandle = allocImageHandle(imgEntry);

        m_hashToHandle[tier][channels][hash] = imgHandle;

        ret.first = texHandle;
        ret.second = imgHandle;
        return ret;
    }

    bool GraphicsTextureManager::ref(ImageHandle handle)
    {
        if (handle == InvalidHandle || handle >= m_imagePool.size())
        {
            return false;
        }

        ImageEntry& entry = m_imagePool[handle];
        if (entry.refCount == 0)
        {
            return false;
        }

        entry.refCount++;
        return true;
    }

    bool GraphicsTextureManager::unref(ImageHandle handle)
    {
        if (handle == InvalidHandle || handle >= m_imagePool.size())
        {
            return false;
        }

        ImageEntry& entry = m_imagePool[handle];
        if (entry.refCount == 0)
        {
            return false;
        }

        entry.refCount--;

        if (entry.refCount == 0)
        {
            unsigned char channels = entry.image.channels();
            m_hashToHandle[entry.tier][channels].erase(entry.hash);

            TextureHandle texHandle = entry.texHandle;
            if (texHandle != InvalidHandle && texHandle < m_textureEntries.size())
            {
                TextureEntry& texEntry = m_textureEntries[texHandle];
                unsigned int packed = (texEntry.arrayIndex << 12) | (texEntry.layer & 0xFFF);
                m_groups[texEntry.slot][texEntry.tier][texEntry.channels].freeSlots.push_back(packed);
                freeTextureHandle(texHandle);
            }

            freeImageHandle(handle);
        }

        return true;
    }

    bool GraphicsTextureManager::queryImageMatch(ImageHandle handle, unsigned int width, unsigned int height, unsigned char channels) const
    {
        if (handle == InvalidHandle || handle >= m_imagePool.size())
        {
            return false;
        }

        const ImageEntry& entry = m_imagePool[handle];
        return entry.image.width() == width && entry.image.height() == height && entry.image.channels() == channels;
    }

} // namespace FX
