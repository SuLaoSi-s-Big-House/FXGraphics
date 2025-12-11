#include "graphics_window.h"
#include "graphics_entity.h"
#include "graphics_scene.h"
#include "graphics_printer.h"
#include "basic_vector.h"
#include "glm.hpp"
#include "ext/matrix_transform.inl"

class Box : public FX::GraphicsEntity {
public:
    Box(void) : GraphicsEntity(FX::NormalFaceStripID), m_position({ 0.0f, 0.0f, 0.0f, 1.0f }) {}
    Box(float x, float y, float z, float radius) : GraphicsEntity(FX::NormalFaceStripID), m_position({ x, y, z, radius }) {}

    ~Box(void) = default;

    void generate(void) override
    {
        m_vertex = {
            m_position.x - m_position.w, m_position.y - m_position.w, m_position.z - m_position.w,
            m_position.x - m_position.w, m_position.y - m_position.w, m_position.z + m_position.w,
            m_position.x - m_position.w, m_position.y + m_position.w, m_position.z - m_position.w,
            m_position.x - m_position.w, m_position.y + m_position.w, m_position.z + m_position.w,
            m_position.x + m_position.w, m_position.y - m_position.w, m_position.z - m_position.w,
            m_position.x + m_position.w, m_position.y - m_position.w, m_position.z + m_position.w,
            m_position.x + m_position.w, m_position.y + m_position.w, m_position.z - m_position.w,
            m_position.x + m_position.w, m_position.y + m_position.w, m_position.z + m_position.w
        };
        m_normal = m_vertex;
        m_uv = m_vertex;

        m_index = { 6, 4, 2, 0, 3, 1, 7, 5, FX::RestartMark, 2, 3, 6, 7, 4, 5, 0, 1 };
    }

private:
    FX::vec4f m_position;
};



int main(void)
{
    FX::GraphicsWindow window(800, 600);
    window.use();
    window.frame();

    Box box1(-4.0f, 0.0f, 0.0f, 0.5f);
    Box box2(-2.0f, 0.0f, 0.0f, 0.5f);
    Box box3(0.0f, 0.0f, 0.0f, 0.5f);
    Box box4(2.0f, 0.0f, 0.0f, 0.5f);
    Box box5(4.0f, 0.0f, 0.0f, 0.5f);

    FX::GraphicsNormalPrinter printer;
    std::ifstream ifs;
    ifs.open("./shader/normal_world.vert");
    printer.addShader(FX::GPUItemType::kVtxShader, ifs);
    ifs.close();
    ifs.open("./shader/normal_world.frag");
    printer.addShader(FX::GPUItemType::kFrgShader, ifs);
    ifs.close();

    FX::GraphicsScene scene1;

    scene1.addPrinter(&printer, FX::NormalFaceStripID);

    scene1.addEntity(&box1);
    scene1.addEntity(&box2);
    scene1.addEntity(&box3);
    scene1.removeEntity(&box2);
    scene1.addEntity(&box4);
    scene1.addEntity(&box5);
    scene1.addEntity(&box1);

    FX::EntityProfile profile;
    profile.color = { 255, 0, 0, 255 };
    box1.setProfile(profile);

    int i = 0;

    while (!window.shouldClose())
    {
        profile = box1.profile();
        profile.color.g = profile.color.b = i % 255;
        box1.setProfile(profile);

        profile = box2.profile();
        profile.matrix = glm::rotate(glm::mat4(1.0f), 3.1415926f * i / 180, glm::vec3(1.0f, 1.0f, 1.0f));
        box5.setProfile(profile);

        scene1.addEntity(&box1);
        scene1.addEntity(&box2);
        scene1.addEntity(&box3);
        scene1.removeEntity(&box2);
        scene1.addEntity(&box4);
        scene1.addEntity(&box5);
        scene1.addEntity(&box1);

        scene1.draw();
        window.frame();

        scene1.removeEntity(&box1);
        scene1.removeEntity(&box2);
        scene1.removeEntity(&box3);
        scene1.removeEntity(&box4);
        scene1.removeEntity(&box5);

        scene1.draw();
        window.frame();

        scene1.addEntity(&box2);
        scene1.addEntity(&box3);
        scene1.addEntity(&box4);
        scene1.addEntity(&box5);

        scene1.draw();
        window.frame();

        scene1.removeEntity(&box5);
        scene1.removeEntity(&box4);

        scene1.draw();
        window.frame();

        scene1.addEntity(&box4);
        scene1.addEntity(&box5);
        scene1.removeEntity(&box5);
        scene1.removeEntity(&box3);

        scene1.draw();
        window.frame();

        i++;
    }
}
