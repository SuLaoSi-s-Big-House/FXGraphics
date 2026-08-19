#ifndef _GRAPHICS_MATERIAL_H_
#define _GRAPHICS_MATERIAL_H_

#include <string>
#include "glm.hpp"

#include "basic_vector.h"
#include "basic_image.h"

namespace FX {

    using TextureSlot = unsigned char;
    constexpr TextureSlot BaseColorTextureSlot = 0;
    constexpr TextureSlot NormalTextureSlot = 1;
    constexpr TextureSlot ORMTextureSlot = 2;
    constexpr unsigned char TextureSlotNum = 3;

    using TextureHandle = unsigned int;
    using ImageHandle = unsigned int;
    constexpr unsigned int InvalidHandle = 0;

    struct Font {
        std::string name = "Arial";
        unsigned char size = 16;

        bool operator==(const Font& other) const;
        bool valid(void) const;
    };

    class TextureKey {
    public:
        bool setImage(TextureSlot slot, ImageHandle handle);
        bool resetImage(TextureSlot slot);

        ImageHandle handle(TextureSlot slot) const;

        //bool operator==(const TextureKey& other) const;
        //bool operator!=(const TextureKey& other) const;

    private:
        ImageHandle m_handles[TextureSlotNum] = { InvalidHandle, InvalidHandle, InvalidHandle };
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
