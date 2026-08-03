#include "graphics_material.h"

#include <assert.h>
#include <string.h>
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
            for (unsigned char i = 0; i < TextureKey::TextureSlotNum; i++)
            {
                if (m_handles[i] != InvalidHandle)
                {
                    GraphicsTextureManager::instance().unref(m_handles[i]);
                }
            }
        }

        TextureHandle setImage(unsigned char slot, const BasicImage<>& image)
        {
            assert(slot < TextureKey::TextureSlotNum);

            if (image.valid() == false)
            {
                BasicLog::out(BasicLog::kWarn, "Trying to add an invalid image to texture key, discard.");
                return InvalidHandle;
            }

            auto result = GraphicsTextureManager::instance().addImage(slot, image);
            if (result.first == InvalidHandle || result.second == InvalidHandle)
            {
                return InvalidHandle;
            }

            if (m_handles[slot] != InvalidHandle)
            {
                GraphicsTextureManager::instance().unref(m_handles[slot]);
            }

            m_handles[slot] = result.second;
            return result.first;
        }

        bool resetImage(TextureSlot slot)
        {
            assert(slot < TextureKey::TextureSlotNum);

            if (m_handles[slot] != InvalidHandle)
            {
                GraphicsTextureManager::instance().unref(m_handles[slot]);
                m_handles[slot] = InvalidHandle;
                return true;
            }

            return false;
        }

        //ImageHandle image(TextureSlot slot) const
        //{
        //    if (slot >= TextureKey::TextureSlotNum)
        //    {
        //        return InvalidHandle;
        //    }

        //    return m_handles[slot];
        //}

        TextureKeyImpl(const TextureKeyImpl& other)
        {
            for (unsigned char i = 0; i < TextureKey::TextureSlotNum; i++)
            {
                m_handles[i] = other.m_handles[i];
                if (m_handles[i] != InvalidHandle)
                {
                    GraphicsTextureManager::instance().ref(m_handles[i]);
                }
            }
        }

        TextureKeyImpl& operator=(const TextureKeyImpl& other)
        {
            if (this != &other)
            {
                for (unsigned char i = 0; i < TextureKey::TextureSlotNum; i++)
                {
                    if (m_handles[i] != InvalidHandle)
                    {
                        GraphicsTextureManager::instance().unref(m_handles[i]);
                    }

                    m_handles[i] = other.m_handles[i];

                    if (m_handles[i] != InvalidHandle)
                    {
                        GraphicsTextureManager::instance().ref(m_handles[i]);
                    }
                }
            }

            return *this;
        }

        TextureKeyImpl(TextureKeyImpl&& other) noexcept
        {
            for (unsigned char i = 0; i < TextureKey::TextureSlotNum; i++)
            {
                m_handles[i] = other.m_handles[i];
                other.m_handles[i] = InvalidHandle;
            }
        }

        TextureKeyImpl& operator=(TextureKeyImpl&& other) noexcept
        {
            if (this != &other)
            {
                for (unsigned char i = 0; i < TextureKey::TextureSlotNum; i++)
                {
                    if (m_handles[i] != InvalidHandle)
                    {
                        GraphicsTextureManager::instance().unref(m_handles[i]);
                    }

                    m_handles[i] = other.m_handles[i];
                    other.m_handles[i] = InvalidHandle;
                }
            }

            return *this;
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

        if (handle != InvalidHandle)
        {
            m_handles[slot] = handle;
            return true;
        }

        return false;
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
            m_handles[slot] = InvalidHandle;
            return true;
        }

        return false;
    }

    //TextureHandle TextureKey::handle(TextureSlot slot) const
    //{
    //    if (slot >= TextureSlotNum)
    //    {
    //        BasicLog::out(BasicLog::kWarn, "Invalid texture slot when querying handle from texture key.");
    //        return InvalidHandle;
    //    }

    //    return m_handles[slot];
    //}

    //ImageHandle TextureKey::image(TextureSlot slot) const
    //{
    //    if (slot >= TextureSlotNum || m_pImpl == nullptr)
    //    {
    //        return InvalidHandle;
    //    }

    //    return m_pImpl->image(slot);
    //}

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
        return memcmp(m_handles, other.m_handles, TextureSlotNum * sizeof(TextureHandle)) == 0;
    }

} // namespace FX
