#include "logic_camera.h"

#include "glm.hpp"
#include "basic_log.h"
#include "graphics_window.h"
#include "graphics_window_impl.h"

namespace FX {

    namespace {

        constexpr float SCALE_SPEED_MAX = 10.0f;
        constexpr float SCALE_SPEED_MIN = 0.1f;
        constexpr float SCALE_RANGE_MAX = 1e4f;
        constexpr float SCALE_RANGE_MIN = 1e-4f;
        constexpr float DEFAULT_OBSERVE_FOV = static_cast<float>(Math::PI / 6);
        constexpr float VIEWPORT_PADDING = 1.2f;
        constexpr float DEFAULT_ROTATE_SPEED = 0.01f;
        constexpr float DEFAULT_MOVE_SPEED = 0.05f;

    }  // namespace

    LogicCamera::LogicCamera(const GraphicsWindow* pWindow) : m_pWindow(pWindow) {}

    GraphicsCamera& LogicCamera::get()
    {
        return m_camera;
    }

    void LogicCamera::setScaleSpeed(float speed)
    {
        if (speed >= SCALE_SPEED_MIN && speed <= SCALE_SPEED_MAX)
        {
            m_scaleSpeed = speed;
        }
        else
        {
            BasicLog::out(BasicLog::kWarn, "The scale speed should be between 0.1 and 10, but trying to set as ", speed, ", discard.");
        }
    }

    void LogicCamera::setScaleLimit(float max, float min)
    {
        if (min >= SCALE_RANGE_MIN && min <= SCALE_RANGE_MAX)
        {
            m_scaleLimit.x = min;
        }
        else
        {
            BasicLog::out(BasicLog::kWarn, "The scale limit should be between 1e-4 and 1e4, but trying to set as ", min, ", discard.");
        }
        if (max >= SCALE_RANGE_MIN && max <= SCALE_RANGE_MAX)
        {
            m_scaleLimit.y = max;
        }
        else
        {
            BasicLog::out(BasicLog::kWarn, "The scale limit should be between 1e-4 and 1e4, but trying to set as ", max, ", discard.");
        }

        if (max < min)
        {
            BasicLog::out(BasicLog::kWarn, "The scale limit ", min, " to ", max, " is strange.");
        }
    }

    void LogicCamera::setRotateSpeed(float speed)
    {
        m_rotateSpeed = speed;
    }

    float LogicCamera::scale() const
    {
        return m_scale;
    }

    LogicObserveCamera::LogicObserveCamera(const GraphicsWindow* pWindow) : LogicCamera(pWindow)
    {
        assert(pWindow != nullptr);
        if (pWindow != nullptr)
        {
            auto size = pWindow->size();
            // 目前画面大小总是和窗口大小一致，因此viewport直接使用window size
            if (size.x > 0 && size.y > 0)
            {
                m_viewport = { static_cast<float>(size.x), static_cast<float>(size.y) };
                m_windowSizeBak = size;
            }
        }

        if (m_viewport == vec2f{ 0, 0 } && m_windowSizeBak == vec2us{ 0, 0 })
        {
            m_viewport = { 800, 600 };
            m_windowSizeBak = { 800, 600 };
        }

        m_ortho = false;
        glm::vec3 positionDir = glm::normalize(glm::vec3(1.0f, 1.0f, 2.0f));
        m_camera.setPosition({ positionDir.x * 20, positionDir.y * 20, positionDir.z * 20 });
        m_camera.setLookAt({ 0.0f, 0.0f, 0.0f });
        glm::vec3 rightDir = glm::normalize(glm::vec3(positionDir.z, 0.0f, -positionDir.x));
        glm::vec3 upDir = glm::normalize(glm::cross(glm::vec3(10.0f, 10.0f, 10.0f), rightDir));
        m_camera.setUp({ upDir.x, upDir.y, upDir.z });
        m_camera.setField(DEFAULT_OBSERVE_FOV, 4.0f / 3);
        m_camera.setNearFar(0.1f, 1e4f);
    }

    void LogicObserveCamera::process()
    {
        // TODO 检查window

        auto& interactor = m_pWindow->interactor();
        auto flag = interactor.enventFlag();

        if (flag == NoneFlag)
        {
            return;
        }

        glm::vec3 position = glm::vec3(m_camera.position().x, m_camera.position().y, m_camera.position().z);
        glm::vec3 lookAt = glm::vec3(m_camera.lookAt().x, m_camera.lookAt().y, m_camera.lookAt().z);
        glm::vec3 positionDir = glm::normalize(position - lookAt);
        glm::vec3 upDir = glm::vec3(m_camera.up().x, m_camera.up().y, m_camera.up().z);
        assert(Math::isEqual(glm::length(upDir), 1.0f));
        glm::vec3 rightDir = glm::normalize(glm::cross(upDir, positionDir));

        if (flag & WindowResizeFlag)
        {
            auto size = m_pWindow->size();
            if (size.x > 0 && size.y > 0 && size != m_windowSizeBak)
            {
                m_viewport.x *= static_cast<float>(size.x) / m_windowSizeBak.x;
                m_viewport.y *= static_cast<float>(size.y) / m_windowSizeBak.y;
                if (m_ortho)
                {
                    m_camera.setField(-m_viewport.x / 2, m_viewport.x / 2, -m_viewport.y / 2, m_viewport.y / 2);
                }
                else
                {
                    m_camera.setField(DEFAULT_OBSERVE_FOV, m_viewport.x / m_viewport.y);
                }
                m_windowSizeBak = size;
            }
        }

        // TODO 优化手感
        if (flag & MouseScrollFlag)
        {
            assert(interactor.isCursorIn());
            auto scroll = static_cast<float>(interactor.mouseScroll());
            auto scale = m_scale * std::pow(m_scaleSpeed * (1 + DefaultScaleSpeed), -scroll);
            if (scale >= m_scaleLimit.x && scale <= m_scaleLimit.y)
            {
                m_viewport.x *= scale / m_scale;
                m_viewport.y *= scale / m_scale;
                m_scale = scale;

                if (m_ortho)
                {
                    m_camera.setField(-m_viewport.x / 2, m_viewport.x / 2, -m_viewport.y / 2, m_viewport.y / 2);
                }
                else
                {
                    auto distance = m_viewport.y / 2 / std::tan(DEFAULT_OBSERVE_FOV / 2);
                    position = lookAt + distance * positionDir;
                    m_camera.setPosition({ position.x, position.y, position.z });
                }
            }
        }

        if (flag & MouseDragFlag)
        {
            assert(interactor.isCursorIn());
            auto& dragInfo = interactor.dragInfo();
            auto pos = interactor.cursorPos();
            vec2f offset = { static_cast<float>(pos.x - dragInfo.startPos.x), static_cast<float>(pos.y - dragInfo.startPos.y) };

            if (dragInfo.button == MouseButton::kRight)
            {
                if (m_rotating == false)
                {
                    m_positionBak = m_camera.position();
                    m_lookAtBak = m_camera.lookAt();
                    m_upBak = m_camera.up();
                    m_rotating = true;
                }

                // 旋转需要使用存档的相机姿态
                position = glm::vec3(m_positionBak.x, m_positionBak.y, m_positionBak.z);
                positionDir = glm::normalize(position - lookAt);
                upDir = glm::vec3(m_upBak.x, m_upBak.y, m_upBak.z);
                assert(Math::isEqual(glm::length(upDir), 1.0f));
                glm::vec3 rightDir = glm::normalize(glm::cross(upDir, positionDir));

                // 根据速度计算出需要旋转的量，并计算出实际旋转的轴与弧度
                glm::vec2 angles = {
                    -offset.x * m_rotateSpeed * DEFAULT_ROTATE_SPEED,
                    offset.y * m_rotateSpeed * DEFAULT_ROTATE_SPEED,
                };

                auto rad = glm::length(angles);
                if (Math::isZero(rad))
                {
                    m_camera.setPosition(m_positionBak);
                    m_camera.setUp(m_upBak);
                }
                else
                {
                    glm::vec3 axis = glm::normalize(glm::vec3(-angles.y, angles.x, 0.0f));
                    // 这里axis是相机坐标系，需要转为世界坐标系
                    axis = axis.x * rightDir + axis.y * upDir;
                    assert(Math::isEqual(glm::length(axis), 1.0f));

                    // 计算出世界坐标系下的旋转矩阵
                    auto matrix = glm::rotate(glm::mat4(1.0f), rad, axis);

                    positionDir = glm::normalize(matrix * glm::vec4(positionDir, 0.0f));
                    auto distance = glm::distance(lookAt, position);
                    assert(Math::isZero(distance) == false);
                    position = lookAt + distance * positionDir;
                    upDir = glm::normalize(matrix * glm::vec4(upDir, 0.0f));

                    m_camera.setPosition({ position.x, position.y, position.z });
                    m_camera.setUp({ upDir.x, upDir.y, upDir.z });
                }
            }
            else if (dragInfo.button == MouseButton::kMiddle)
            {
                if (m_moving == false)
                {
                    m_positionBak = m_camera.position();
                    m_lookAtBak = m_camera.lookAt();
                    m_upBak = m_camera.up();
                    m_moving = true;
                }

                glm::vec3 move = -rightDir * offset.x + upDir * offset.y;
                move *= DEFAULT_MOVE_SPEED * m_scale;
                position = glm::vec3(m_positionBak.x, m_positionBak.y, m_positionBak.z) + move;
                lookAt = glm::vec3(m_lookAtBak.x, m_lookAtBak.y, m_lookAtBak.z) + move;

                m_camera.setPosition({ position.x, position.y, position.z });
                m_camera.setLookAt({ lookAt.x, lookAt.y, lookAt.z });
            }

            m_rotating = (dragInfo.button == MouseButton::kRight);
            m_moving = (dragInfo.button == MouseButton::kMiddle);
        }
        else
        {
            m_rotating = false;
            m_moving = false;
        }
    }

    void LogicObserveCamera::setOrtho()
    {
        if (m_ortho)
        {
            return;
        }

        m_ortho = true;
        m_camera.setField(-m_viewport.x / 2, m_viewport.x / 2, -m_viewport.y / 2, m_viewport.y / 2);
    }

    void LogicObserveCamera::setPerspective()
    {
        if (!m_ortho)
        {
            return;
        }

        m_ortho = false;

        glm::vec3 lookAt = glm::vec3(m_camera.lookAt().x, m_camera.lookAt().y, m_camera.lookAt().z);
        glm::vec3 position = glm::vec3(m_camera.position().x, m_camera.position().y, m_camera.position().z);
        glm::vec3 positionDir = glm::normalize(position - lookAt);

        auto distance = m_viewport.y / 2 / std::tan(DEFAULT_OBSERVE_FOV / 2);
        position = lookAt + distance * positionDir;

        m_camera.setPosition({ position.x, position.y, position.z });
        m_camera.setField(DEFAULT_OBSERVE_FOV, m_viewport.x / m_viewport.y);
    }

    bool LogicObserveCamera::isOrtho() const
    {
        return m_ortho;
    }

    void LogicObserveCamera::observe(const BasicBounding<>& box)
    {
        if (box.valid() == false)
        {
            BasicLog::out(BasicLog::kWarn, "Trying to observe an invalid bounding box, discard.");
            return;
        }

        glm::vec3 observeDir;
        {
            auto& position = m_camera.position();
            auto& lookAt = m_camera.lookAt();
            if (position != lookAt)
            {
                observeDir = glm::normalize(glm::vec3(lookAt.x, lookAt.y, lookAt.z) - glm::vec3(position.x, position.y, position.z));
            }
            else
            {
                observeDir = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));
            }
        }

        glm::vec3 lookAt = { box.center().x, box.center().y, box.center().z };
        glm::vec3 rightDir = glm::normalize(glm::vec3(-observeDir.z, 0.0f, observeDir.x));
        glm::vec3 upDir = glm::normalize(glm::cross(rightDir, observeDir));

        glm::vec3 corners[8] =
        {
            { box.m_min.x, box.m_min.y, box.m_min.z },
            { box.m_min.x, box.m_min.y, box.m_max.z },
            { box.m_min.x, box.m_max.y, box.m_min.z },
            { box.m_min.x, box.m_max.y, box.m_max.z },
            { box.m_max.x, box.m_min.y, box.m_min.z },
            { box.m_max.x, box.m_min.y, box.m_max.z },
            { box.m_max.x, box.m_max.y, box.m_min.z },
            { box.m_max.x, box.m_max.y, box.m_max.z },
        };

        vec2f max = { std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest() };
        vec2f min = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
        float near = std::numeric_limits<float>::max();
        float far = std::numeric_limits<float>::lowest();

        // 虚构一个以lookAt为中心、observeDir为法向的viewpoet平面，便于统一正交与透视投影
        // 将包围盒八个顶点投影到viewport平面，计算出合适的相机位置
        for (auto& pos : corners)
        {
            auto distance = glm::dot(pos - lookAt, observeDir);
            near = std::min(near, distance);
            far = std::max(far, distance);
            glm::vec3 projPos = pos - distance * observeDir;
            glm::vec3 projDir = projPos - lookAt;
            auto x = glm::dot(projDir, rightDir);
            auto y = glm::dot(projDir, upDir);
            max.x = std::max(max.x, x);
            max.y = std::max(max.y, y);
            min.x = std::min(min.x, x);
            min.y = std::min(min.y, y);
        }

        assert(max.x > min.x);
        assert(max.y > min.y);

        // 根据所有的投影点计算出viewport的长宽（保持长宽比不变），并扩大20%
        auto width = max.x - min.x;
        auto height = max.y - min.y;

        if (Math::isZero(width) && Math::isZero(height))
        {
            assert(0);
            width = 800;
            height = 600;
        }
        else if (Math::isZero(width))
        {
            height *= VIEWPORT_PADDING;
            width = m_viewport.x / m_viewport.y * height;
        }
        else if (Math::isZero(height))
        {
            width *= VIEWPORT_PADDING;
            height = m_viewport.y / m_viewport.x * width;
        }
        else
        {
            if (width / height > m_viewport.x / m_viewport.y)
            {
                height = m_viewport.y / m_viewport.x * width;
            }
            else
            {
                width = m_viewport.x / m_viewport.y * height;
            }
            height *= VIEWPORT_PADDING;
            width *= VIEWPORT_PADDING;
        }

        m_viewport = { width, height };

        // 根据上述信息推算出相机位置
        // TODO 精细化near far
        if (m_ortho)
        {
            glm::vec3 position = lookAt - near * observeDir + 0.1f;
            m_camera.setPosition({ position.x, position.y, position.z });
            m_camera.setField(-width / 2, width / 2, -height / 2, height / 2);
            m_camera.setNearFar(0.1f, 1e4f);
            //m_camera.setNearFar(0.1f, far - near + 0.1f);
        }
        else
        {
            auto distance = height / 2 / std::tan(DEFAULT_OBSERVE_FOV / 2);
            glm::vec3 position = lookAt - distance * observeDir;
            m_camera.setPosition({ position.x, position.y, position.z });
            m_camera.setField(DEFAULT_OBSERVE_FOV, width / height);
            m_camera.setNearFar(0.1f, 1e4f);
            //m_camera.setNearFar(std::max(distance + near, 0.1f), std::min(distance + far, 1e6f));
        }

        m_camera.setLookAt({ lookAt.x, lookAt.y, lookAt.z });
        m_camera.setUp({ upDir.x, upDir.y, upDir.z });
        m_scale = 1.0f;
    }

    void LogicObserveCamera::fit(const BasicBounding<>& box)
    {
        if (box.valid() == false)
        {
            BasicLog::out(BasicLog::kWarn, "Trying to fit an invalid bounding box, discard.");
            return;
        }

        glm::vec3 position = glm::vec3(m_camera.position().x, m_camera.position().y, m_camera.position().z);
        glm::vec3 lookAt = glm::vec3(m_camera.lookAt().x, m_camera.lookAt().y, m_camera.lookAt().z);
        glm::vec3 observeDir = glm::normalize(lookAt - position);

        glm::vec3 corners[8] =
        {
            { box.m_min.x, box.m_min.y, box.m_min.z },
            { box.m_min.x, box.m_min.y, box.m_max.z },
            { box.m_min.x, box.m_max.y, box.m_min.z },
            { box.m_min.x, box.m_max.y, box.m_max.z },
            { box.m_max.x, box.m_min.y, box.m_min.z },
            { box.m_max.x, box.m_min.y, box.m_max.z },
            { box.m_max.x, box.m_max.y, box.m_min.z },
            { box.m_max.x, box.m_max.y, box.m_max.z },
        };

        float near = std::numeric_limits<float>::max();
        float far = std::numeric_limits<float>::lowest();

        for (auto& pos : corners)
        {
            auto distance = glm::dot(pos - lookAt, observeDir);
            near = std::min(near, distance);
            far = std::max(far, distance);
        }

        if (m_ortho)
        {
            //m_camera.setNearFar(0.1f, 1e4f);
            m_camera.setNearFar(0.1f, far - near + 0.1f);
        }
        else
        {
            auto distance = glm::distance(lookAt, position);
            //m_camera.setNearFar(0.1f, 1e4f);
            m_camera.setNearFar(std::max(distance + near, 0.1f), std::min(distance + far, 1e6f));
        }
    }

} // namespace FX
