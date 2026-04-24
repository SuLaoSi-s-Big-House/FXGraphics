#ifndef _GRAPHICS_GPU_ITEM_H_
#define _GRAPHICS_GPU_ITEM_H_

#include <unordered_map>
#include "basic_macro.h"

namespace FX {

    enum class GPUItemType : unsigned int {
        kVAO = 1,
        kVBO = 0x8892,
        kEBO = 0x8893,
        kSSBO = 0x90D2,
        kDIBO = 0x8F3F,
        kUBO = 0x8A11,
        kProgram = 0x82E2,
        kVtxShader = 0x8B31,
        kFrgShader = 0x8B30,
        kGeoShader = 0x8DD9
    };

    class GraphicsWindowImpl;
    class ItemInfo;

    // 此文件定义了GraphicsGPUItem与ItemInfo。
    // 每个ItemInfo表示一个OpenGL GPU资源，其中保存了OpenGL handle与资源类型type。
    // GraphicsGPUItem是ItemInfo在不同窗口中的集合，其中保存了每个窗口中的ItemInfo对象（如果有）。
    // 当涉及GPU资源的创建与销毁、窗口的销毁时，GraphicsGPUItem会与GraphicsWindow互动，以维护GPU资源的生命周期。

    // For users:
    // 如果用户需要创建新的类型且涉及GPU资源，都应当从GraphicsGPUItem与ItemInfo派生。
    // 用户不应当直接使用或修改GraphicsGPUItem。

    class GraphicsGPUItem {
    protected:
        friend class GraphicsWindowImpl;

        explicit GraphicsGPUItem(GPUItemType type) : m_type(type) {}
        virtual ~GraphicsGPUItem(void);

        DELETE_COPY_AND_MOVE_CONSTRUCT(GraphicsGPUItem);

        // 派生类需要实现create函数，需要创建对应的ItemInfo派生类对象并返回。
        // 必须通过new创建对象，用户不需要维护其生命周期。
        virtual ItemInfo* create(void) const = 0;

        void clearItem(GraphicsWindowImpl* pWindow);

    public:
        // 如果当前没有正在使用的GraphicsWindow，此函数会返回空指针。
        // 如果bForceCreate为true，则始终会创建新的ItemInfo，如果当前GraphicsWindow下已经存在ItemInfo，则会删除旧的ItemInfo。
        ItemInfo* getOrCreate(bool bForceCreate = false);    

        GPUItemType type(void) const;

    protected:
        std::unordered_map<GraphicsWindowImpl*, ItemInfo*> m_itemList;
        const GPUItemType m_type = GPUItemType(0);
    };


    // For users:
    // ItemInfo与OpenGL GPU资源的生命周期保持一致，即派生类需要在构造函数里创建GPU资源，在析构函数里销毁GPU资源。

    class ItemInfo {
    protected:
        friend class GraphicsGPUItem;
        friend class GraphicsWindowImpl;

        explicit ItemInfo(const GraphicsGPUItem* pOwner);

    public:
        virtual ~ItemInfo(void) = default;

        DELETE_COPY_AND_MOVE_CONSTRUCT(ItemInfo);

    public:
        const GraphicsGPUItem* m_pOwner = nullptr;
        const GPUItemType m_type = GPUItemType(0);
        unsigned int m_handle = 0;
    };

} // namespace FX

#endif // _GRAPHICS_GPU_ITEM_H_
