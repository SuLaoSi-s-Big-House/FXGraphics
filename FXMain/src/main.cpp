#include "graphics_window.h"
#include "graphics_printer.h"
#include "graphics_font_manager.h"
#include "graphics_scene.h"
#include "surf_text_item.h"
#include "graphics_interactor.h"
#include "graphics_camera.h"

class Box : public FX::GraphicsEntity {
public:
    Box(void) : GraphicsEntity(FX::NormalFaceID) {}

    void generate(void) override
    {
        m_vertex = {
            0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1,
            1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1,
            1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1,
            0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1,
            0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1,
            0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0
        };

        m_normal = {
            0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0,
            1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0,
            0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0,
            -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0,
            0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1,
            0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1
        };

        m_uv = m_normal;

        m_index.resize(36);
        for (int i = 0; i < 36; i++)
        {
            m_index[i] = i;
        }
    }
};

class Sphere : public FX::GraphicsEntity {
public:
    Sphere(void) : GraphicsEntity(FX::NormalFaceStripID)
    {
        m_profile.matrix = {
            0.7, 0, 0, 0,
            0, 0.7, 0, 0,
            0, 0, 0.7, 0,
            -0.5, 0.7, 1.5, 1,
        };
    }

    void generate(void) override
    {
        m_vertex.resize(16 * 32 * 3);
        m_normal.resize(16 * 32 * 3);
        for (int i = 0; i < 32; i++)
        {
            for (int j = 0; j < 16; j++)
            {
                FX::vec3f pos = {
                    std::sin(static_cast<float>(FX::Math::PI * i / 16)) * std::sin(static_cast<float>(FX::Math::PI * j / 15)),
                    std::cos(static_cast<float>(FX::Math::PI * i / 16)) * std::sin(static_cast<float>(FX::Math::PI * j / 15)),
                    std::cos(static_cast<float>(FX::Math::PI * j / 15)) };
                m_vertex[(16 * i + j) * 3] = m_normal[(16 * i + j) * 3] = pos.x;
                m_vertex[(16 * i + j) * 3 + 1] = m_normal[(16 * i + j) * 3 + 1] = pos.y;
                m_vertex[(16 * i + j) * 3 + 2] = m_normal[(16 * i + j) * 3 + 2] = pos.z;
            }
        }
        m_uv = m_normal;

        m_index.resize(16 * 32 * 2 + 32);
        for (int i = 0; i < 32; i++)
        {
            for (int j = 0; j < 16; j++)
            {
                m_index[(16 * i + j) * 2 + i] = 16 * ((i + 1) % 32) + j;
                m_index[(16 * i + j) * 2 + i + 1] = 16 * i + j;
            }
            m_index[(16 * i + 16) * 2 + i] = FX::RestartMark;
        }
    }
};

class Cylinder : public FX::GraphicsEntity {
public:
    Cylinder(void) : GraphicsEntity(FX::NormalFaceStripID) {}
};

class Axis : public FX::GraphicsEntity {
public:
    Axis(FX::vec4uc color, FX::vec3f dir) : GraphicsEntity(FX::NormalLineID), m_dir(dir)
    {
        m_profile.color = color;
    }

    void generate(void) override
    {
        m_uv = m_normal = m_vertex = { 0, 0, 0, 100 * m_dir.x, 100 * m_dir.y, 100 * m_dir.z };
        m_index = { 0, 1 };
    }

private:
    FX::vec3f m_dir;
};

int main(void)
{
    auto name1 = FX::GraphicsFontManager::instance().loadFontFile("./font/AlibabaPuHuiTi-3-55-Regular.ttf");
    auto name2 = FX::GraphicsFontManager::instance().loadFontFile("./font/BlueakaBeta-DB-GBK.ttf");
    //auto name2 = FX::GraphicsFontManager::instance().loadFontFile("C:/Windows/Fonts/SIMYOU.TTF");
    FX::GraphicsFontManager::instance().generate({ name1, 20 }, "0123456789,.[](): FPS窗口大小鼠标位置在内正在拖拽");
    FX::GraphicsFontManager::instance().generate({ name2, 14 }, "鼠标左中右侧键12滚轮: ↑↓");

    FX::TextEntity texts[14];
    int time[6] = { 60, 60, 60, 60, 60, 60 };
    FX::EntityProfile profile;
    profile.font = { name1, 20 };
    profile.color = { 100, 150, 255, 255 };
    for (int i = 0; i < 5; i++)
    {
        texts[i].setProfile(profile);
        texts[i].setPosition({ 3, 20 * i });
    }
    profile.font = { name2, 14 };
    profile.color = { 255, 150, 150, 255 };
    for (int i = 0; i < 6; i++)
    {
        texts[i + 5].setProfile(profile);
        texts[i + 5].setPosition({ 720, 20 * i + 4 });
    }
    profile.font = { name1, 20 };
    profile.color = { 200, 200, 200, 255 };
    for (int i = 0; i < 3; i++)
    {
        texts[i + 11].setProfile(profile);
        texts[i + 11].setPosition({ 3, 20 * i + 540 });
    }
    texts[0].setText("FPS: 0");
    texts[1].setText("窗口大小: [0, 0]");
    texts[2].setText("鼠标位置: (0, 0)");
    texts[3].setText("鼠标在内");
    texts[4].setText("正在拖拽");
    texts[5].setText("鼠标左键");
    texts[6].setText("鼠标右键");
    texts[7].setText("鼠标中键");
    texts[8].setText("鼠标侧键1");
    texts[9].setText("鼠标侧键2");
    texts[10].setText("鼠标滚轮: ");
    texts[11].setText("fov: ");
    texts[12].setText("相机 α: ");
    texts[13].setText("相机 β: ");

    FX::GraphicsWindow window(800, 600);
    window.use();
    window.frame();

    FX::GraphicsNormalPrinter printer[3];
    std::ifstream ifs;
    ifs.open("./shader/screen_text.vert");
    printer[0].addShader(FX::GPUItemType::kVtxShader, ifs);
    ifs.close();
    ifs.open("./shader/screen_text.frag");
    printer[0].addShader(FX::GPUItemType::kFrgShader, ifs);
    ifs.close();
    ifs.open("./shader/normal_world.vert");
    printer[1].addShader(FX::GPUItemType::kVtxShader, ifs);
    ifs.close();
    ifs.open("./shader/normal_world_phong.frag");
    printer[1].addShader(FX::GPUItemType::kFrgShader, ifs);
    ifs.close();
    ifs.open("./shader/normal_world.vert");
    printer[2].addShader(FX::GPUItemType::kVtxShader, ifs);
    ifs.close();
    ifs.open("./shader/normal_world.frag");
    printer[2].addShader(FX::GPUItemType::kFrgShader, ifs);
    ifs.close();

    FX::GraphicsScene scene;
    scene.addPrinter(printer, FX::ScreenTextID);
    scene.addPrinter(printer + 1, FX::NormalFaceID);
    scene.addPrinter(printer + 1, FX::NormalFaceStripID);
    scene.addPrinter(printer + 2, FX::NormalLineID);
    for (int i = 0; i < 14; i++)
    {
        scene.addEntity(texts + i);
    }
    Box box;
    scene.addEntity(&box);
    Sphere sphere;
    scene.addEntity(&sphere);
    Axis axiss[3] = { {{255, 0, 0, 255}, {1, 0, 0}}, {{0, 255, 0, 255}, {0, 1, 0}}, {{0, 0, 255, 255}, {0, 0, 1}} };
    scene.addEntity(axiss);
    scene.addEntity(axiss + 1);
    scene.addEntity(axiss + 2);

    float fov1 = static_cast<float>(FX::Math::PI / 6);
    float fov2 = static_cast<float>(FX::Math::PI / 6);
    float speed1 = 5e-6f;
    float alpha1 = 30;
    float alpha2 = 30;
    float beta1 = 30;
    float beta2 = 30;
    bool dragFinished = true;
    FX::GraphicsCamera camera;
    scene.bindCamera(&camera);

    auto last = std::chrono::high_resolution_clock::now();
    int frames = 0;
    int sum = 0;
    int i = 0;

    while (!window.shouldClose())
    {
        auto now = std::chrono::high_resolution_clock::now();
        auto dur = std::chrono::duration_cast<std::chrono::microseconds>(now - last).count();
        last = now;
        sum += dur;
        i += dur;
        if (sum >= 5e5)
        {
            frames = (int)(1e6 * frames / sum + 0.5);
            texts[0].setText("FPS: " + std::to_string(frames));
            sum = 0;
            frames = 0;
        }

        auto size = window.size();
        texts[1].setText("窗口大小: [" + std::to_string(size.x) + ", " + std::to_string(size.y) + "]");

        if (window.interactor().isCursorIn())
        {
            auto pos = window.interactor().cursorPos();
            texts[2].setText("鼠标位置: (" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ")");
        }

        profile = texts[3].profile();
        profile.visible = window.interactor().isCursorIn();
        texts[3].setProfile(profile);

        auto flag = window.interactor().enventFlag();

        profile = texts[4].profile();
        profile.visible = (flag & FX::MouseDragFlag);
        texts[4].setProfile(profile);

        for (int i = 0; i < 5; i++)
        {
            window.interactor().isButtonPressing(FX::MouseButton(i)) ? time[i] = 6e5 : time[i] -= dur;
        }

        for (int i = 0; i < 6; i++)
        {
            profile = texts[i + 5].profile();
            profile.color.a = std::max(0, std::min(255, (int)(5e-4 * time[i])));
            texts[i + 5].setProfile(profile);
            auto pos = texts[i + 5].position();
            pos.x = size.x - 80;
            texts[i + 5].setPosition(pos);
        }

        for (int i = 0; i < 3; i++)
        {
            auto pos = texts[i + 11].position();
            pos.y = size.y - 60 + 20 * i;
            texts[i + 11].setPosition(pos);
        }

        if (flag & FX::MouseScrollFlag)
        {
            time[5] = 6e5;
            auto s = window.interactor().mouseScroll();
            s > 0 ? texts[10].setText("鼠标滚轮: ↑") : texts[10].setText("鼠标滚轮: ↓");

            fov2 -= 6 * static_cast<float>(FX::Math::PI / 180 * s);
            fov2 = std::max(std::min(fov2, static_cast<float>(FX::Math::PI / 3)), static_cast<float>(FX::Math::PI / 18));
        }
        else
        {
            time[5] -= dur;
        }

        for (int i = 0; i < 6; i++)
        {
            if (time[i] < 0)
            {
                time[i] = 0;
            }
        }

        if (!FX::Math::isEqual(fov1, fov2))
        {
            if (std::abs(fov1 - fov2) < speed1 * dur)
            {
                fov1 = fov2;
            }
            else
            {
                fov1 > fov2 ? fov1 -= speed1 * dur : fov1 += speed1 * dur;
            }
        }
        camera.setField(fov1, 1.0f * size.x / size.y, 0.1f, 1000.0f);

        if (flag & FX::MouseDragFlag)
        {
            auto& drag = window.interactor().dragInfo();
            if (drag.button == FX::MouseButton::kLeft)
            {
                auto pos = window.interactor().cursorPos();
                alpha1 = alpha2 - (pos.x - drag.startPos.x) * 0.5f;
                beta1 = beta2 + (pos.y - drag.startPos.y) * 0.4f;
                dragFinished = false;
            }
            else
            {
                alpha2 = alpha1;
                beta2 = beta1;
                dragFinished = true;
            }
        }
        else if (dragFinished == false)
        {
            alpha2 = alpha1;
            beta2 = beta1;
            dragFinished = true;
        }

        {
            beta1 = std::max(std::min(beta1, 90.0f), -90.0f);
            FX::vec3f pos = {
                10 * std::sin(static_cast<float>(FX::Math::PI * alpha1 / 180)) * std::cos(static_cast<float>(FX::Math::PI * beta1 / 180)),
                10 * std::sin(static_cast<float>(FX::Math::PI * beta1 / 180)),
                10 * std::cos(static_cast<float>(FX::Math::PI * alpha1 / 180)) * std::cos(static_cast<float>(FX::Math::PI * beta1 / 180))
            };
            camera.setPosition(pos);
            auto up = glm::normalize(glm::cross(glm::vec3(pos.x, pos.y, pos.z),
                glm::vec3(std::cos(static_cast<float>(FX::Math::PI * alpha1 / 180)), 0, -std::sin(static_cast<float>(FX::Math::PI * alpha1 / 180)))));
            camera.setUp({ up.x, up.y, up.z });
        }
        
        texts[11].setText("fov: " + std::to_string(fov1 * 180 / static_cast<float>(FX::Math::PI)));
        texts[12].setText("相机 α: " + std::to_string(alpha1));
        texts[13].setText("相机 β: " + std::to_string(beta1));

        scene.draw();
        window.frame();

        frames++;
    }
}
