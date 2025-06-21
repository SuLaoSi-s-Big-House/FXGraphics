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

    void GraphicsBuffer::setRebuildStart(int start)
    {
        for (auto&& itr : m_itemList)
        {
            static_cast<BufferInfo*>(itr.second)->setRebuildStart(start);
        }
    }

    void GraphicsBuffer::addDirtyList(const std::set<int>& dirtyList)
    {
        for (auto&& itr : m_itemList)
        {
            static_cast<BufferInfo*>(itr.second)->addDirtyList(dirtyList);
        }
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

    int BufferInfo::rebuildStart() const
    {
        return m_rebuildStart;
    }

    const std::set<int>& BufferInfo::dirtyList() const
    {
        return m_dirtyList;
    }

    void BufferInfo::cleanDirty()
    {
        if (m_rebuildStart < 0 || m_dirtyList.empty())
        {
            return;
        }

        auto itr = m_dirtyList.lower_bound(m_rebuildStart);
        m_dirtyList.erase(itr, m_dirtyList.end());
    }

    void BufferInfo::setReady()
    {
        m_rebuildStart = -1;
        m_dirtyList.clear();
    }

    void BufferInfo::setRebuildStart(int start)
    {
        if (m_rebuildStart < 0 || start < m_rebuildStart)
        {
            m_rebuildStart = start;
        }
    }

    void BufferInfo::addDirtyList(const std::set<int>& dirtyList)
    {
        m_dirtyList.insert(dirtyList.begin(), dirtyList.end());
    }

    ItemInfo* GraphicsVBO::create() const
    {
        return new VBOInfo(this);
    }

    ItemInfo* GraphicsEBO::create() const
    {
        return new EBOInfo(this);
    }

    ItemInfo* GraphicsSSBO::create() const
    {
        return new SSBOInfo(this);
    }

    ItemInfo* GraphicsDIBO::create() const
    {
        return new DIBOInfo(this);
    }

    ItemInfo* GraphicsUBO::create() const
    {
        return new UBOInfo(this);
    }

    ItemInfo* GraphicsVAO::create() const
    {
        return new VAOInfo(this);
    }

    VAOInfo::VAOInfo(const GraphicsVAO* pOwner) : ItemInfo(pOwner)
    {
        glGenVertexArrays(1, &m_handle);
    }

    VAOInfo::~VAOInfo()
    {
        glDeleteVertexArrays(1, &m_handle);
    }

    void VAOInfo::bind() const
    {
        glBindVertexArray(m_handle);
    }

    void VAOInfo::unbind() const
    {
        glBindVertexArray(0);
    }

} // namespace FX
