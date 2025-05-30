#ifndef _GRAPHICS_BUFFER_H_
#define _GRAPHICS_BUFFER_H_

#include "graphics_gpu_item.h"

namespace FX {

    class GraphicsBuffer : public GraphicsGPUItem {
    protected:
        explicit GraphicsBuffer(GPUItemType type);

    public:

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
        void setData(unsigned int size, const void* pData);
        void setSubData(unsigned int start, unsigned int size, const void* pData);

    protected:
        unsigned int m_bufferSize = 0;
        unsigned int m_dataSize = 0;
    };


    class GraphicsVBO : public GraphicsBuffer {
    protected:
        GraphicsVBO(void) : GraphicsBuffer(GPUItemType::kVBO) {}

        ItemInfo* create(void) const override;
    };


    class VBOInfo : public BufferInfo {
    protected:
        friend class GraphicsVBO;

        explicit VBOInfo(const GraphicsVBO* pOwner) : BufferInfo(pOwner) {}
    };


    class GraphicsUBO : public GraphicsBuffer {
    protected:
        friend class GraphicsScene;

        GraphicsUBO(void) : GraphicsBuffer(GPUItemType::kUBO) {}

        ItemInfo* create(void) const override;
    };


    class UBOInfo : public BufferInfo {
    protected:
        friend class GraphicsUBO;

        explicit UBOInfo(const GraphicsUBO* pOwner) : BufferInfo(pOwner) {}
    };

} // namespace FX

#endif // _GRAPHICS_BUFFER_H_
