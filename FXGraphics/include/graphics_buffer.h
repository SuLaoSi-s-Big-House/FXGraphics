#ifndef _GRAPHICS_BUFFER_H_
#define _GRAPHICS_BUFFER_H_

#include <set>
#include "graphics_gpu_item.h"

namespace FX {

    using BufferSlot = unsigned char;
    constexpr BufferSlot NormalGlobalInfoSlot = 0;
    constexpr BufferSlot NormalProfileSlot = 1;

    // 此文件定义了GraphicsBuffer与GraphicsVAO，对OpenGL buffer与OpenGL vao进行了封装。

    // For users:
    // 如果用户需要实现特定的数据生成，应当从GraphicsBufferManager派生并实现相应的函数，而不应当直接修改这个文件中的类。

    class GraphicsBuffer : public GraphicsGPUItem {
    protected:
        explicit GraphicsBuffer(GPUItemType type);

    public:
        void setRebuildStart(int start);
        void addDirtyList(const std::set<int>& dirtyList);
    };


    class BufferInfo : public ItemInfo {
    protected:
        friend class GraphicsBuffer;

        explicit BufferInfo(const GraphicsBuffer* pOwner);
        ~BufferInfo(void) override;

    public:
        void bind(void) const;
        void bind(BufferSlot slot) const;
        void unbind(void) const;
        // 注意setData、setSubData与OpenGL函数含义不同，总是会申请足够的空间并存放数据
        void setData(unsigned int size, const void* pData);
        void setSubData(unsigned int start, unsigned int size, const void* pData);

        int rebuildStart(void) const;
        const std::set<int>& dirtyList(void) const;
        void cleanDirty(void);
        void setReady(void);

    protected:
        void setRebuildStart(int start);
        void addDirtyList(const std::set<int>& dirtyList);

    protected:
        unsigned int m_bufferSize = 0;
        unsigned int m_dataSize = 0;
        int m_rebuildStart = 0;
        std::set<int, std::less<int>> m_dirtyList;
    };


    class GraphicsVBO : public GraphicsBuffer {
    public:
        GraphicsVBO(void) : GraphicsBuffer(GPUItemType::kVBO) {}

    protected:
        ItemInfo* create(void) const override;
    };


    class VBOInfo : public BufferInfo {
    protected:
        friend class GraphicsVBO;

        explicit VBOInfo(const GraphicsVBO* pOwner) : BufferInfo(pOwner) {}
    };


    class GraphicsEBO : public GraphicsBuffer {
    public:
        GraphicsEBO(void) : GraphicsBuffer(GPUItemType::kEBO) {}

    protected:
        ItemInfo* create(void) const override;
    };


    class EBOInfo : public BufferInfo {
    protected:
        friend class GraphicsEBO;

        explicit EBOInfo(const GraphicsEBO* pOwner) : BufferInfo(pOwner) {}
    };


    class GraphicsSSBO : public GraphicsBuffer {
    public:
        GraphicsSSBO(void) : GraphicsBuffer(GPUItemType::kSSBO) {}

    protected:
        ItemInfo* create(void) const override;
    };


    class SSBOInfo : public BufferInfo {
    protected:
        friend class GraphicsSSBO;

        explicit SSBOInfo(const GraphicsSSBO* pOwner) : BufferInfo(pOwner) {}
    };


    class GraphicsDIBO : public GraphicsBuffer {
    public:
        GraphicsDIBO(void) : GraphicsBuffer(GPUItemType::kDIBO) {}

    protected:
        ItemInfo* create(void) const override;
    };


    class DIBOInfo : public BufferInfo {
    protected:
        friend class GraphicsDIBO;

        explicit DIBOInfo(const GraphicsDIBO* pOwner) : BufferInfo(pOwner) {}

    public:
        void setCommandNum(unsigned int num);
        unsigned int commandNum(void) const;

    protected:
        unsigned int m_commandNum = 0;
    };


    class GraphicsUBO : public GraphicsBuffer {
    public:
        GraphicsUBO(void) : GraphicsBuffer(GPUItemType::kUBO) {}

    protected:
        ItemInfo* create(void) const override;
    };


    class UBOInfo : public BufferInfo {
    protected:
        friend class GraphicsUBO;

        explicit UBOInfo(const GraphicsUBO* pOwner) : BufferInfo(pOwner) {}
    };


    class GraphicsVAO : public GraphicsGPUItem {
    public:
        GraphicsVAO(void) : GraphicsGPUItem(GPUItemType::kVAO) {}

    protected:
        ItemInfo* create(void) const override;
    };


    class VAOInfo : public ItemInfo {
    protected:
        friend class GraphicsVAO;

        explicit VAOInfo(const GraphicsVAO* pOwner);
        ~VAOInfo(void) override;

    public:
        void bind(void) const;
        void unbind(void) const;
    };

} // namespace FX

#endif // _GRAPHICS_BUFFER_H_
