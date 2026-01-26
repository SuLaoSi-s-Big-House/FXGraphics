#include "graphics_window.h"
#include "graphics_printer.h"
#include "graphics_font_manager.h"
#include "graphics_scene.h"
#include "surf_text_item.h"
#include "graphics_interactor.h"

int main(void)
{
    auto name1 = FX::GraphicsFontManager::instance().loadFontFile("./font/AlibabaPuHuiTi-3-55-Regular.ttf");
    auto name2 = FX::GraphicsFontManager::instance().loadFontFile("./font/BlueakaBeta-DB-GBK.ttf");
    //auto name2 = FX::GraphicsFontManager::instance().loadFontFile("C:/Windows/Fonts/SIMYOU.TTF");

    FX::TextEntity fps;
    FX::TextEntity extent;
    FX::TextEntity cursor1;
    FX::TextEntity cursor2;
    FX::TextEntity drag;
    FX::TextEntity mouse[5];
    FX::TextEntity scroll;
    int time[6] = { 60, 60, 60, 60, 60, 60 };

    FX::EntityProfile profile;
    profile.font = { name1, 32 };
    profile.color = { 100, 150, 255, 255 };

    fps.setProfile(profile);
    extent.setProfile(profile);
    cursor1.setProfile(profile);
    cursor2.setProfile(profile);
    drag.setProfile(profile);

    profile.font = { name2, 24 };
    profile.color = { 255, 150, 150, 255 };
    for (int i = 0; i < 5; i++)
    {
        mouse[i].setProfile(profile);
        mouse[i].setPosition({ 450, 40 * i + 10 });
    }
    scroll.setProfile(profile);

    fps.setPosition({ 0, 0 });
    fps.setText("FPS: 0");
    extent.setPosition({ 0, 35 });
    extent.setText("窗口大小: [0, 0]");
    cursor1.setPosition({ 0, 70 });
    cursor1.setText("鼠标位置: (0, 0)");
    cursor2.setPosition({ 0, 105 });
    cursor2.setText("鼠标在内");
    drag.setPosition({ 0, 140 });
    drag.setText("正在拖拽");

    mouse[0].setText("鼠标左键");
    mouse[1].setText("鼠标右键");
    mouse[2].setText("鼠标中键");
    mouse[3].setText("鼠标侧键1");
    mouse[4].setText("鼠标侧键2");

    scroll.setPosition({ 450, 210 });
    scroll.setText("鼠标滚轮: ");

    FX::GraphicsWindow window(800, 600, "", false);
    window.use();
    window.frame();

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

    scene.addEntity(&fps);
    scene.addEntity(&extent);
    scene.addEntity(&cursor1);
    scene.addEntity(&cursor2);
    scene.addEntity(&drag);
    for (int i = 0; i < 5; i++)
    {
        scene.addEntity(mouse + i);
    }
    scene.addEntity(&scroll);

    auto last = std::chrono::high_resolution_clock::now();
    int frames = 0;
    int sum = 0;

    while (!window.shouldClose())
    {
        auto now = std::chrono::high_resolution_clock::now();
        auto dur = std::chrono::duration_cast<std::chrono::microseconds>(now - last).count();
        last = now;
        sum += dur;
        if (sum >= 5e5)
        {
            frames = (int)(1e6 * frames / sum + 0.5);
            fps.setText("FPS: " + std::to_string(frames));
            sum = 0;
            frames = 0;
        }

        auto size = window.size();
        extent.setText("窗口大小: [" + std::to_string(size.x) + ", " + std::to_string(size.y) + "]");

        if (window.interactor().isCursorIn())
        {
            auto pos = window.interactor().cursorPos();
            cursor1.setText("鼠标位置: (" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ")");
        }

        profile = cursor2.profile();
        profile.visible = window.interactor().isCursorIn();
        cursor2.setProfile(profile);

        for (int i = 0; i < 5; i++)
        {
            if (window.interactor().isButtonPressing(FX::GraphicsInteractor::MouseButton(i)))
            {
                time[i] = 6e5;
            }
            else if (time[i] > 0)
            {
                time[i] -= dur;
            }
        }

        for (int i = 0; i < 5; i++)
        {
            profile = mouse[i].profile();
            profile.color.a = std::max(0, std::min(255, (int)(5e-4 * time[i])));
            mouse[i].setProfile(profile);
        }

        auto flag = window.interactor().enventFlag();

        if (flag & FX::MouseScrollFlag)
        {
            time[5] = 6e5;
            auto s = window.interactor().mouseScroll();
            if (s > 0)
            {
                scroll.setText("鼠标滚轮：↑");
            }
            else
            {
                scroll.setText("鼠标滚轮：↓");
            }
        }
        else
        {
            if (time[5] > 0)
            {
                time[5] -= dur;
            }
        }

        profile = scroll.profile();
        profile.color.a = std::max(0, std::min(255, (int)(5e-4 * time[5])));
        scroll.setProfile(profile);

        profile = drag.profile();
        profile.visible = (flag & FX::MouseDragFlag);
        drag.setProfile(profile);

        scene.draw();
        window.frame();

        frames++;
    }
}
