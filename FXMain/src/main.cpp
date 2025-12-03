#include "graphics_window.h"
#include "graphics_entity.h"
#include "graphics_scene.h"
#include "basic_vector.h"
#include "graphics_printer.h"
#include "graphics_font_manager.h"
#include "surf_text_item.h"

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
    auto a = FX::GraphicsFontManager::instance().loadFontFile("./font/AlibabaPuHuiTi-3-55-Regular.ttf");

    FX::GraphicsWindow window(800, 600);
    window.use();
    window.frame();

    FX::GraphicsNormalPrinter printer(FX::PrintType::kTriangleStrip);
    std::ifstream ifs;
    ifs.open("./shader/normal_text.vert");
    printer.addShader(FX::GPUItemType::kVtxShader, ifs);
    ifs.close();
    ifs.open("./shader/normal_text.frag");
    printer.addShader(FX::GPUItemType::kFrgShader, ifs);
    ifs.close();

    FX::GraphicsScene scene1;

    scene1.addPrinter(&printer, FX::NormalTextID);

    FX::TextEntity text1;
    text1.setText("abcd我他123^%&");

    FX::EntityProfile profile;
    profile.font.name = a;
    profile.font.size = 32;

    text1.setProfile(profile);

    scene1.addEntity(&text1);

    while (!window.shouldClose())
    {
        scene1.draw();
        window.frame();
    }
}
