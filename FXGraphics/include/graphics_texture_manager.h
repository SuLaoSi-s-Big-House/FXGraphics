#ifndef _GRAPHICS_TEXTURE_MANAGER_H_
#define _GRAPHICS_TEXTURE_MANAGER_H_

#include "graphics_material.h"

namespace FX {

    // 设计意图：GraphicsTextureManager用于管理image与texture数据，协助绘制
    // GraphicsTextureManager会接收TextureKeyImpl传递的image，在内部做去重，根据image的width/height/channles/pData做hash去重，并维护引用计数
    // 接收到新的image后，需要根据width/height/channles/slot找到合适的texture array，将图片存入texture array（决定TextureHandle）

    class GraphicsTextureManager {
    public:
        static GraphicsTextureManager& instance(void);

        std::pair<TextureHandle, ImageHandle> addImage(TextureSlot slot, const BasicImage<>& image);

        bool ref(ImageHandle handle);
        bool unref(ImageHandle handle);

        // TODO query接口，帮助TextureKeyImpl判断是否能成功添加image

    private:
        GraphicsTextureManager(void) = default;
        ~GraphicsTextureManager(void) = default;

    private:
        // TODO 成员变量
        // 1.image pool
        // 根据image的width/height/channles/pData做hash（根据operator==判断是否碰撞），维护ImageHandle与image数据和引用计数
        // 添加image时，image pool中会存储（拷贝）一份一样的BasicImage（不依赖外部数据的生命周期）
        // 2.texture pool
        // 当新增image时，需要为image找到存放的texture，并确定TextureHandle
        // 由于image的尺寸不可控，texture pool只存储标准大小的texture，只有64/256/512/1024像素的四种档位，需要为image找到合适的档位，并填充空缺的数据
        // 即texture总是几种标准尺寸，而image是与原始数据相同的大小，这意味着texture array需要记录原始image数据的尺寸，便于生成uv数据
    };

} // namespace FX

#endif // _GRAPHICS_TEXTURE_MANAGER_H_
