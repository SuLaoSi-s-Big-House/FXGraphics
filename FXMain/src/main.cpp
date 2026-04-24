#include <random>
#include "graphics_window.h"
#include "graphics_entity.h"
#include "graphics_scene.h"
#include "graphics_printer.h"
#include "graphics_camera.h"
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


std::random_device rDevice;
std::mt19937 rEngine(rDevice());
std::uniform_int_distribution<> range(0, 7);

int main(void)
{
    FX::GraphicsWindow window1(800, 600);
    window1.use();
    window1.frame();

    FX::GraphicsWindow window2(800, 600);
    window2.use();
    window2.frame();

    Box* boxs[8] = {};
    for (int i = 0; i < 8; i++)
    {
        boxs[i] = new Box(5 * std::sin(2 * 3.1415926f * i / 8), 5 * std::cos(2 * 3.1415926f * i / 8), 0.0f, 1.0f);
    }

    FX::GraphicsNormalPrinter printer;
    std::ifstream ifs;
    ifs.open("./shader/normal_world.vert");
    printer.addShader(FX::GPUItemType::kVtxShader, ifs);
    ifs.close();
    ifs.open("./shader/normal_world.frag");
    printer.addShader(FX::GPUItemType::kFrgShader, ifs);
    ifs.close();

    FX::GraphicsScene scene;
    scene.addPrinter(&printer, FX::NormalFaceStripID);
    for (int i = 0; i < 8; i++)
    {
        scene.addEntity(boxs[i]);
    }

    FX::GraphicsCamera camera;
    camera.setLookAt({ 0.0f, 0.0f, 0.0f });
    camera.setField(-40.0f, 40.0f, -30.0f, 30.0f);
    scene.bindCamera(&camera);

    int i = 0;
    int n = 0;
    while (!window1.shouldClose() && !window2.shouldClose())
    {
        glm::vec3 position = glm::vec3(5 * std::sin(n / 60.0f), 3 * std::cos(n / 60.0f), 5 * std::cos(n / 60.0f));
        camera.setPosition({ position.x, position.y, position.z });
        glm::normalize(position);
        glm::vec3 rightDir = glm::normalize(glm::vec3(-position.z, 0.0f, position.x));
        glm::vec3 upDir = glm::normalize(glm::cross(position, rightDir));
        camera.setUp({ upDir.x, upDir.y, upDir.z });

        if (n % 10 == 0)
        {
            scene.addEntity(boxs[i]);
            i = range(rEngine);
            scene.removeEntity(boxs[i]);
        }

        window1.use();
        scene.draw();
        window1.frame();

        if (n % 60 == 0)
        {
            window2.use();
            scene.draw();
            window2.frame();
        }

        n++;
    }

    for (int i = 0; i < 8; i++)
    {
        delete boxs[i];
    }
}
