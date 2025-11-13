#include "graphics_window.h"
#include "graphics_entity.h"
#include "graphics_scene.h"
#include "basic_vector.h"
#include "graphics_printer.h"

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

    FX::GraphicsNormalPrinter printer(FX::PrintType::kTriangleStrip);
    std::ifstream ifs;
    ifs.open("./shader/normal.vert");
    printer.addShader(FX::GPUItemType::kVtxShader, ifs);
    ifs.close();
    ifs.open("./shader/normal.frag");
    printer.addShader(FX::GPUItemType::kFrgShader, ifs);
    ifs.close();

    FX::GraphicsScene scene;

    scene.addPrinter(&printer, FX::NormalFaceStripID);

    scene.addEntity(&box1);
    scene.addEntity(&box2);
    scene.addEntity(&box3);
    scene.removeEntity(&box2);
    scene.addEntity(&box4);
    scene.addEntity(&box5);
    scene.addEntity(&box1);

    scene.draw();
    window.frame();

    scene.removeEntity(&box1);
    scene.removeEntity(&box2);
    scene.removeEntity(&box3);
    scene.removeEntity(&box4);
    scene.removeEntity(&box5);

    scene.draw();
    window.frame();

    scene.addEntity(&box2);
    scene.addEntity(&box3);
    scene.addEntity(&box4);
    scene.addEntity(&box5);

    scene.draw();
    window.frame();

    scene.removeEntity(&box5);
    scene.removeEntity(&box4);

    scene.draw();
    window.frame();

    FX::EntityProfile profile;
    profile.color = { 255, 0, 0, 255 };

    scene.addEntity(&box1);
    scene.addEntity(&box2);
    scene.addEntity(&box3);
    scene.addEntity(&box4);
    scene.addEntity(&box5);

    box1.setProfile(profile);
    box2.setProfile(profile);

    scene.draw();
    window.frame();

    profile.color = { 0, 255, 0, 100 };

    box3.setProfile(profile);
    box4.setProfile(profile);

    scene.draw();
    window.frame();
}
