#include "surf_item.h"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "graphics_material_manager.h"

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
        Material material = GraphicsMaterialManager::instance().get(m_materialHandle);
        material.custom1.x = depth;
        GraphicsMaterialManager::instance().unref(m_materialHandle);
        m_materialHandle = GraphicsMaterialManager::instance().ref(material);
        setDirty(MaterialDirty);
    }

    float SurfEntity::depth() const
    {
        return m_depth;
    }

    const glm::mat4& SurfEntity::matrix()
    {
        m_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(m_position.x, m_position.y, 0.0f));
        return m_matrix;
    }

} // namespace FX
