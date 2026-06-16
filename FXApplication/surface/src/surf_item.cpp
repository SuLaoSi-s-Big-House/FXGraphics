#include "surf_item.h"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"

namespace FX {

    void SurfEntity::setPosition(const vec2i& pos)
    {
        m_position = pos;
        setDirty(MatrixDirty);
    }

    const vec2i& SurfEntity::position() const
    {
        return m_position;
    }

    void SurfEntity::setDepth(float depth)
    {
        m_depth = depth;
        setDirty(ColorDirty);
    }

    float SurfEntity::depth() const
    {
        return m_depth;
    }

    const EntityProfile& SurfEntity::profile()
    {
        m_profile.matrix = glm::translate(glm::mat4(1.0f), glm::vec3(m_position.x, m_position.y, 0.0f));
        m_profile.custom1.x = m_depth;    // custom1.x作为实体深度
        return m_profile;
    }

} // namespace FX
