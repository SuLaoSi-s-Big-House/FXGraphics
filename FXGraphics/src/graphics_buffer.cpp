#include "graphics_buffer.h"

#include <assert.h>
#include "glad.h"
#include "basic_log.h"

namespace FX {

    GraphicsBuffer::GraphicsBuffer(GPUItemType type)
        : GraphicsGPUItem(type)
    {
        assert((type == GPUItemType::kVBO) || (type == GPUItemType::kEBO) || (type == GPUItemType::kSSBO) ||
            (type == GPUItemType::kDIBO) || (type == GPUItemType::kUBO));
    }

    BufferInfo::BufferInfo(const GraphicsBuffer* pOwner)
        : ItemInfo(pOwner)
    {
        assert((m_type == GPUItemType::kVBO) || (m_type == GPUItemType::kEBO) || (m_type == GPUItemType::kSSBO) ||
            (m_type == GPUItemType::kDIBO) || (m_type == GPUItemType::kUBO));
        glGenBuffers(1, &m_handle);
        assert(m_handle != 0);
    }

    BufferInfo::~BufferInfo()
    {
        glDeleteBuffers(1, &m_handle);
    }

    void BufferInfo::bind() const
    {
        glBindBuffer((GLenum)m_type, m_handle);
    }

    void BufferInfo::bind(BufferSlot slot) const
    {
        glBindBufferBase((GLenum)m_type, slot, m_handle);
    }

    void BufferInfo::unbind() const
    {
        glBindBuffer((GLenum)m_type, 0);
    }

    void BufferInfo::setData(unsigned int size, const void* pData)
    {
        assert(pData != nullptr);
        glBufferData((GLenum)m_type, size, pData, GL_STATIC_DRAW);
        m_bufferSize = m_dataSize = size;
    }

    void BufferInfo::setSubData(unsigned int start, unsigned int size, const void* pData)
    {
        assert(pData != nullptr);

        if (m_bufferSize == 0 || m_dataSize == 0)
        {
            glBufferData((GLenum)m_type, std::max(100u, (start + size) * 3 / 2), nullptr, GL_STATIC_DRAW);
            glBufferSubData((GLenum)m_type, start, size, pData);
            m_bufferSize = std::max(100u, (start + size) * 3 / 2);
            m_dataSize = start + size;
        }

        if (start + size > m_bufferSize)
        {
            unsigned int handle = 0;
            glGenBuffers(1, &handle);
            assert(handle != 0);
            glBindBuffer((GLenum)m_type, handle);
            glBufferData((GLenum)m_type, std::max(100u, (start + size) * 3 / 2), nullptr, GL_STATIC_DRAW);

            glBindBuffer(GL_COPY_READ_BUFFER, m_handle);
            glBindBuffer(GL_COPY_WRITE_BUFFER, handle);
            glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, m_dataSize);
            glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
            glBindBuffer(GL_COPY_READ_BUFFER, 0);
            glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

            glDeleteBuffers(1, &m_handle);
            m_handle = handle;
            m_bufferSize = std::max(100u, (start + size) * 3 / 2);
        }

        glBufferSubData((GLenum)m_type, start, size, pData);
        m_dataSize = std::max(m_dataSize, start + size);
    }

    ItemInfo* GraphicsVBO::create() const
    {
        return new VBOInfo(this);
    }

    ItemInfo* GraphicsUBO::create() const
    {
        return new UBOInfo(this);
    }

} // namespace FX
