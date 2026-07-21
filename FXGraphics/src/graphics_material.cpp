#include "graphics_material.h"

#include <assert.h>
#include "basic_log.h"
#include "graphics_texture_manager.h"

namespace FX {

    bool Font::operator==(const Font& other) const
    {
        return this->name == other.name && this->size == other.size;
    }

    bool Font::valid() const
    {
        return name.empty() == false && size > 0;
    }


    class TextureKeyImpl {
    public:
        TextureKeyImpl() = default;
        ~TextureKeyImpl()
        {
            // TODO 通知GraphicsTextureManager减计数
        }

        TextureHandle setImage(unsigned char slot, const BasicImage<>& image)
        {
            assert(slot < TextureKey::TextureSlotNum);

            if (image.valid() == false)
            {
                BasicLog::out(BasicLog::kWarn, "Trying to add an invalid image to texture key, discard.");
                return false;
            }

            // TODO 如果已经设置过image（m_handles中存在有效的handle），需要向GraphicsTextureManager查询image信息
            // image必须width/height/channels相同才能成功设置，否则立刻返回InvalidHandle

            if (m_handles[slot] != InvalidHandle)
            {
                // TODO 如果这个slot已经有image，需要向GraphicsTextureManager解引用
            }

            // TODO 向GraphicsTextureManager添加image，拿到ImageHandle与TextureHandle
            // m_handles[slot] = imageHandle;
            // return textureHandle;
        }

        bool resetImage(TextureSlot slot)
        {
            assert(slot < TextureKey::TextureSlotNum);

            if (m_handles[slot] != InvalidHandle)
            {
                // TODO 通知GraphicsTextureManager解引用
            }
        }

        TextureKeyImpl(const TextureKeyImpl& other)
        {
            // TODO 拷贝构造，需要通知GraphicsTextureManager加引用
        }

        TextureKeyImpl& operator=(const TextureKeyImpl& other)
        {
            // TODO 拷贝操作符，需要通知GraphicsTextureManager加引用
        }

        TextureKeyImpl(TextureKeyImpl&& other) noexcept
        {
            // TODO
        }

        TextureKeyImpl& operator=(TextureKeyImpl&& other) noexcept
        {
            // TODO
        }

    private:
        ImageHandle m_handles[TextureKey::TextureSlotNum] = { InvalidHandle, InvalidHandle, InvalidHandle };
    };


    TextureKey::TextureKey() : m_pImpl(new TextureKeyImpl)
    {
    }

    TextureKey::~TextureKey()
    {
        if (m_pImpl != nullptr)
        {
            delete m_pImpl;
        }
    }

    bool TextureKey::setImage(TextureSlot slot, const BasicImage<>& image)
    {
        if (slot >= TextureSlotNum)
        {
            BasicLog::out(BasicLog::kWarn, "Invalid texture slot when setting image to texture key.");
            return false;
        }

        auto handle = m_pImpl->setImage(slot, image);
        m_handles[slot] = handle;
        return handle != InvalidHandle;
    }

    bool TextureKey::resetImage(TextureSlot slot)
    {
        if (slot >= TextureSlotNum)
        {
            BasicLog::out(BasicLog::kWarn, "Invalid texture slot when resetting image in texture key.");
            return false;
        }

        if (m_handles[slot] != InvalidHandle)
        {
            m_pImpl->resetImage(slot);
            return true;
        }

        return false;
    }

    TextureHandle TextureKey::handle(TextureSlot slot) const
    {
        if (slot >= TextureSlotNum)
        {
            BasicLog::out(BasicLog::kWarn, "Invalid texture slot when querying handle from texture key.");
            return InvalidHandle;
        }

        return m_handles[slot];
    }

    TextureKey::TextureKey(const TextureKey& other) : m_pImpl(new TextureKeyImpl(*other.m_pImpl))
    {
        memcpy(m_handles, other.m_handles, TextureSlotNum * sizeof(TextureHandle));
    }

    TextureKey& TextureKey::operator=(const TextureKey& other)
    {
        if (this != &other)
        {
            *m_pImpl = *other.m_pImpl;
            memcpy(m_handles, other.m_handles, TextureSlotNum * sizeof(TextureHandle));
        }
        return *this;
    }

    TextureKey::TextureKey(TextureKey&& other) noexcept : m_pImpl(other.m_pImpl)
    {
        for (unsigned char i = 0; i < TextureSlotNum; i++)
        {
            m_handles[i] = other.m_handles[i];
            other.m_handles[i] = InvalidHandle;
        }
        other.m_pImpl = nullptr;
    }

    TextureKey& TextureKey::operator=(TextureKey&& other) noexcept
    {
        if (this != &other)
        {
            for (unsigned char i = 0; i < TextureSlotNum; i++)
            {
                m_handles[i] = other.m_handles[i];
                other.m_handles[i] = InvalidHandle;
            }

            if (m_pImpl != nullptr)
            {
                delete m_pImpl;
            }
            m_pImpl = other.m_pImpl;
            other.m_pImpl = nullptr;
        }
        return *this;
    }

    bool TextureKey::operator==(const TextureKey& other) const
    {
        // TextureKey::operator==的设计意图较为特殊，仅用于判断GraphicsEntity是否能进入某一个EntityList
        // 如果两个TextureKey相等（即所有TextureHandle），意味着使用同一套texture，能合并绘制，因此可以放在一起
        return memcmp(m_handles, other.m_handles, TextureSlotNum * sizeof(TextureHandle));
    }

} // namespace FX
