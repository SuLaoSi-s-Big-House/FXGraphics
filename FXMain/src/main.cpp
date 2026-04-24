//#include "graphics_window.h"
#include "graphics_entity.h"
#include "graphics_scene.h"
#include "graphics_printer.h"
#include "graphics_camera.h"
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

    scene1.addEntity(&box1);
    scene1.addEntity(&box2);
    scene1.addEntity(&box3);
    scene1.addEntity(&box4);
    scene1.addEntity(&box5);

    FX::LogicObserveCamera camera(window);
    scene1.bindCamera(&camera.get());

    FX::BasicBounding<> box;
    box.expand(FX::vec3f{ -4, -4, -4 });
    box.expand(FX::vec3f{ 5, 5, 5 });
    camera.observe(box);

    int i = 0;
    while (!window.shouldClose())
    {
        camera.process();
        scene1.draw();
        window.frame();
        i++;
    }
}
