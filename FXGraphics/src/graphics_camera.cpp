#include "graphics_camera.h"

#include "graphics_scene.h"

namespace FX {

    GraphicsCamera::GraphicsCamera()
    {
        updateMatrix();
    }

    GraphicsCamera::~GraphicsCamera()
    {
        std::unordered_set<GraphicsScene*> sceneList = m_sceneList;
        for (auto pScene : sceneList)
        {
            pScene->unbindCamera();
        }
    }

    void GraphicsCamera::setPosition(const vec3f& position)
    {
        if (m_position != position)
        {
            m_position = position;
            updateMatrix();
        }
    }

    void GraphicsCamera::setLookAt(const vec3f& lookAt)
    {
        if (m_lookAt != lookAt)
        {
            m_lookAt = lookAt;
            updateMatrix();
        }
    }

    void GraphicsCamera::setUp(const vec3f& up)
    {
        if (m_up != up)
        {
            m_up = up;
            updateMatrix();
        }
    }

    const vec3f& GraphicsCamera::position() const
    {
        return m_position;
    }

    const vec3f& GraphicsCamera::lookAt() const
    {
        return m_lookAt;
    }

    const vec3f& GraphicsCamera::up() const
    {
        return m_up;
    }

    void GraphicsCamera::setField(float left, float right, float bottom, float top)
    {
        if (m_ortho && m_extent == vec4f{ left, right, bottom, top })
        {
            return;
        }

        m_extent = { left, right, bottom, top };
        m_ortho = true;
        updateMatrix();
    }

    void GraphicsCamera::setField(float fov, float ratio)
    {
        if (!m_ortho && Math::isEqual(m_fov, fov) && Math::isEqual(m_ratio, ratio))
        {
            return;
        }

        m_fov = fov;
        m_ratio = ratio;
        m_ortho = false;
        updateMatrix();
    }

    void GraphicsCamera::setNearFar(float near, float far)
    {
        if (Math::isEqual(m_near, near) && Math::isEqual(m_far, far))
        {
            return;
        }

        m_near = near;
        m_far = far;
        updateMatrix();
    }

    const glm::mat4& GraphicsCamera::vMatrix() const
    {
        return m_vMatrix;
    }

    const glm::mat4& GraphicsCamera::pMatrix() const
    {
        return m_pMatrix;
    }

    const glm::mat4& GraphicsCamera::vPMatrix() const
    {
        return m_vPMatrix;
    }

    glm::mat4 GraphicsCamera::defaultVPMatrix()
    {
        return glm::mat4(1.0f);
    }

    GraphicsCamera::GraphicsCamera(const GraphicsCamera& other)
        : m_position(other.m_position)
        , m_lookAt(other.m_lookAt)
        , m_up(other.m_up)
        , m_near(other.m_near)
        , m_far(other.m_far)
        , m_extent(other.m_extent)
        , m_fov(other.m_fov)
        , m_ratio(other.m_ratio)
        , m_vMatrix(other.m_vMatrix)
        , m_pMatrix(other.m_pMatrix)
        , m_vPMatrix(other.m_vPMatrix)
        , m_ortho(other.m_ortho)
    {
    }

    GraphicsCamera::GraphicsCamera(GraphicsCamera&& other) noexcept
        : m_position(other.m_position)
        , m_lookAt(other.m_lookAt)
        , m_up(other.m_up)
        , m_near(other.m_near)
        , m_far(other.m_far)
        , m_extent(other.m_extent)
        , m_fov(other.m_fov)
        , m_ratio(other.m_ratio)
        , m_vMatrix(other.m_vMatrix)
        , m_pMatrix(other.m_pMatrix)
        , m_vPMatrix(other.m_vPMatrix)
        , m_ortho(other.m_ortho)
    {
        std::unordered_set<GraphicsScene*> sceneList = other.m_sceneList;
        for (auto pScene : sceneList)
        {
            assert(pScene != nullptr);
            pScene->bindCamera(this);
        }
    }

    GraphicsCamera& GraphicsCamera::operator=(const GraphicsCamera& other)
    {
        if (this == &other)
        {
            return *this;
        }

        m_position = other.m_position;
        m_lookAt = other.m_lookAt;
        m_up = other.m_up;
        m_near = other.m_near;
        m_far = other.m_far;
        m_extent = other.m_extent;
        m_fov = other.m_fov;
        m_ratio = other.m_ratio;
        m_vMatrix = other.m_vMatrix;
        m_pMatrix = other.m_pMatrix;
        m_vPMatrix = other.m_vPMatrix;
        m_ortho = other.m_ortho;

        return *this;
    }

    GraphicsCamera& GraphicsCamera::operator=(GraphicsCamera&& other)
    {
        if (this == &other)
        {
            return *this;
        }

        m_position = other.m_position;
        m_lookAt = other.m_lookAt;
        m_up = other.m_up;
        m_near = other.m_near;
        m_far = other.m_far;
        m_extent = other.m_extent;
        m_fov = other.m_fov;
        m_ratio = other.m_ratio;
        m_vMatrix = other.m_vMatrix;
        m_pMatrix = other.m_pMatrix;
        m_vPMatrix = other.m_vPMatrix;
        m_ortho = other.m_ortho;

        std::unordered_set<GraphicsScene*> sceneList = other.m_sceneList;
        for (auto pScene : sceneList)
        {
            assert(pScene != nullptr);
            pScene->bindCamera(this);
        }

        return *this;
    }

    void GraphicsCamera::updateMatrix()
    {
        m_vMatrix = glm::lookAt(
            glm::vec3(m_position.x, m_position.y, m_position.z),
            glm::vec3(m_lookAt.x, m_lookAt.y, m_lookAt.z),
            glm::vec3(m_up.x, m_up.y, m_up.z));

        if (m_ortho)
        {
            m_pMatrix = glm::ortho(m_extent.x, m_extent.y, m_extent.z, m_extent.w, m_near, m_far);
        }
        else
        {
            m_pMatrix = glm::perspective(m_fov, m_ratio, m_near, m_far);
        }

        m_vPMatrix = m_pMatrix * m_vMatrix;
    }

    void GraphicsCamera::addScene(GraphicsScene* pScene)
    {
        assert(pScene != nullptr);
        m_sceneList.insert(pScene);
    }

    void GraphicsCamera::eraseScene(GraphicsScene* pScene)
    {
        assert(pScene != nullptr);
        m_sceneList.erase(pScene);
    }

} // namespace FX
