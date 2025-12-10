#include "graphics_window.h"
#include "graphics_entity.h"
#include "graphics_scene.h"
#include "graphics_printer.h"

class Box : public FX::GraphicsEntity {
public:
    Box(void) : GraphicsEntity(FX::NormalFaceStripID), m_position({ 0.0f, 0.0f, 0.0f, 1.0f }) {}
    explicit Box(float x, float y, float z, float radius) : GraphicsEntity(FX::NormalFaceStripID),
        m_position({ x, y, z, radius }) {}

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

    Box box1;
    Box box2(1.0f, 2.0f, 3.0f, 0.5f);

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

    Box box3(4.0f, 4.0f, 4.0f, 0.3f);
    Box box4(-1.0f, -2.0f, -3.0f, 1.0f);
    Box box5(-3.0f, -3.0f, -1.0f, 0.5f);

    while (!window.shouldClose())
    {
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
    }
}
