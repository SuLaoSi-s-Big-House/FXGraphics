#include "graphics_material.h"

#include <assert.h>
#include "basic_log.h"
#include "graphics_texture_manager.h"

namespace FX {

    bool TextureKey::setImage(TextureSlot slot, ImageHandle handle)
    {
        if (slot >= TextureSlotNum)
        {
            BasicLog::out(BasicLog::kWarn, "Invalid texture slot when setting image, discard.");
            return false;
        }

        if (GraphicsTextureManager::instance().valid(handle) == false)
        {
            BasicLog::out(BasicLog::kWarn, "Invalid image handle when setting image, please register first.");
            return false;
        }

        m_handles[slot] = handle;
        return true;
    }

    bool TextureKey::resetImage(TextureSlot slot)
    {
        if (slot >= TextureSlotNum)
        {
            BasicLog::out(BasicLog::kWarn, "Invalid texture slot when resetting image, discard.");
            return false;
        }

        m_handles[slot] = InvalidHandle;
        return true;
    }

    ImageHandle TextureKey::handle(TextureSlot slot) const
    {
        if (slot >= TextureSlotNum)
        {
            BasicLog::out(BasicLog::kWarn, "Invalid texture slot when querying image, discard.");
            return InvalidHandle;
        }

        return m_handles[slot];
    }

    //bool TextureKey::operator==(const TextureKey& other) const
    //{
    //    for (TextureSlot i = 0; i < TextureSlotNum; i++)
    //    {
    //        if (m_handles[i] != other.m_handles[i])
    //        {
    //            return false;
    //        }
    //    }

    //    return true;
    //}

    //bool TextureKey::operator!=(const TextureKey& other) const
    //{
    //    return !(*this == other);
    //}

    bool Font::operator==(const Font& other) const
    {
        return this->name == other.name && this->size == other.size;
    }

    bool Font::valid() const
    {
        return name.empty() == false && size > 0;
    }

} // namespace FX
