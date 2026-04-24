#ifndef _GRAPHICS_SHADER_PROGRAM_H_
#define _GRAPHICS_SHADER_PROGRAM_H_

#include <fstream>
#include <string>

#include "graphics_gpu_item.h"

namespace FX {

    // 此文件定义了GraphicsShader、ShaderInfo、GraphicsProgram、ProgramInfo。
    // 对Opengl program与Opengl shader进行了封装，并添加了常用的实现如shader compile、program link等。

    // For users:
    // 如果用户需要实现特定的管线，应当从GraphicsPrinter派生并实现相应的函数，而不应当直接修改这个文件中的类。

    class GraphicsShader : public GraphicsGPUItem {
    public:
        GraphicsShader(GPUItemType type, const std::ifstream& file);
        GraphicsShader(GPUItemType type, const std::string& source);
        ~GraphicsShader(void) = default;

        const char* source(void) const;

    protected:
        ItemInfo* create(void) const override;

    protected:
        std::string m_source;
    };


    class ShaderInfo : public ItemInfo {
    protected:
        friend class GraphicsShader;

        explicit ShaderInfo(const GraphicsShader* pOwner);
        ~ShaderInfo(void) override;

    public:
        bool isDirty(void) const;
        void setDirty(bool isDirty = true);

        void compile(void) const;

    protected:
        bool m_dirty = true;
    };

    class GraphicsPrinter;
    class ProgramInfo;

    class GraphicsProgram : public GraphicsGPUItem {
    public:
        friend class GraphicsPrinter;

        GraphicsProgram(void) : GraphicsGPUItem(GPUItemType::kProgram) {}
        ~GraphicsProgram(void) = default;

        void setDirty(bool isDirty = true);

        const ProgramInfo* get(void) const;

    protected:
        ItemInfo* create(void) const override;
    };


    class ProgramInfo : public ItemInfo {
    protected:
        friend class GraphicsProgram;

        explicit ProgramInfo(const GraphicsProgram* pOwner);
        ~ProgramInfo(void) override;

    public:
        bool isDirty(void) const;
        void setDirty(bool isDirty = true);

        void use(void) const;
        void link(void) const;

    protected:
        bool m_dirty = true;
    };

} // namespace FX

#endif // _GRAPHICS_SHADER_PROGRAM_H_
