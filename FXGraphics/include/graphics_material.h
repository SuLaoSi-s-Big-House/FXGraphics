#ifndef _GRAPHICS_MATERIAL_H_
#define _GRAPHICS_MATERIAL_H_

#include <string>
#include "glm.hpp"

#include "basic_vector.h"
#include "basic_image.h"

namespace FX {

    struct Font {
        std::string name = "Arial";
        unsigned char size = 16;

        bool operator==(const Font& other) const;
        bool valid(void) const;
    };

    using TextureSlot = unsigned char;
    constexpr TextureSlot BaseColorTextureSlot = 0;
    constexpr TextureSlot NormalTextureSlot = 1;
    constexpr TextureSlot ORMTextureSlot = 2;

    using TextureHandle = unsigned int;
    using ImageHandle = unsigned int;
    constexpr unsigned int InvalidHandle = 0;

    class TextureKeyImpl;

    // 设计意图：TextureKey是暴露给用户的控制块，允许用户添加BasicImage用于纹理显示
    // 目前TextureKey支持三个TextureSlot，对应三个纹理单元，即一个物体最多同时使用三个纹理进行绘制
    // TextureKey存储TextureHandle，用于判断GraphicsEntity是否能放在同一个EntityList。TextureHandle不需要加减计数
    // TextureKeyImpl存储ImageHandle，用于维护image数据在GraphicsTextureManager中的声明周期（加减计数）

    class TextureKey {
    public:
        TextureKey(void);
        ~TextureKey(void);

        bool setImage(TextureSlot slot, const BasicImage<>& image);
        bool resetImage(TextureSlot slot);

        TextureHandle handle(TextureSlot slot) const;

        TextureKey(const TextureKey& other);
        TextureKey& operator=(const TextureKey& other);
        TextureKey(TextureKey&& other) noexcept;
        TextureKey& operator=(TextureKey&& other) noexcept;
        bool operator==(const TextureKey& other) const;

        static constexpr unsigned char TextureSlotNum = 3;

    private:
        TextureHandle m_handles[TextureSlotNum] = { InvalidHandle, InvalidHandle, InvalidHandle };
        TextureKeyImpl* m_pImpl = nullptr;
    };

    // 实体属性
    struct EntityProfile {
        glm::mat4 matrix = glm::mat4(1.0f);
        Font font;
        vec4uc color = { 255, 255, 255, 255 };
        bool visible = true;
        TextureKey texture;
        vec4f custom1;    // 预留属性，修改后用户需要自行调用setDirty通知图形系统
        vec4f custom2;
    };

} // namespace FX

#endif // _GRAPHICS_MATERIAL_H_
