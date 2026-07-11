#include <random>
#include <array>
#include <glm.hpp>
#include "graphics_window.h"
#include "graphics_entity.h"
#include "graphics_scene.h"
#include "graphics_printer.h"
#include "graphics_camera.h"
#include "graphics_font_manager.h"
#include "graphics_material_manager.h"
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
        FX::Material material = FX::GraphicsMaterialManager::instance().get(m_materialHandle);
        material.baseColor = { 0, 0, 0, 255 };
        FX::GraphicsMaterialManager::instance().unref(m_materialHandle);
        m_materialHandle = FX::GraphicsMaterialManager::instance().ref(material);
        setDirty(FX::MaterialDirty);
    }
    explicit Edge(float x, float y, float z, float radius) : GraphicsEntity(FX::NormalLineStripID),
        m_position({ x, y, z, radius })
    {
        FX::Material material = FX::GraphicsMaterialManager::instance().get(m_materialHandle);
        material.baseColor = { 0, 0, 0, 255 };
        FX::GraphicsMaterialManager::instance().unref(m_materialHandle);
        m_materialHandle = FX::GraphicsMaterialManager::instance().ref(material);
        setDirty(FX::MaterialDirty);
    }

    ~Edge(void) = default;

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
        if (!FX::Math::isEqual(m_scale, scale))
        {
            m_scale = scale;
            setMatrix(glm::scale(glm::mat4(1.0f), glm::vec3(scale)));
        }
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
            glm::vec3 tangent  = glm::cross(d, glm::normalize(base_pts[i] - base_center));
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

            m_vertex.insert(m_vertex.end(), {tip.x, tip.y, tip.z});
            m_normal.insert(m_normal.end(), {tip_n.x, tip_n.y, tip_n.z});

            m_vertex.insert(m_vertex.end(), {base_pts[i].x, base_pts[i].y, base_pts[i].z});
            m_normal.insert(m_normal.end(), {side_normals[i].x, side_normals[i].y, side_normals[i].z});

            m_vertex.insert(m_vertex.end(), {base_pts[j].x, base_pts[j].y, base_pts[j].z});
            m_normal.insert(m_normal.end(), {side_normals[j].x, side_normals[j].y, side_normals[j].z});

            m_index.insert(m_index.end(), {idx, idx + 1, idx + 2});
            idx += 3;
        }

        glm::vec3 base_normal = -d;
        for (int i = 0; i < segments; ++i) {
            int j = (i + 1) % segments;

            m_vertex.insert(m_vertex.end(), {base_center.x, base_center.y, base_center.z});
            m_normal.insert(m_normal.end(), {base_normal.x, base_normal.y, base_normal.z});

            m_vertex.insert(m_vertex.end(), {base_pts[j].x, base_pts[j].y, base_pts[j].z});
            m_normal.insert(m_normal.end(), {base_normal.x, base_normal.y, base_normal.z});

            m_vertex.insert(m_vertex.end(), {base_pts[i].x, base_pts[i].y, base_pts[i].z});
            m_normal.insert(m_normal.end(), {base_normal.x, base_normal.y, base_normal.z});

            m_index.insert(m_index.end(), {idx, idx + 1, idx + 2});
            idx += 3;
        }

        m_uv = m_vertex;
    }

    void setScale(float scale)
    {
        if (!FX::Math::isEqual(m_scale, scale))
        {
            m_scale = scale;
            setMatrix(glm::scale(glm::mat4(1.0f), glm::vec3(scale)));
        }
    }

private:
    FX::vec3f m_direction;
    float m_scale = 1.0f;
};

std::random_device rDevice;
std::mt19937 rEngine(rDevice());
std::uniform_int_distribution<> range1(0, 99999);
std::uniform_int_distribution<> range2(0, 255);

int main(void)
{
    auto name1 = FX::GraphicsFontManager::instance().loadFontFile("./font/HarmonyOS_Sans_SC_Regular.ttf");
    auto name2 = FX::GraphicsFontManager::instance().loadFontFile("./font/AlibabaPuHuiTi-3-55-Regular.ttf");

    FX::GraphicsWindow window1(800, 600);
    window1.use();
    window1.frame();

    std::vector<Box*> boxs;
    std::vector<Edge*> edges;
    boxs.resize(100000);
    edges.resize(100000);
    for (int i = 0; i < boxs.size(); i++)
    {
        boxs[i] = new Box(static_cast<float>((i % 10000) / 100), static_cast<float>((i % 10000) % 100), static_cast<float>(i / 10000),
            0.2f + 0.4f * std::sin(i * static_cast<float>(FX::Math::PI)));
        edges[i] = new Edge(static_cast<float>((i % 10000) / 100), static_cast<float>((i % 10000) % 100), static_cast<float>(i / 10000),
            0.2f + 0.4f * std::sin(i * static_cast<float>(FX::Math::PI)));
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
    for (int i = 0; i < boxs.size(); i++)
    {
        scene.addEntity(boxs[i]);
        scene.addEntity(edges[i]);
    }

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
        FX::Material material = text1.material();
        material.font = { name1, 16 };
        material.baseColor = { 0, 200, 255, 255 };
        text1.setMaterial(material);
        scene.addEntity(&text1);
        text1.setPosition({ 5, 5 });
    }

    FX::SurfTextEntity text2;
    {
        FX::Material material = text2.material();
        material.font = { name1, 12 };
        material.baseColor = { 200, 200, 200, 255 };
        text2.setMaterial(material);
        scene.addEntity(&text2);
        text2.setPosition({ 5, 545 });
    }

    std::vector<int> visibleList;
    visibleList.resize(1000, 0);
    int n = 0;
    long long sumT = 0;
    int frames = 0;
    int fps = 0;
    auto last = std::chrono::high_resolution_clock::now();
    while (!window1.shouldClose())
    {
        auto now = std::chrono::high_resolution_clock::now();
        auto dur = std::chrono::duration_cast<std::chrono::microseconds>(now - last).count();
        last = now;
        sumT += dur;

        auto& interactor = window1.interactor();
        auto flag = interactor.enventFlag();

        {
            if (sumT >= 5e5)
            {
                fps = static_cast<int>(1e6f * frames / sumT + 0.5f);
            }

            std::string str = "FPS: " + std::to_string(fps) + "\n";

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
            }
        }

        if (n % 10 == 0)
        {
            //for (int i = 0; i < 1000; i++)
            //{
            //    auto j = range1(rEngine);
            //    FX::Material material = boxs[j]->material();
            //    material.baseColor = {
            //        static_cast<unsigned char>(range2(rEngine)),
            //        static_cast<unsigned char>(range2(rEngine)),
            //        static_cast<unsigned char>(range2(rEngine)),
            //        static_cast<unsigned char>(range2(rEngine)),
            //    };
            //    boxs[j]->setMaterial(material);
            //}

            for (auto j : visibleList)
            {
                FX::Material material = boxs[j]->material();
                material.visible = true;
                boxs[j]->setMaterial(material);
            }

            for (int i = 0; i < 1000; i++)
            {
                auto j = range1(rEngine);
                FX::Material material = boxs[j]->material();
                material.visible = false;
                boxs[j]->setMaterial(material);
                visibleList[i] = j;
            }
        }

        window1.use();
        scene.draw();
        window1.frame();

        if (sumT >= 5e5)
        {
            sumT = frames = 0;
        }

        n++;
        frames++;
    }

    for (int i = 0; i < boxs.size(); i++)
    {
        delete boxs[i];
        delete edges[i];
    }
}
