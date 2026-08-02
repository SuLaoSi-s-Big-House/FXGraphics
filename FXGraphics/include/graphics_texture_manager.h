#ifndef _GRAPHICS_TEXTURE_MANAGER_H_
#define _GRAPHICS_TEXTURE_MANAGER_H_

#include "graphics_material.h"

#include <stdint.h>
#include <vector>
#include <unordered_map>

namespace FX {

    class GraphicsTexture;

    class GraphicsTextureManager {
    public:
        static GraphicsTextureManager& instance(void);

        std::pair<TextureHandle, ImageHandle> addImage(TextureSlot slot, const BasicImage<>& image);

        bool ref(ImageHandle handle);
        bool unref(ImageHandle handle);

        bool queryImageMatch(ImageHandle handle, unsigned int width, unsigned int height, unsigned char channels) const;

    private:
        GraphicsTextureManager(void);
        ~GraphicsTextureManager(void) = default;

        TextureHandle allocTextureHandle(const TextureEntry& entry);
        void freeTextureHandle(TextureHandle handle);

        ImageHandle allocImageHandle(const ImageEntry& entry);
        void freeImageHandle(ImageHandle handle);

    public:
        static constexpr unsigned char TextureSlotNum = 3;
        static constexpr unsigned char TextureLevelNum = 4;
        static constexpr unsigned char ImageChannelNum = 4;

    private:

        

        using Hash = uint64_t;

        struct ImageData {
            BasicImage<> image;
            Hash hash = 0;
            unsigned long long refNum = 0;
            // unsigned char level = 0;
            TextureHandle textureHandle = InvalidHandle;
        };

        std::unordered_map<Hash, ImageHandle> m_imageMap[TextureLevelNum][ImageChannelNum];
        std::vector<ImageData> m_imagePool;
        std::vector<unsigned int> m_freeList;

        struct TextureData {
            GraphicsTexture texture;
            std::vector<unsigned int> freeList;
        };

        std::vector<unsigned int> m_textureMap[TextureSlotNum][TextureLevelNum][ImageChannelNum];
        std::vector<TextureData> m_texturePool;


        //struct TextureEntry {
        //    TextureSlot slot = 0;
        //    unsigned char tier = 0;
        //    unsigned char channels = 0;
        //    unsigned int arrayIndex = 0;
        //    unsigned int layer = 0;
        //};

        //struct ImageEntry {
        //    BasicImage<unsigned char> image;
        //    uint64_t hash = 0;
        //    unsigned int refCount = 0;
        //    unsigned char tier = 0;
        //    TextureHandle texHandle = InvalidHandle;
        //};

        //struct TextureGroup {
        //    std::vector<GraphicsTexture*> arrays;
        //    std::vector<unsigned int> freeSlots;
        //};

        //TextureGroup m_groups[SlotCount][TierCount][MaxChannelCount];

        //std::vector<TextureEntry> m_textureEntries;
        //std::vector<unsigned int> m_textureFreeList;

        //std::vector<ImageEntry> m_imagePool;
        
    };

} // namespace FX

#endif // _GRAPHICS_TEXTURE_MANAGER_H_
