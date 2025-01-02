#include "graphics_window.h"
#include "graphics_printer.h"

constexpr char* V =
"#version 430 core\n"
"layout (location = 0) in vec3 position;\n"
"void main()\n"
"{\n"
"gl_Position = vec4(position, 1.0);\n"
"}\n\n";

constexpr char* F =
"#version 430 core\n"
"out vec4 fs_color;\n"
"void main()\n"
"{\n"
"fs_color = vec4(1.0, 1.0, 1.0, 1.0);\n"
"}\n\n";

int main(void)
{
    FX::GraphicsNormalPrinter printer1(FX::PrintType::kTriangles);
    printer1.addShader(FX::GPUItemType::kVtxShader, V);
    printer1.addShader(FX::GPUItemType::kFrgShader, F);

    FX::GraphicsWindow window1(800, 600);
    window1.use();

    gladLoadGL();

    printer1.use();

    FX::GraphicsNormalPrinter printer2(FX::PrintType::kTriangles);
    printer2.addShader(FX::GPUItemType::kVtxShader, V);
    printer2.addShader(FX::GPUItemType::kFrgShader, F);

    printer2.use();

    FX::GraphicsWindow window2(800, 600);
    window2.use();

    printer1.use();

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    window2.frame();

    window1.use();
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    window1.frame();
}
