#include "graphics_window.h"
#include "graphics_entity.h"
#include "graphics_scene.h"
#include "graphics_printer.h"
#include "graphics_font_manager.h"
#include "surf_text_item.h"

int main(void)
{
    FX::GraphicsWindow window(800, 600, "", false);
    window.use();
    window.frame();

    auto name1 = FX::GraphicsFontManager::instance().loadFontFile("./font/AlibabaPuHuiTi-3-55-Regular.ttf");
    //auto name2 = FX::GraphicsFontManager::instance().loadFontFile("./font/BlueakaBeta-DB-GBK.ttf");
    auto name2 = FX::GraphicsFontManager::instance().loadFontFile("C:/Windows/Fonts/SIMYOU.TTF");

    FX::GraphicsFontManager::instance().generate({ name1, 64 }, "1234567890+-");
    FX::GraphicsFontManager::instance().generate({ name2, 24 }, "1234567890+-");

    FX::TextEntity text1;
    FX::TextEntity text2;

    FX::EntityProfile profile;
    profile.font = { name1, 64 };
    profile.color = { 100, 150, 255, 255 };
    text1.setProfile(profile);
    text1.setPosition({ 0, 0 });
    text1.setText("■_°①∫\n123$%^我他龘");

    profile.font = { name2, 24 };
    profile.color = { 255, 255, 255, 255 };
    text2.setProfile(profile);
    text2.setPosition({ 200, 300 });
    text2.setText("1234567890\n泛函极值： 最速降线问题要求的是一个函数（曲线 y(x)）\n使得另一个依赖于这个函数的量（下滑时间 T[y(x)]） 达到最小值\n时间 T 是一个泛函（Functional），即“函数的函数”。");

    FX::GraphicsNormalPrinter printer;
    std::ifstream ifs;
    ifs.open("./shader/normal_text.vert");
    printer.addShader(FX::GPUItemType::kVtxShader, ifs);
    ifs.close();
    ifs.open("./shader/normal_text.frag");
    printer.addShader(FX::GPUItemType::kFrgShader, ifs);
    ifs.close();

    FX::GraphicsScene scene;

    scene.addPrinter(&printer, FX::ScreenTextID);

    scene.addEntity(&text1);
    scene.addEntity(&text2);

    int i = 0;
    while (!window.shouldClose())
    {
        i++;
        profile.font.size = 10 + (i % 2000) / 100;
        text2.setProfile(profile);

        scene.draw();
        window.frame();
    }
}
