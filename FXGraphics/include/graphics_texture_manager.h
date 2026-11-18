#ifndef _GRAPHICS_TEXTURE_MANAGER_H_
#define _GRAPHICS_TEXTURE_MANAGER_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include "basic_vector.h"
#include "graphics_material.h"
#include "graphics_texture.h"

namespace FX {

    struct ImageBlock {
        vec2f scale = { 1.0f, 1.0f };
        unsigned int slice = 0;
        TextureHandle textureHandle = InvalidHandle;
        BasicImage<>::AlphaType alpha = BasicImage<>::AlphaType::kAllOpaque;
    };

    class GraphicsTextureManager {
    public:
        static GraphicsTextureManager& instance(void);

        ImageHandle registerImage(const BasicImage<>& image);
        ImageHandle registerImage(const std::string& path);
        std::vector<ImageHandle> registerImage(const std::vector<std::string>& paths);

        bool unregisterImage(ImageHandle handle);

        bool valid(ImageHandle handle) const;

        bool query(ImageHandle handle, ImageBlock& imgBlock) const;

        // 获取纹理对象供GraphicsScene创建TextureInfo并绑定，0表示默认纹理
        GraphicsTexture* texture(TextureHandle handle);

        // 默认纹理的alpha类型，用于无效图片的fallback分类
        BasicImage<>::AlphaType defaultAlpha(void) const;

    private:
        GraphicsTextureManager(void);
        ~GraphicsTextureManager(void) = default;

        void addDefaultImage(void);

        TextureHandle findBestTexture(unsigned char level, unsigned char channels);

    private:
        struct ImageData {
            std::unique_ptr<BasicImage<>> pImage;
            TextureHandle texture = InvalidHandle;
            unsigned int slice = 0;
            vec2f scale = { 1.0f, 1.0f };
            BasicImage<>::AlphaType alpha = BasicImage<>::AlphaType::kAllOpaque;
        };

        std::vector<ImageData> m_imagePool;
        std::vector<unsigned int> m_freeList;

        struct TextureData {
            std::unique_ptr<GraphicsTexture> pTexture;
            std::vector<unsigned int> freeList;
        };

        std::vector<TextureData> m_texturePool;
        std::unordered_map<unsigned long long, std::vector<TextureHandle>> m_textureMap;
    };

} // namespace FX

#endif // _GRAPHICS_TEXTURE_MANAGER_H_
