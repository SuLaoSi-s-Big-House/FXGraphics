#ifndef _GRAPHICS_MATERIAL_MANAGER_H_
#define _GRAPHICS_MATERIAL_MANAGER_H_

#include <string>
#include <map>
#include <set>
#include "glm.hpp"

#include "basic_vector.h"
#include "graphics_buffer.h"

namespace FX {

    using MaterialHandle = unsigned int;
    constexpr MaterialHandle DefaultMaterialHandle = 0;

    struct Font {
        std::string name = "Arial";
        unsigned char size = 16;

        bool operator==(const Font& other) const;
        bool operator<(const Font& other) const;
        bool valid(void) const;
    };

    struct Material {
        vec4uc baseColor = { 255, 255, 255, 255 };
        bool visible = true;
        bool selectable = true;
        float metallic = 0.8f;
        float roughness = 0.3f;
        Font font;
        vec4f custom1;    // 预留属性，修改后用户需要自行调用setDirty通知图形系统
        vec4f custom2;

        bool operator==(const Material& other) const;
        bool operator<(const Material& other) const;
    };

    class GraphicsMaterialManager {
    private:
        GraphicsMaterialManager(void);
        ~GraphicsMaterialManager(void) = default;

    public:
        static GraphicsMaterialManager& instance(void);

        // 用户不应调用ref与unref
        MaterialHandle ref(const Material& material);
        void unref(MaterialHandle handle);

        const Material& get(MaterialHandle handle) const;

        void bind(BufferSlot slot);

    private:
        struct MaterialRef {
            std::map<Material, MaterialHandle>::iterator itr;
            unsigned long long refCount = 0;
        };

        std::map<Material, MaterialHandle> m_materialMap;
        std::vector<MaterialRef> m_materialList;
        std::vector<MaterialHandle> m_availableList;
        std::set<int> m_dirtyList;

        struct BufferBlock {
            vec4f rgba;
            vec4f pbr;
            vec4f custom1;
            vec4f custom2;
        };

        GraphicsSSBO m_buffer;
    };

} // namespace FX

#endif // _GRAPHICS_MATERIAL_MANAGER_H_
