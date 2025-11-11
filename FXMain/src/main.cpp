#include <fstream>
#include "graphics_window.h"
#include "graphics_printer.h"

int main()
{
    FX::GraphicsNormalPrinter printer1(FX::PrintType::kPoints);

    FX::GraphicsWindow window1(800, 600);
    FX::GraphicsWindow window2(400, 300);
    window1.use();

    FX::GraphicsNormalPrinter printer2(FX::PrintType::kTriangles);

    std::ifstream ifs;
    ifs.open("./shader/normal_world.vert");
    printer1.addShader(FX::GPUItemType::kVtxShader, ifs);
    ifs.close();
    ifs.open("./shader/normal_world.vert");
    printer2.addShader(FX::GPUItemType::kVtxShader, ifs);
    ifs.close();
    ifs.open("./shader/normal_world.frag");
    printer1.addShader(FX::GPUItemType::kFrgShader, ifs);
    ifs.close();
    ifs.open("./shader/normal_world.frag");
    printer2.addShader(FX::GPUItemType::kFrgShader, ifs);
    ifs.close();

    window1.use();
    window1.frame();

    printer1.use();

    window1.frame();

    window2.use();

    printer1.use();
    printer2.use();

    window2.frame();

    window1.use();
}
