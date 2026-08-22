#include <condition_variable>
#include <mutex>
#include <random>
#include <thread>
#include <glm.hpp>
#include "graphics_window.h"
#include "graphics_entity.h"
#include "graphics_scene.h"
#include "graphics_printer.h"
#include "graphics_camera.h"
#include "graphics_font_manager.h"
#include "surf_text_item.h"
#include "logic_camera.h"
#include "basic_vector.h"

class Box : public FX::GraphicsEntity {
public:
    Box(void) : GraphicsEntity(FX::NormalFaceStripID), m_position({ 0.0f, 0.0f, 0.0f, 1.0f }) {}
    explicit Box(float x, float y, float z, float radius) : GraphicsEntity(FX::NormalFaceStripID),
        m_position({ x, y, z, radius }) {}

    ~Box(void) = default;

    void generate(void) override
    {
        m_vertex = {
            m_position.x + m_position.w, m_position.y - m_position.w, m_position.z + m_position.w,
            m_position.x + m_position.w, m_position.y - m_position.w, m_position.z - m_position.w,
            m_position.x + m_position.w, m_position.y + m_position.w, m_position.z + m_position.w,
            m_position.x + m_position.w, m_position.y + m_position.w, m_position.z - m_position.w,
            m_position.x - m_position.w, m_position.y - m_position.w, m_position.z - m_position.w,
            m_position.x - m_position.w, m_position.y - m_position.w, m_position.z + m_position.w,
            m_position.x - m_position.w, m_position.y + m_position.w, m_position.z - m_position.w,
            m_position.x - m_position.w, m_position.y + m_position.w, m_position.z + m_position.w,
            m_position.x + m_position.w, m_position.y + m_position.w, m_position.z - m_position.w,
            m_position.x - m_position.w, m_position.y + m_position.w, m_position.z - m_position.w,
            m_position.x + m_position.w, m_position.y + m_position.w, m_position.z + m_position.w,
            m_position.x - m_position.w, m_position.y + m_position.w, m_position.z + m_position.w,
            m_position.x + m_position.w, m_position.y - m_position.w, m_position.z - m_position.w,
            m_position.x + m_position.w, m_position.y - m_position.w, m_position.z + m_position.w,
            m_position.x - m_position.w, m_position.y - m_position.w, m_position.z - m_position.w,
            m_position.x - m_position.w, m_position.y - m_position.w, m_position.z + m_position.w,
            m_position.x - m_position.w, m_position.y - m_position.w, m_position.z + m_position.w,
            m_position.x + m_position.w, m_position.y - m_position.w, m_position.z + m_position.w,
            m_position.x - m_position.w, m_position.y + m_position.w, m_position.z + m_position.w,
            m_position.x + m_position.w, m_position.y + m_position.w, m_position.z + m_position.w,
            m_position.x + m_position.w, m_position.y - m_position.w, m_position.z - m_position.w,
            m_position.x - m_position.w, m_position.y - m_position.w, m_position.z - m_position.w,
            m_position.x + m_position.w, m_position.y + m_position.w, m_position.z - m_position.w,
            m_position.x - m_position.w, m_position.y + m_position.w, m_position.z - m_position.w,
        };
        m_normal = {
            1, 0, 0,  1, 0, 0,  1, 0, 0,  1, 0, 0,
            -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0,
            0, 1, 0,  0, 1, 0,  0, 1, 0,  0, 1, 0,
            0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0,
            0, 0, 1,  0, 0, 1,  0, 0, 1,  0, 0, 1,
            0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1,
        };
        m_uv = m_vertex;
        m_index = {
            0,  1,  2,  3,  FX::RestartMark,
            4,  5,  6,  7,  FX::RestartMark,
            8,  9,  10, 11, FX::RestartMark,
            12, 13, 14, 15, FX::RestartMark,
            16, 17, 18, 19, FX::RestartMark,
            20, 21, 22, 23
        };
    }

private:
    FX::vec4f m_position;
};

class Edge : public FX::GraphicsEntity {
public:
    Edge(void) : GraphicsEntity(FX::NormalLineStripID), m_position({ 0.0f, 0.0f, 0.0f, 1.0f })
    {
        m_profile.color = { 0, 0, 0, 255 };
    }
    explicit Edge(float x, float y, float z, float radius) : GraphicsEntity(FX::NormalLineStripID),
        m_position({ x, y, z, radius })
    {
        m_profile.color = { 0, 0, 0, 255 };
    }

    ~Edge(void) = default;

    void generate(void) override
    {
        m_vertex = {
            m_position.x + m_position.w, m_position.y - m_position.w, m_position.z + m_position.w,
            m_position.x + m_position.w, m_position.y - m_position.w, m_position.z - m_position.w,
            m_position.x + m_position.w, m_position.y + m_position.w, m_position.z + m_position.w,
            m_position.x + m_position.w, m_position.y + m_position.w, m_position.z - m_position.w,
            m_position.x - m_position.w, m_position.y - m_position.w, m_position.z + m_position.w,
            m_position.x - m_position.w, m_position.y - m_position.w, m_position.z - m_position.w,
            m_position.x - m_position.w, m_position.y + m_position.w, m_position.z + m_position.w,
            m_position.x - m_position.w, m_position.y + m_position.w, m_position.z - m_position.w,
        };
        m_normal = m_vertex;
        m_uv = m_vertex;
        m_index = {
            0, 1, 3, 2, 0, 4, 5, 7, 6, 4, FX::RestartMark,
            1, 5, 7, 3, 2, 6
        };
    }

private:
    FX::vec4f m_position;
};


class AxisLine : public FX::GraphicsEntity {
public:
    explicit AxisLine(const FX::vec3f& dir) : GraphicsEntity(FX::NormalLineID), m_direction(dir) {}

    void generate(void) override
    {
        m_vertex = { 0, 0, 0, m_direction.x * 5.0f * m_scale, m_direction.y * 5.0f * m_scale, m_direction.z * 5.0f * m_scale };
        m_normal = m_vertex;
        m_uv = m_vertex;
        m_index = { 0, 1 };
    }

    void setScale(float scale)
    {
        scale *= 5;
        m_scale = scale;
        m_profile.matrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale));
        setDirty(FX::MatrixDirty);
    }

private:
    FX::vec3f m_direction;
    float m_scale = 1.0f;
};

class AxisCone : public FX::GraphicsEntity {
public:
    explicit AxisCone(const FX::vec3f& dir) : GraphicsEntity(FX::NormalFaceID), m_direction(dir) {}

    void generate(void) override
    {
        const float height = 1.0f;
        const float radius = 0.3f;
        const int   segments = 16;

        float h = height * m_scale;
        float r = radius * m_scale;

        glm::vec3 d = glm::normalize(glm::vec3(m_direction.x, m_direction.y, m_direction.z));
        glm::vec3 tip = d * 5.0f * m_scale;
        glm::vec3 base_center = tip - d * h;

        glm::vec3 u;
        if (std::abs(d.x) < 0.9f)
            u = glm::normalize(glm::cross(d, glm::vec3(1.0f, 0.0f, 0.0f)));
        else
            u = glm::normalize(glm::cross(d, glm::vec3(0.0f, 1.0f, 0.0f)));
        glm::vec3 v = glm::normalize(glm::cross(d, u));

        std::vector<glm::vec3> base_pts(segments);
        std::vector<glm::vec3> side_normals(segments);
        for (int i = 0; i < segments; ++i) {
            float angle = 2.0f * glm::pi<float>() * static_cast<float>(i) / static_cast<float>(segments);
            base_pts[i] = base_center + r * (std::cos(angle) * u + std::sin(angle) * v);
            glm::vec3 edge_dir = glm::normalize(base_pts[i] - tip);
            glm::vec3 tangent = glm::cross(d, glm::normalize(base_pts[i] - base_center));
            side_normals[i] = glm::normalize(glm::cross(edge_dir, tangent));
        }

        m_vertex.clear();
        m_normal.clear();
        m_uv.clear();
        m_index.clear();

        unsigned int idx = 0;

        for (int i = 0; i < segments; ++i) {
            int j = (i + 1) % segments;
            glm::vec3 tip_n = glm::normalize(side_normals[i] + side_normals[j]);

            m_vertex.insert(m_vertex.end(), { tip.x, tip.y, tip.z });
            m_normal.insert(m_normal.end(), { tip_n.x, tip_n.y, tip_n.z });

            m_vertex.insert(m_vertex.end(), { base_pts[i].x, base_pts[i].y, base_pts[i].z });
            m_normal.insert(m_normal.end(), { side_normals[i].x, side_normals[i].y, side_normals[i].z });

            m_vertex.insert(m_vertex.end(), { base_pts[j].x, base_pts[j].y, base_pts[j].z });
            m_normal.insert(m_normal.end(), { side_normals[j].x, side_normals[j].y, side_normals[j].z });

            m_index.insert(m_index.end(), { idx, idx + 1, idx + 2 });
            idx += 3;
        }

        glm::vec3 base_normal = -d;
        for (int i = 0; i < segments; ++i) {
            int j = (i + 1) % segments;

            m_vertex.insert(m_vertex.end(), { base_center.x, base_center.y, base_center.z });
            m_normal.insert(m_normal.end(), { base_normal.x, base_normal.y, base_normal.z });

            m_vertex.insert(m_vertex.end(), { base_pts[j].x, base_pts[j].y, base_pts[j].z });
            m_normal.insert(m_normal.end(), { base_normal.x, base_normal.y, base_normal.z });

            m_vertex.insert(m_vertex.end(), { base_pts[i].x, base_pts[i].y, base_pts[i].z });
            m_normal.insert(m_normal.end(), { base_normal.x, base_normal.y, base_normal.z });

            m_index.insert(m_index.end(), { idx, idx + 1, idx + 2 });
            idx += 3;
        }

        m_uv = m_vertex;
    }

    void setScale(float scale)
    {
        scale *= 5;
        m_scale = scale;
        m_profile.matrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale));
        setDirty(FX::MatrixDirty);
    }

private:
    FX::vec3f m_direction;
    float m_scale = 1.0f;
};

struct RamdomColor {
    unsigned int index = 0;
    FX::vec4uc color;
};

struct RamdomRotate {
    unsigned int index = 0;
    unsigned int rotate = 0;
};

std::vector<RamdomColor> colorList;
std::vector<RamdomRotate> rotateList;
std::vector<unsigned int> visibleList;

std::mutex randomMutex;
std::condition_variable randomCv;

bool stop = false;
bool work = true;

void randomFunc(void)
{
    std::random_device rDevice;
    std::mt19937 rEngine(rDevice());
    std::uniform_int_distribution<> range1(0, 999999);
    std::uniform_int_distribution<> range2(0, 510);

    while (true)
    {
        colorList.resize(10000);
        rotateList.resize(10000);
        visibleList.resize(10000);

        for (int i = 0; i < 10000; i++)
        {
            colorList[i].index = range1(rEngine);
            colorList[i].color = {
                static_cast<unsigned char>(std::min(range2(rEngine), 255)),
                static_cast<unsigned char>(std::min(range2(rEngine), 255)),
                static_cast<unsigned char>(std::min(range2(rEngine), 255)),
                static_cast<unsigned char>(std::min(range2(rEngine), 255))
            };

            rotateList[i].index = range1(rEngine);
            rotateList[i].rotate = range1(rEngine);

            visibleList[i] = range1(rEngine);
        }

        {
            std::lock_guard<std::mutex> lock(randomMutex);
            work = false;
        }

        randomCv.notify_one();

        std::unique_lock<std::mutex> lock(randomMutex);
        randomCv.wait(lock, []() { return stop || work; });

        if (stop)
        {
            return;
        }
    }
}


int main(void)
{
    auto name1 = FX::GraphicsFontManager::instance().loadFontFile("./font/HarmonyOS_Sans_SC_Regular.ttf");
    auto name2 = FX::GraphicsFontManager::instance().loadFontFile("./font/AlibabaPuHuiTi-3-55-Regular.ttf");
    auto name3 = FX::GraphicsFontManager::instance().loadFontFile("./font/BlueakaBeta-DB-GBK.ttf");

    std::vector<Box*> boxs;
    std::vector<Edge*> edges;
    boxs.resize(1000000);
    edges.resize(1000000);
    for (int i = 0; i < boxs.size(); i++)
    {
        boxs[i] = new Box(static_cast<float>((i % 10000) / 100), static_cast<float>((i % 10000) % 100), static_cast<float>(i / 10000), 0.25f);
        edges[i] = new Edge(static_cast<float>((i % 10000) / 100), static_cast<float>((i % 10000) % 100), static_cast<float>(i / 10000), 0.25f);
    }

    FX::GraphicsNormalPrinter printer1;
    std::ifstream ifs;
    ifs.open("./shader/normal_world.vert");
    printer1.addShader(FX::GPUItemType::kVtxShader, ifs);
    ifs.close();
    ifs.open("./shader/normal_world.frag");
    printer1.addShader(FX::GPUItemType::kFrgShader, ifs);
    ifs.close();

    FX::GraphicsNormalPrinter printer2;
    ifs.open("./shader/normal_world.vert");
    printer2.addShader(FX::GPUItemType::kVtxShader, ifs);
    ifs.close();
    ifs.open("./shader/normal_world_phong.frag");
    printer2.addShader(FX::GPUItemType::kFrgShader, ifs);
    ifs.close();

    FX::GraphicsNormalPrinter printer3;
    ifs.open("./shader/normal_screen_text.vert");
    printer3.addShader(FX::GPUItemType::kVtxShader, ifs);
    ifs.close();
    ifs.open("./shader/normal_screen_text.frag");
    printer3.addShader(FX::GPUItemType::kFrgShader, ifs);
    ifs.close();

    FX::GraphicsScene scene;
    scene.addPrinter(&printer2, FX::NormalFaceStripID);
    scene.addPrinter(&printer1, FX::NormalFaceID);
    scene.addPrinter(&printer1, FX::NormalLineStripID);
    scene.addPrinter(&printer1, FX::NormalLineID);
    scene.addPrinter(&printer3, FX::ScreenTextID);

    FX::GraphicsWindow window1(800, 600, "100w正方体", true);
    window1.use();
    window1.frame();

    AxisLine axisX({ 1, 0, 0 });
    AxisCone coneX({ 1, 0, 0 });
    AxisLine axisY({ 0, 1, 0 });
    AxisCone coneY({ 0, 1, 0 });
    AxisLine axisZ({ 0, 0, 1 });
    AxisCone coneZ({ 0, 0, 1 });

    scene.addEntity(&axisX);
    scene.addEntity(&coneX);
    scene.addEntity(&axisY);
    scene.addEntity(&coneY);
    scene.addEntity(&axisZ);
    scene.addEntity(&coneZ);

    FX::LogicObserveCamera camera(&window1);
    scene.bindCamera(&camera.get());

    FX::BasicBounding<> box;
    box.expand(FX::vec3f{ 120, 120, 120 });
    box.expand(FX::vec3f{ -20, -20, -20 });
    camera.observe(box);

    FX::SurfTextEntity text1;
    {
        text1.setText("FPS: 0\n窗口大小: [800, 600]");
        FX::EntityProfile profile = text1.profile();
        profile.font = { name1, 16 };
        profile.color = { 0, 200, 255, 255 };
        text1.setProfile(profile);
        scene.addEntity(&text1);
        text1.setPosition({ 5, 5 });
        text1.setDepth(-0.99f);
    }

    FX::SurfTextEntity text2;
    {
        text2.setText("观察中心: (0, 0, 0)\n相机方向: (0, 0, 0)\n相机距离: 0\n缩放比例: 0");
        FX::EntityProfile profile = text2.profile();
        profile.font = { name1, 12 };
        profile.color = { 200, 200, 200, 255 };
        text2.setProfile(profile);
        scene.addEntity(&text2);
        text2.setPosition({ 5, 545 });
        text2.setDepth(-0.99f);
    }

    FX::SurfTextEntity text3;
    FX::SurfTextEntity text4;
    {
        text3.setText("Loading...");
        FX::EntityProfile profile = text3.profile();
        profile.font = { name3, 20 };
        profile.color = { 255, 150, 0, 255 };
        text3.setProfile(profile);
        scene.addEntity(&text3);
        text3.setPosition({ (800 - 85) / 2, 75 });
        text3.setDepth(-0.99f);

        text4.setText("每100毫秒改变随机10000个正方体的颜色，10000个可见性，10000个矩阵");
        profile = text4.profile();
        profile.font = { name1, 18 };
        profile.color = { 200, 200, 200, 255 };
        text4.setProfile(profile);
        scene.addEntity(&text4);
        text4.setPosition({ (800 - 570) / 2, 110});
        text4.setDepth(-0.99f);
    }

    scene.draw();
    window1.frame();

    for (int i = 0; i < boxs.size(); i++)
    {
        scene.addEntity(boxs[i]);
        scene.addEntity(edges[i]);
    }

    std::thread randomThread(&randomFunc);

    text3.setText("性能测试，100w正方体");
    text3.setPosition({ (800 - 210) / 2, 77 });

    long long sumT = 0;
    long long updateT = 0;
    int frames = 0;
    int fps = 0;
    auto last = std::chrono::high_resolution_clock::now();
    while (!window1.shouldClose())
    {
        auto now = std::chrono::high_resolution_clock::now();
        auto dur = std::chrono::duration_cast<std::chrono::microseconds>(now - last).count();
        last = now;
        sumT += dur;
        updateT += dur;

        auto& interactor = window1.interactor();
        auto flag = interactor.enventFlag();

        {
            if (sumT >= 5e5)
            {
                fps = static_cast<int>(1e6f * frames / sumT * 10 + 0.5f);
            }

            std::string str = "FPS: " + std::to_string(fps / 10) + "." + std::to_string(fps % 10) + "\n";

            auto size = window1.size();
            str += "窗口大小: [" + std::to_string(size.x) + ", " + std::to_string(size.y) + "]\n";

            if (interactor.isCursorIn())
            {
                auto pos = interactor.cursorPos();
                str += "鼠标位置: (" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ")\n";
            }
            else
            {
                str += "\n";
            }

            if (flag & FX::MouseDragFlag)
            {
                str += "正在拖拽\n";
            }

            text1.setText(str);
        }

        camera.process();
        axisX.setScale(camera.scale());
        coneX.setScale(camera.scale());
        axisY.setScale(camera.scale());
        coneY.setScale(camera.scale());
        axisZ.setScale(camera.scale());
        coneZ.setScale(camera.scale());

        {
            auto& cam = camera.get();
            auto& position = cam.position();
            auto& lookAt = cam.lookAt();
            glm::vec3 dir = glm::vec3(position.x - lookAt.x, position.y - lookAt.y, position.z - lookAt.z);
            auto dis = glm::length(dir);
            dir = glm::normalize(dir);

            std::string str = "观察中心: (" + std::to_string(lookAt.x) + ", " + std::to_string(lookAt.y) + ", " + std::to_string(lookAt.z) + ")\n";
            str += "相机方向: (" + std::to_string(dir.x) + ", " + std::to_string(dir.y) + ", " + std::to_string(dir.z) + ")\n";
            str += "相机距离: " + std::to_string(dis) + "\n";
            str += "缩放比例: " + std::to_string(camera.scale());

            text2.setText(str);

            if (flag & FX::WindowResizeFlag)
            {
                auto size = window1.size();
                text2.setPosition({ 5, size.y - 55 });
                text3.setPosition({ (size.x - 210) / 2, 75});
                text4.setPosition({ (size.x - 570) / 2, 100});
            }
        }

        if (updateT >= 100000)
        {
            updateT -= 100000;

            std::unique_lock<std::mutex> lock(randomMutex);
            randomCv.wait(lock, []() { return !work; });

            for (auto& data : colorList)
            {
                FX::EntityProfile profile = boxs[data.index]->profile();
                profile.color = data.color;
                boxs[data.index]->setProfile(profile);
            }

            for (auto& i : visibleList)
            {
                FX::EntityProfile profile = boxs[i]->profile();
                profile.visible = (i % 2) == 0;
                boxs[i]->setProfile(profile);

            }

            for (auto& data : rotateList)
            {
                auto position = glm::vec3(static_cast<float>((data.index % 10000) / 100), static_cast<float>((data.index % 10000) % 100), static_cast<float>(data.index / 10000));
                auto matrix1 = glm::translate(glm::mat4(1.0f), -position);
                auto matrix2 = (data.rotate > 500000) ? glm::mat4(1.0f) : glm::rotate(glm::mat4(1.0f), 2 * 3.1415926f * (data.rotate / 500000.0f), glm::vec3(1.0f));
                auto matrix3 = glm::translate(glm::mat4(1.0f), position);
                FX::EntityProfile profile = boxs[data.index]->profile();
                profile.matrix = matrix3 * matrix2 * matrix1;
                boxs[data.index]->setProfile(profile);
                profile = edges[data.index]->profile();
                profile.matrix = matrix3 * matrix2 * matrix1;
                edges[data.index]->setProfile(profile);
            }

            work = true;
            lock.unlock();
            randomCv.notify_one();
        }

        window1.use();
        scene.draw();
        window1.frame();

        if (sumT >= 5e5)
        {
            sumT = frames = 0;
        }

        frames++;
    }

    {
        std::lock_guard<std::mutex> lock(randomMutex);
        stop = true;
    }
    randomCv.notify_one();
    randomThread.join();

    for (int i = 0; i < boxs.size(); i++)
    {
        delete boxs[i];
        delete edges[i];
    }
}
