#ifndef _GRAPHICS_CAMERA_H_
#define _GRAPHICS_CAMERA_H_

#include <unordered_set>
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"

#include "math_common.h"
#include "basic_vector.h"

namespace FX {

    class GraphicsScene;

    class GraphicsCamera {
    public:
        friend class GraphicsScene;

        GraphicsCamera(void);
        ~GraphicsCamera(void);

        void setPosition(const vec3f& position);
        void setLookAt(const vec3f& lookAt);
        void setUp(const vec3f& up);
        const vec3f& position(void) const;
        const vec3f& lookAt(void) const;
        const vec3f& up(void) const;

        void setField(float left, float right, float bottom, float top);
        void setField(float fov, float ratio);
        void setNearFar(float near, float far);

        const glm::mat4& vMatrix(void) const;
        const glm::mat4& pMatrix(void) const;
        const glm::mat4& vPMatrix(void) const;

        static glm::mat4 defaultVPMatrix(void);

    protected:
        void updateMatrix(void);

        void addScene(GraphicsScene* pScene);
        void eraseScene(GraphicsScene* pScene);

    protected:
        vec3f m_position = { 5.0f, 0.0f, 5.0f };
        vec3f m_lookAt = { 0.0f, 0.0f, 0.0f };
        vec3f m_up = { 0.0f, 1.0f, 0.0f };
        float m_near = 0.1f;
        float m_far = 1000.0f;

        // ortho
        vec4f m_extent = { -400.0f, 400.0f, -300.0f, 300.0f };

        // perspective
        float m_fov = static_cast<float>(Math::PI / 6);
        float m_ratio = 4 / 3.0f;

        glm::mat4 m_vMatrix = glm::mat4(1.0f);
        glm::mat4 m_pMatrix = glm::mat4(1.0f);
        glm::mat4 m_vPMatrix = glm::mat4(1.0f);

        std::unordered_set<GraphicsScene*> m_sceneList;

        bool m_ortho = false;
    };

} // namespace FX

#endif // _GRAPHICS_CAMERA_H_
