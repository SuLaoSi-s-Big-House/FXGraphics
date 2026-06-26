#ifndef _COMPASS_H_
#define _COMPASS_H_

#include "glm.hpp"
#include "graphics_entity.h"
#include "graphics_scene.h"
#include "graphics_camera.h"
#include "logic_camera.h"

using namespace FX;

constexpr int COMPASS_SIZE = 200;
const vec4uc CREAM_COLOR = { 255, 238, 210, 255 };
const vec4uc CHEESE_COLOR = { 183, 154, 107, 255 };

class CompassScene : public GraphicsScene {
    void clear(void) override;
    void bindGlobal(void) override;
};


class CompassPlane : public GraphicsEntity {
public:
    CompassPlane(void);

    void generate(void) override;
};


class CompassPlaneEdge : public GraphicsEntity {
public:
    CompassPlaneEdge(void);

    void generate(void) override;
};


class CompassArrow : public GraphicsEntity {
public:
    CompassArrow(const glm::mat4& matrix);

    void generate(void) override;
};


class CompassArrowLine : public GraphicsEntity {
public:
    CompassArrowLine(const glm::mat4& matrix, float radius = 0.03f, vec4uc color = CHEESE_COLOR);

    void generate(void) override;
    void setScale(float scale);
    const EntityProfile& profile(void) override;

private:
    glm::mat4 m_matrix;
    float m_radius = 0.03f;
    float m_scale = 1.0f;
};


void syncCamera(LogicObserveCamera* pMainCamera, GraphicsCamera* pCompassCamera);


class ObjEntity : public GraphicsEntity {
public:
    ObjEntity(const std::string& path);

    void generate(void) override {}
};

#endif
