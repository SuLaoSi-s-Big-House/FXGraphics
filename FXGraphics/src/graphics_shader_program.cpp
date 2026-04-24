#include "graphics_shader_program.h"

#include <assert.h>
#include <sstream>
#include <iostream>
#include "glad.h"
#include "basic_log.h"
#include "graphics_window.h"

namespace FX {

    GraphicsShader::GraphicsShader(GPUItemType type, const std::ifstream& file) : GraphicsGPUItem(type)
    {
        assert((type == GPUItemType::kFrgShader) || (type == GPUItemType::kVtxShader) || (type == GPUItemType::kGeoShader));

        if (file.is_open())
        {
            std::stringstream buffer;
            buffer << file.rdbuf();
            auto source = buffer.str();

            if (source.empty())
            {
                assert(0);
                BasicLog::out(BasicLog::kWarn, "The shader file is empty, please check your file.");
                return;
            }

            int i = 0;    // 检查字符串是否带有文件前缀
            for (; i < source.length(); i++)
            {
                if (std::isprint(static_cast<unsigned char>(source[i])))
                {
                    break;
                }
            }

            if (i < source.length())
            {
                m_source = source.substr(i);
            }
            else
            {
                assert(0);
                BasicLog::out(BasicLog::kWarn, "The shader file is empty, please check your file.");
            }
        }
        else
        {
            assert(0);
            BasicLog::out(BasicLog::kWarn, "Unopened fstream, leaving an empty shader source.");
        }
    }

    GraphicsShader::GraphicsShader(GPUItemType type, const std::string& source) : GraphicsGPUItem(type)
    {
        assert((type == GPUItemType::kFrgShader) || (type == GPUItemType::kVtxShader) || (type == GPUItemType::kGeoShader));
        m_source = source;

        if (source.empty())
        {
            assert(0);
            BasicLog::out(BasicLog::kWarn, "Shader received an empty string.");
        }
    }

    const char* GraphicsShader::source() const
    {
        return m_source.c_str();
    }

    ItemInfo* GraphicsShader::create() const
    {
        return new ShaderInfo(this);
    }

    ShaderInfo::ShaderInfo(const GraphicsShader* pOwner) : ItemInfo(pOwner)
    {
        assert((m_type == GPUItemType::kFrgShader) || (m_type == GPUItemType::kVtxShader) || (m_type == GPUItemType::kGeoShader));
        m_handle = glCreateShader((GLenum)m_type);
        assert(m_handle != 0);
    }

    ShaderInfo::~ShaderInfo()
    {
        glDeleteShader(m_handle);
    }

    bool ShaderInfo::isDirty() const
    {
        return m_dirty;
    }

    void ShaderInfo::setDirty(bool isDirty)
    {
        m_dirty = isDirty;
    }

    void ShaderInfo::compile() const
    {
        auto pSource = static_cast<const GraphicsShader*>(m_pOwner)->source();
        glShaderSource(m_handle, 1, &pSource, nullptr);
        glCompileShader(m_handle);
#if defined(_DEBUG) || defined(DEBUG)
        int success = 0;
        glGetShaderiv(m_handle, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            constexpr unsigned int LogLength = 1024;
            char log[LogLength] = { 0 };
            glGetShaderInfoLog(m_handle, LogLength, nullptr, log);
            BasicLog::out(BasicLog::kError, "SHADER COMPILE FAILED!");
            std::cout << log << std::endl;
        }
#endif
    }

    void GraphicsProgram::setDirty(bool isDirty)
    {
        for (auto&& itr : m_itemList)
        {
            static_cast<ProgramInfo*>(itr.second)->setDirty(isDirty);
        }
    }

    const ProgramInfo* GraphicsProgram::get() const
    {
        auto pWindow = GraphicsWindow::currentWindow();
        if (pWindow == nullptr)
        {
            assert(pWindow);
            BasicLog::out(BasicLog::kWarn, "No window is used, cannot get the program info.");
            return nullptr;
        }

        auto itr = m_itemList.find(pWindow);
        return itr == m_itemList.end() ? nullptr : static_cast<const ProgramInfo*>(itr->second);
    }

    ItemInfo* GraphicsProgram::create() const
    {
        return new ProgramInfo(this);
    }

    ProgramInfo::ProgramInfo(const GraphicsProgram* pOwner) : ItemInfo(pOwner)
    {
        m_handle = glCreateProgram();
        assert(m_handle != 0);
    }

    ProgramInfo::~ProgramInfo()
    {
        glDeleteProgram(m_handle);
    }

    bool ProgramInfo::isDirty() const
    {
        return m_dirty;
    }

    void ProgramInfo::setDirty(bool isDirty)
    {
        m_dirty = isDirty;
    }

    void ProgramInfo::use() const
    {
        glUseProgram(m_handle);
    }

    void ProgramInfo::link() const
    {
        glLinkProgram(m_handle);
#if defined(_DEBUG) || defined(DEBUG)
        int success = 0;
        glGetProgramiv(m_handle, GL_LINK_STATUS, &success);
        if (!success)
        {
            constexpr unsigned int LogLength = 1024;
            char log[LogLength] = { 0 };
            glGetProgramInfoLog(m_handle, LogLength, nullptr, log);
            BasicLog::out(BasicLog::kError, "PROGRAM LINK FAILED!");
            std::cout << log << std::endl;
        }
#endif
    }

} // namespace FX
