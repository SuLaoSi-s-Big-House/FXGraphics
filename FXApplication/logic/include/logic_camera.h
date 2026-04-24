#ifndef _LOGIC_CAMERA_H_
#define _LOGIC_CAMERA_H_

#include "basic_bounding.h"
#include "graphics_window.h"
#include "graphics_camera.h"

namespace FX {

    constexpr float DefaultScaleSpeed = 0.25f;

    class LogicCamera {
    public:
        explicit LogicCamera(const GraphicsWindow& window);
        virtual ~LogicCamera(void) = default;

        virtual void process(void) = 0;

        // 设置缩放速度，speed应该在[0.1, 10.0]范围内
        void setScaleSpeed(float speed);
        // 设置缩放限制，max与min均应该在[1e-4, 1e4]范围内
        void setScaleLimit(float max, float min);

        void setRotateSpeed(float speed);

        GraphicsCamera& get(void);

    protected:
        GraphicsCamera m_camera;
        const GraphicsWindow& m_window;
        float m_scale = 1.0f;
        float m_scaleSpeed = 1.0f;
        vec2f m_scaleLimit = { 0.01f, 100.0f };
        float m_rotateSpeed = 1.0f;
        vec2f m_viewport;
        vec2us m_windowSizeBak = { 800, 600 };
    };


    class LogicObserveCamera : public LogicCamera {
    public:
        explicit LogicObserveCamera(const GraphicsWindow& window);

        void process(void) override;

        void setOrtho(void);
        void setPerspective(void);
        bool isOrtho(void) const;

        // 计算出合适的相机位置与观察中心，保证所有物体都是可见的
        void observe(const BasicBounding<>& box);

        // 根据包围盒调整远近平面，不修改相机位置与观察中心
        void fit(const BasicBounding<>& box);

    protected:
        bool m_ortho = false;
        bool m_rotating = false;
        bool m_moving = false;
        vec3f m_positionBak;   // 记录变换前的相机姿态
        vec3f m_lookAtBak;
        vec3f m_upBak;
        // TODO 精细化near far
    };

} // namespace FX

#endif // _LOGIC_CAMERA_H_
