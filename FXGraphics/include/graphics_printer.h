#ifndef _GRAPHICS_PRINTER_H_
#define _GRAPHICS_PRINTER_H_

#include <vector>
#include <memory>

#include "graphics_shader_program.h"

namespace FX {

    // GraphicsPrinter类型，对应OpenGL的图元类型
    enum class PrintType : unsigned int {
        kPoints = 0x0000,
        kLines = 0x0001,
        kLineStrip = 0x0003,
        kLineStripAdj = 0x000B,
        kTriangles = 0x0004,
        kTriangleStrip = 0x0005,
        kTriangleStripAdj = 0x000D,
    };

    using PrintPipeline = unsigned int;
    constexpr PrintPipeline NormalOpaquePipeline = 0;
    constexpr PrintPipeline NormalTransPipeline = 1;
    // For users: 用户可以添加自己的PrintPipeline，用于自己的GraphicsPrinter派生类

    // 此文件定义了GraphicsPrinter
    // GraphicsPrinter是对OpenGL program的拓展，可以存放多个GraphicsProgram与多个GraphicsShader，组成多种渲染管线。
    // GraphicsPrinter负责切换OpenGL状态与绑定GraphicsProgram，也负责更新脏的GraphicsProgram与GraphicsShader。

    // For users:
    // 用户可以从GraphicsPrinter派生，以实现特定的渲染方式。
    // 用户需要首先向GraphicsPrinter中添加GraphicsProgram与GraphicsShader，并将其加入到GraphicsScene，才能真正用于渲染。

    class GraphicsPrinter {
    protected:
        explicit GraphicsPrinter(PrintType type) : m_type(type) {}
        virtual ~GraphicsPrinter(void) = default;

    public:
        // For users:
        // 派生类需要实现use函数，通常包含切换OpenGL状态与绑定GraphicsProgram。
        // 入参pipe可以帮助判断切换到哪一种状态。
        // 此函数会在渲染过程中由GraphicsScene调用，用户通常不需要主动调用。
        virtual void use(PrintPipeline pipe = NormalOpaquePipeline) = 0;

        virtual void draw(unsigned int num) const;

        virtual void addProgram(void);
        virtual void addShader(GPUItemType type, const std::ifstream& file);
        virtual void addShader(GPUItemType type, const std::string& source);

    protected:
        // For users:
        // 在绑定GraphicsProgram时，如果确认GraphicsProgram为脏，调用此函数重新生成GraphicsProgram并绑定。
        // 入参program与shaders均表示在vector中的下标。
        virtual void _use(unsigned int program, const std::vector<unsigned int>& shaders);

        virtual void _updateReady(void);
        virtual void _dirtyPrograms(void);
        void _useOpaque(void) const;
        void _useTrans(void) const;

    protected:
        std::vector<std::unique_ptr<GraphicsProgram>> m_programs;
        std::vector<std::unique_ptr<GraphicsShader>> m_shaders;
        PrintType m_type = PrintType::kPoints;
        bool m_ready = false;
    };


    // 基础的GraphicsPrinter，包含一个GraphicsProgram和若干GraphicsShader，无法继续添加GraphicsProgram。
    class GraphicsNormalPrinter : public GraphicsPrinter {
    public:
        explicit GraphicsNormalPrinter(PrintType type);

        void use(PrintPipeline pipe = NormalOpaquePipeline) override;

    private:
        void addProgram(void) override {}
    };

} // namespace FX

#endif // _GRAPHICS_PRINTER_H_
