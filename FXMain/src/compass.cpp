#include "compass.h"

#include "glad.h"
#include "graphics_window.h"
#include "graphics_camera.h"

namespace {

    const vec4uc CREAM_COLOR = { 255, 238, 210, 255 };
    const vec4uc CHEESE_COLOR = { 183, 154, 107, 255 };

    struct NormalGlobalInfo {
        glm::mat4 vMatrix = glm::mat4(1.0f);
        glm::mat4 pMatrix = glm::mat4(1.0f);
        glm::mat4 vpMatrix = glm::mat4(1.0f);
        vec2i viewport = { 0, 0 };
    };

}  // namespace

void CompassScene::clear()
{
    glDepthMask(GL_TRUE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void CompassScene::bindGlobal()
{
    NormalGlobalInfo info;

    if (m_pCamera == nullptr)
    {
        info.vMatrix = info.pMatrix = info.vpMatrix = GraphicsCamera::defaultVPMatrix();
    }
    else
    {
        info.vMatrix = m_pCamera->vMatrix();
        info.pMatrix = m_pCamera->pMatrix();
        info.vpMatrix = m_pCamera->vPMatrix();
    }

    auto pWindow = GraphicsWindow::currentWindow();
    assert(pWindow != nullptr);
    auto size = pWindow->size();
    info.viewport = { COMPASS_SIZE, COMPASS_SIZE };

    auto pUbo = static_cast<UBOInfo*>(m_globalUbo.getOrCreate());
    assert(pUbo != nullptr);
    pUbo->bind();
    pUbo->setData(sizeof(info), &info);
    pUbo->bind(0);

    glViewport(size.x - COMPASS_SIZE, size.y - COMPASS_SIZE, COMPASS_SIZE, COMPASS_SIZE);
}

CompassPlane::CompassPlane() : GraphicsEntity(NormalFaceStripID)
{
    m_profile.color = CREAM_COLOR;
}

void CompassPlane::generate()
{
    m_vertex.resize(32 * 3);
    m_normal.resize(32 * 3);
    for (int i = 0; i < 32; i++)
    {
        m_vertex[i * 3] = static_cast<float>(std::sin((i / 32.0f) * 2 * Math::PI));
        m_vertex[i * 3 + 1] = 0;
        m_vertex[i * 3 + 2] = static_cast<float>(std::cos((i / 32.0f) * 2 * Math::PI));
        m_normal[i * 3] = 0;
        m_normal[i * 3 + 1] = 1;
        m_normal[i * 3 + 2] = 0;
    }
    m_uv = m_normal;

    m_index.resize(32);
    m_index[0] = 0;
    for (int i = 0; i < 15; i++)
    {
        m_index[2 * i + 1] = i + 1;
        m_index[2 * i + 2] = 31 - i;
    }
    m_index[31] = 16;
}

CompassPlaneEdge::CompassPlaneEdge() : GraphicsEntity(NormalLineStripID)
{
    m_profile.color = CREAM_COLOR;
}

void CompassPlaneEdge::generate()
{
    m_vertex.resize(32 * 3);
    for (int i = 0; i < 32; i++)
    {
        m_vertex[i * 3] = static_cast<float>(std::sin((i / 32.0f) * 2 * Math::PI));
        m_vertex[i * 3 + 1] = 0;
        m_vertex[i * 3 + 2] = static_cast<float>(std::cos((i / 32.0f) * 2 * Math::PI));
    }

    m_normal = m_vertex;
    m_uv = m_vertex;

    m_index.resize(32);
    for (int i = 0; i < 32; i++)
    {
        m_index[i] = i;
    }
    m_index.push_back(0);
}

void syncCamera(LogicObserveCamera* pMainCamera, GraphicsCamera* pCompassCamera)
{
    auto& camera = pMainCamera->get();
    auto& position = camera.position();
    auto& lookAt = camera.lookAt();
    glm::vec3 dir = glm::vec3(position.x - lookAt.x, position.y - lookAt.y, position.z - lookAt.z);
    dir = glm::normalize(dir);

    pCompassCamera->setPosition({ dir.x * 5, dir.y * 5, dir.z * 5 });
    pCompassCamera->setLookAt({ 0, 0, 0 });
    pCompassCamera->setUp(camera.up());
}

CompassArrow::CompassArrow(const glm::mat4& matrix) : GraphicsEntity(NormalFaceID)
{
    m_profile.matrix = matrix;
    m_profile.color = CHEESE_COLOR;
}

void CompassArrow::generate()
{
    constexpr float radius = 0.15f;
    constexpr float length = 0.3f;

    m_vertex.resize(12 * 3);
    for (int i = 0; i < 12; i++)
    {
        m_vertex[i * 3] = 1.2f;
        m_vertex[i * 3 + 1] = radius * static_cast<float>(std::sin((i / 12.0f) * 2 * Math::PI));
        m_vertex[i * 3 + 2] = radius * static_cast<float>(std::cos((i / 12.0f) * 2 * Math::PI));
    }
    m_vertex.push_back(1.2f);
    m_vertex.push_back(0);
    m_vertex.push_back(0);
    m_vertex.push_back(1.2f + length);
    m_vertex.push_back(0);
    m_vertex.push_back(0);
    m_normal = m_uv = m_vertex;

    m_index.resize(12 * 3 + 12 * 3);
    for (int i = 0; i < 12; i++)
    {
        m_index[i * 3] = 12;
        m_index[i * 3 + 1] = i;
        m_index[i * 3 + 2] = (i + 1) % 12;
    }
    for (int i = 0; i < 12; i++)
    {
        m_index[i * 3 + 12] = 13;
        m_index[i * 3 + 13] = i;
        m_index[i * 3 + 14] = (i + 1) % 12;
    }
}

CompassArrowLine::CompassArrowLine(const glm::mat4& matrix) : GraphicsEntity(NormalFaceStripID)
{
    m_profile.matrix = matrix;
    m_profile.color = CHEESE_COLOR;
}

void CompassArrowLine::generate()
{
    constexpr float radius = 0.03f;

    m_vertex.resize(6 * 3 * 2);
    for (int i = 0; i < 6; i++)
    {
        m_vertex[i * 3] = 0;
        m_vertex[i * 3 + 18] = 1;
        m_vertex[i * 3 + 1] = m_vertex[i * 3 + 19] = radius * static_cast<float>(std::sin((i / 6.0f) * 2 * Math::PI));
        m_vertex[i * 3 + 2] = m_vertex[i * 3 + 20] = radius * static_cast<float>(std::cos((i / 6.0f) * 2 * Math::PI));
    }
    m_normal = m_uv = m_vertex;

    m_index = { 0, 5, 1, 4, 2, 3, RestartMark};
    m_index.resize(7 + 7 + 14);
    for (int i = 0; i < 6; i++)
    {
        m_index[i + 7] = m_index[i] + 6;
    }
    m_index[13] = RestartMark;
    for (int i = 0; i < 7; i++)
    {
        m_index[i * 2 + 14] = i % 6;
        m_index[i * 2 + 15] = i % 6 + 6;
    }
}
