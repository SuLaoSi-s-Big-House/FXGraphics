#include "graphics_texture_manager.h"

#include <assert.h>
#include <string.h>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "basic_log.h"
#include "basic_hash.h"
#include "graphics_texture.h"

namespace FX {

    namespace {

        constexpr unsigned int TEXTURE_LEVEL_SIZE[GraphicsTextureManager::TextureLevelNum] = { 64, 256, 512, 1024 };
        constexpr unsigned int MAX_TEXTURE_DEPTH = 1024;

        inline unsigned char calculateLevel(unsigned int width, unsigned int height)
        {
            assert(width > 0 && height > 0);
            assert(width <= TEXTURE_LEVEL_SIZE[GraphicsTextureManager::TextureLevelNum - 1] && height <= TEXTURE_LEVEL_SIZE[GraphicsTextureManager::TextureLevelNum - 1]);
            auto size = std::max(width, height);
            unsigned char i = 0;
            while (i < GraphicsTextureManager::TextureLevelNum && size > TEXTURE_LEVEL_SIZE[i])
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

        void fillImage(const BasicImage<>& src, BasicImage<>& dst, unsigned int tierSize)
        {
            auto w = src.width();
            auto h = src.height();
            auto c = src.channels();
            auto srcData = static_cast<const unsigned char*>(src.data());
            unsigned int srcRowBytes = w * c;
            unsigned int dstRowBytes = tierSize * c;

            auto totalBytes = tierSize * tierSize * c;
            auto dstBuf = new unsigned char[totalBytes];

            for (unsigned int row = 0; row < h; row++)
            {
                auto pSrcRow = srcData + row * srcRowBytes;
                auto pDstRow = dstBuf + row * dstRowBytes;
                memcpy(pDstRow, pSrcRow, srcRowBytes);
                auto lastPixel = pSrcRow + srcRowBytes - c;
                for (unsigned int x = w; x < tierSize; x++)
                {
                    memcpy(pDstRow + x * c, lastPixel, c);
                }
            }

            auto lastRow = dstBuf + (h - 1) * dstRowBytes;
            for (unsigned int row = h; row < tierSize; row++)
            {
                memcpy(dstBuf + row * dstRowBytes, lastRow, dstRowBytes);
            }

            dst.setData(tierSize, tierSize, c, dstBuf, false);
            delete[] dstBuf;
        }

        struct TextureTask {
            BasicImage<> image;
            TextureHandle textureHandle = InvalidHandle;
            unsigned int layer = 0;
        };

        std::deque<TextureTask> taskQueue;
        std::mutex mutex;
        std::thread worker;
        std::condition_variable workCv;
        //std::condition_variable doneCv;
        //bool busy = false;
        bool stop = false;

    }  // namespace
    

    class TextureWorker {
    public:
        TextureWorker()
        {
            worker = std::thread(&TextureWorker::threadFunc, this);
        }

        ~TextureWorker()
        {
            {
                std::lock_guard<std::mutex> lock(mutex);
                stop = true;
            }

            workCv.notify_one();
            if (worker.joinable())
            {
                worker.join();
            }
        }

        void sync()
        {
            std::unique_lock<std::mutex> lock(mutex);
            doneCv.wait(lock, [this] { return taskQueue.empty() && !busy; });
        }

    private:
        void threadFunc()
        {
            while (true)
            {

                std::unique_lock<std::mutex> lock(mutex);
                workCv.wait(lock, [this] { return !m_taskQueue.empty() || m_stop; });
            }
        }
    };


    /*class TextureWorker {
    public:
        static TextureWorker& instance()
        {
            static TextureWorker instance;
            return instance;
        }

        void link(GraphicsTextureManager* pOwner)
        {
            assert(pOwner);
            m_worker = std::thread(&TextureWorker::threadFunc, this, pOwner);
        }

        void stop()
        {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_stop = true;
            }

            m_workCv.notify_one();
            if (m_worker.joinable())
            {
                m_worker.join();
            }
        }

        void addTask(TextureTask&& task)
        {
            m_taskQueue.emplace_back(task);
            m_workCv.notify_one();
        }

        void sync()
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_doneCv.wait(lock, [this] { return m_taskQueue.empty() && !m_busy; });
        }

        std::mutex& mutex()
        {
            return m_mutex;
        }

    private:
        void threadFunc(GraphicsTextureManager* pOwner)
        {
            assert(pOwner);

            while (true)
            {
                TextureTask task;
                GraphicsTexture* pTexture = nullptr;

                {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    m_workCv.wait(lock, [this] { return !m_taskQueue.empty() || m_stop; });
                    if (m_stop && m_taskQueue.empty())
                    {
                        break;
                    }

                    task = std::move(m_taskQueue.front());
                    m_taskQueue.pop_front();
                    m_busy = true;

                    assert(task.textureHandle != InvalidHandle && task.textureHandle < pOwner->m_texturePool.size());
                    pTexture = pOwner->m_texturePool[task.textureHandle].pTexture;
                }

                BasicImage<> filledImage;
                auto level = calculateLevel(task.image.width(), task.image.height());
                assert(level >= 0 && level < GraphicsTextureManager::TextureLevelNum);
                auto targetSize = TEXTURE_LEVEL_SIZE[level];

                if (task.image.width() != targetSize || task.image.height() != targetSize)
                {
                    fillImage(task.image, filledImage, targetSize);
                }
                else
                {
                    filledImage = std::move(task.image);
                }

                assert(task.layer < pTexture->depth());
                pTexture->setImage(filledImage, task.layer);

                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_busy = false;
                }

                m_doneCv.notify_all();
            }
        }

        
        std::thread m_worker;
        std::mutex m_mutex;
        std::condition_variable m_workCv;
        std::condition_variable m_doneCv;
        bool m_busy = false;
        bool m_stop = false;
    };*/

    GraphicsTextureManager::GraphicsTextureManager()
    {
        m_imagePool.emplace_back(ImageData());
        m_texturePool.emplace_back(TextureData());
        m_pWorker.reset(new TextureWorker());
    }

    GraphicsTextureManager::~GraphicsTextureManager()
    {
        assert(m_pWorker);
        m_pWorker->stop();
    }

    GraphicsTextureManager& GraphicsTextureManager::instance()
    {
        static GraphicsTextureManager instance;
        return instance;
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

        if (width > TEXTURE_LEVEL_SIZE[TextureLevelNum - 1] || height > TEXTURE_LEVEL_SIZE[TextureLevelNum - 1] || channels > ImageChannelNum)
        {
            BasicLog::out(BasicLog::kWarn, "Image is too large or has too many channels, not supported yet.");
            return ret;
        }

        auto level = calculateLevel(width, height);
        auto dataSize = static_cast<unsigned int>(width * height * channels * sizeof(unsigned char));
        auto hash = xxHash64(image.data(), dataSize);

        std::lock_guard<std::mutex> lock(TextureWorker::instance().mutex());

        // 检查是否存在相同的image（遍历所有同hash的entry，支持hash碰撞）
        auto& imageMap = m_imageMap[level][channels];
        auto range = imageMap.equal_range(hash);
        for (auto itr = range.first; itr != range.second; ++itr)
        {
            auto imageHandle = itr->second;
            assert(imageHandle > 0 && imageHandle < m_imagePool.size());
            auto& storage = m_imagePool[imageHandle];
            if (storage.refNum > 0 && isSameImage(image, storage.image, dataSize))
            {
                storage.refNum++;
                ret = { storage.textureHandle, imageHandle };
                return ret;
            }
        }

        // 新image，需要确定handle
        // 首先确定texture handle，即image存放在哪个texture中
        TextureHandle textureHandle = InvalidHandle;
        auto& textureList = m_textureMap[slot][level][channels];
        for (auto index : textureList)
        {
            assert(index > 0 && index < m_texturePool.size());
            auto& textureData = m_texturePool[index];
            assert(textureData.pTexture);
            if (textureData.freeList.empty() == false || textureData.pTexture->depth() < MAX_TEXTURE_DEPTH)
            {
                textureHandle = index;
                break;
            }
        }

        // 没有texture可以容纳新image，创建新的texture
        if (textureHandle == InvalidHandle)
        {
            textureHandle = static_cast<TextureHandle>(m_texturePool.size());
            m_texturePool.emplace_back(TextureData{ new GraphicsTexture, {} });
            textureList.push_back(textureHandle);
            //m_texturePool.emplace_back();
            //auto& newPool = m_texturePool.back();
            //newPool.slot = slot;
            //newPool.level = level;
            //newPool.channels = channels;
            //textureList.push_back(textureHandle);
        }

        assert(textureHandle > 0 && textureHandle < m_texturePool.size());
        auto& textureData = m_texturePool[textureHandle];
        assert(textureData.pTexture);

        unsigned int layer = 0;
        if (textureData.freeList.empty() == false)
        {
            layer = textureData.freeList.back();
            textureData.freeList.pop_back();
        }
        else
        {
            layer = textureData.pTexture->depth();
        }
        // 注意这里的layer可能越界，将在worker中真正填充texture

        //bool usePush = false;
        //if (pool.freeList.empty() == false)
        //{
        //    layer = pool.freeList.back();
        //    pool.freeList.pop_back();
        //    usePush = false;
        //}
        //else
        //{
        //    layer = pool.layerCount;
        //    pool.layerCount++;
        //    usePush = true;
        //}

        // 随后确定image handle
        ImageHandle imageHandle = InvalidHandle;
        if (m_imageFreeList.empty() == false)
        {
            imageHandle = m_imageFreeList.back();
            m_imageFreeList.pop_back();
        }
        else
        {
            imageHandle = static_cast<ImageHandle>(m_imagePool.size());
            m_imagePool.emplace_back();
        }

        assert(imageHandle > 0 && imageHandle < m_imagePool.size());
        auto& imageData = m_imagePool[imageHandle];
        imageData.image = image;
        imageData.textureHandle = textureHandle;
        imageData.layer = layer;
        imageData.refNum = 1;
        imageData.hash = hash;

        //auto& imgData = m_imagePool[imageHandle];
        //imgData.image = image;
        //imgData.hash = hash;
        //imgData.textureHandle = textureHandle;
        //imgData.layer = layer;
        //imgData.refNum = 1;

        imageMap.insert({ hash, imageHandle });

        // 将image填充与texture填充等耗时的计算放在子线程中
        TextureWorker::instance().addTask(TextureTask{ image, textureHandle, layer });

        ret = { textureHandle, imageHandle };
        return ret;
    }

    bool GraphicsTextureManager::ref(ImageHandle handle)
    {
        if (handle == InvalidHandle || handle >= m_imagePool.size())
        {
            BasicLog::out(BasicLog::kWarn, "Invalid image handle when adding reference count.");
            return false;
        }

        std::lock_guard<std::mutex> lock(TextureWorker::instance().mutex());

        auto& imageData = m_imagePool[handle];
        if (imageData.refNum == 0)
        {
            return false;
        }

        imageData.refNum++;
        return true;
    }

    bool GraphicsTextureManager::unref(ImageHandle handle)
    {
        if (handle == InvalidHandle || handle >= m_imagePool.size())
        {
            BasicLog::out(BasicLog::kWarn, "Invalid image handle when decreasing reference count.");
            return false;
        }

        std::lock_guard<std::mutex> lock(TextureWorker::instance().mutex());

        auto& imageData = m_imagePool[handle];
        if (imageData.refNum == 0)
        {
            return false;
        }

        imageData.refNum--;

        if (imageData.refNum == 0)
        {
            auto textureHandle = imageData.textureHandle;
            assert(textureHandle < m_texturePool.size());
            auto& textureData = m_texturePool[textureHandle];
            assert(imageData.layer < textureData.pTexture->depth());
            textureData.freeList.push_back(imageData.layer);

            auto level = calculateLevel(imageData.image.width(), imageData.image.height());
            auto channels = imageData.image.channels();
            auto& imgMap = m_imageMap[level][channels];
            auto range = imgMap.equal_range(imageData.hash);
            for (auto itr = range.first; itr != range.second; ++itr)
            {
                if (itr->second == handle)
                {
                    imgMap.erase(itr);
                    break;
                }
            }

            m_imageFreeList.push_back(handle);
        }

        return true;
    }

    //bool GraphicsTextureManager::queryImageMatch(ImageHandle handle, unsigned int width, unsigned int height, unsigned char channels) const
    //{
    //    std::lock_guard<std::mutex> lock(TextureWorker::instance().mutex());

    //    if (handle == InvalidHandle || handle >= m_imagePool.size())
    //    {
    //        return false;
    //    }

    //    const auto& entry = m_imagePool[handle];
    //    return entry.image.width() == width && entry.image.height() == height && entry.image.channels() == channels;
    //}

    void GraphicsTextureManager::sync() const
    {
        TextureWorker::instance().sync();
    }

    //unsigned int GraphicsTextureManager::imageLayer(ImageHandle handle) const
    //{
    //    std::lock_guard<std::mutex> lock(TextureWorker::instance().mutex());

    //    if (handle == InvalidHandle || handle >= m_imagePool.size())
    //    {
    //        return 0;
    //    }

    //    return m_imagePool[handle].layer;
    //}

    //int GraphicsTextureManager::imageWidth(ImageHandle handle) const
    //{
    //    std::lock_guard<std::mutex> lock(TextureWorker::instance().mutex());

    //    if (handle == InvalidHandle || handle >= m_imagePool.size())
    //    {
    //        return 0;
    //    }

    //    return static_cast<int>(m_imagePool[handle].image.width());
    //}

    //int GraphicsTextureManager::imageHeight(ImageHandle handle) const
    //{
    //    std::lock_guard<std::mutex> lock(TextureWorker::instance().mutex());

    //    if (handle == InvalidHandle || handle >= m_imagePool.size())
    //    {
    //        return 0;
    //    }

    //    return static_cast<int>(m_imagePool[handle].image.height());
    //}

    //unsigned int GraphicsTextureManager::imageTierSize(ImageHandle handle) const
    //{
    //    std::lock_guard<std::mutex> lock(TextureWorker::instance().mutex());

    //    if (handle == InvalidHandle || handle >= m_imagePool.size())
    //    {
    //        return 0;
    //    }

    //    auto texHandle = m_imagePool[handle].textureHandle;
    //    assert(texHandle < m_texturePool.size());
    //    auto level = m_texturePool[texHandle].level;
    //    return TEXTURE_LEVEL_SIZE[level];
    //}

    //const GraphicsTexture* GraphicsTextureManager::poolTexture(TextureHandle handle) const
    //{
    //    std::lock_guard<std::mutex> lock(TextureWorker::instance().mutex());

    //    if (handle == InvalidHandle || handle >= m_texturePool.size())
    //    {
    //        return nullptr;
    //    }

    //    return &m_texturePool[handle].texture;
    //}

} // namespace FX
