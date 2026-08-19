#ifndef _GRAPHICS_TEXTURE_MANAGER_H_
#define _GRAPHICS_TEXTURE_MANAGER_H_

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "basic_macro.h"
#include "graphics_material.h"
#include "graphics_texture.h"

namespace FX {

    //// 解析后的image信息，供profile导出与绑定使用
    //struct ResolvedImage {
    //    GraphicsTexture* pTexture = nullptr;
    //    unsigned int slice = 0;
    //    float scaleX = 1.0f;    // 实际可用的纹理范围：uv * scale
    //    float scaleY = 1.0f;
    //    bool hasAlpha = false;
    //};

    struct TexturePack {
        TextureHandle textureHandle = InvalidHandle;
        unsigned int slice = 0;
        vec2f scale = { 1.0f, 1.0f };
    };

    class GraphicsTextureManager {
    public:
        static GraphicsTextureManager& instance(void);

        ImageHandle registerImage(const BasicImage<>& image);
        ImageHandle registerImage(const std::string& path);
        std::vector<ImageHandle> registerImage(const std::vector<std::string>& paths);

        bool unregisterImage(ImageHandle handle);

        bool valid(ImageHandle handle) const;
        TexturePack query(ImageHandle handle) const;

        void bind(const TextureKey& texture);

        // 解析ImageHandle对应的纹理信息，handle无效或未设置时返回对应slot的中性fallback
        //ResolvedImage resolve(ImageHandle handle, TextureSlot slot);

    private:
        GraphicsTextureManager(void);
        ~GraphicsTextureManager(void) = default;

        void addDefaultImage(void);

        TextureHandle findBestTexture(unsigned char level, unsigned char channels);

        // handle = index + 1（0保留给InvalidHandle），是简单的数值下标
        // 失效handle不做编码检测（契约制），仅当index未被复用时valid()返回false
        //struct ImageEntry {
        //    BasicImage<> image;                   // 原始image，保留用于后续脏更新（markImageDirty）
        //    GraphicsTexture* pTexture = nullptr;  // 填充后image所在的texture
        //    unsigned int slice = 0;               // 填充后image所在的slice
        //    bool hasAlpha = false;                // 原始image是否含非不透明像素
        //};

        //struct TextureEntry {
        //    std::unique_ptr<GraphicsTexture> pTexture;
        //    std::vector<unsigned int> freeList;    // 反注册后标记为可复用的slice下标，下次注册优先复用
        //};

        //unsigned int allocIndex(void);
        //void releaseIndex(unsigned int index);
        //void ensureFallbackTexture(void);

    private:
        struct ImageData {
            std::unique_ptr<BasicImage<>> pImage;
            TextureHandle texture = InvalidHandle;
            unsigned int slice = 0;
            vec2f scale = { 1.0f, 1.0f };
        };

        std::vector<ImageData> m_imagePool;
        std::vector<unsigned int> m_freeList;

        struct TextureData {
            std::unique_ptr<GraphicsTexture> pTexture;
            std::vector<unsigned int> freeList;
        };

        std::vector<TextureData> m_texturePool;
        std::unordered_map<unsigned long long, std::vector<TextureHandle>> m_textureMap;

        //using TexturePool = std::map<std::pair<unsigned char, unsigned char>, std::vector<TextureData>>;
        //TexturePool m_texturePool;
        //std::vector<TextureData*> m_textureList;


        // 按(尺寸档位, 格式)分组的texture容器，texture在首次registerImage时懒创建
        //using TextureGroup = std::vector<TextureEntry>;
        //std::map<std::pair<unsigned int, GraphicsTextureBase::Format>, TextureGroup> m_textureContainer;

        //std::vector<ImageEntry> m_handleTable;
        //std::vector<unsigned int> m_freeList;

        // 1×1×3的fallback纹理，slice与TextureSlot对应：base白色、normal中性法向、orm中性值
        //std::unique_ptr<GraphicsTexture> m_pFallbackTexture;
    };

} // namespace FX

#endif // _GRAPHICS_TEXTURE_MANAGER_H_
