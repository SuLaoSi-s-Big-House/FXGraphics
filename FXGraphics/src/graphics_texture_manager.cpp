#include "graphics_texture_manager.h"

namespace FX {

    GraphicsTextureManager& GraphicsTextureManager::instance()
    {
        static GraphicsTextureManager instance;
        return instance;
    }

    std::pair<TextureHandle, ImageHandle> GraphicsTextureManager::addImage(TextureSlot slot, const BasicImage<>& image)
    {
        // TODO 计算image hash（根据width/height/channels/pData），判断是否已经有相同的image
        // 如果有完全一样的image，则立刻返回TextureHandle与ImageHandle
        // 如果不存在一样的image，则需要计算ImageHandle，并将其添加到合适的texture中，得到TextureHandle
    }

    bool GraphicsTextureManager::ref(ImageHandle handle)
    {
        // TODO
    }

    bool GraphicsTextureManager::unref(ImageHandle handle)
    {
        // TODO
    }


} // namespace FX
