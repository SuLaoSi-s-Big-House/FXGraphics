#ifndef _COMPASS_H_
#define _COMPASS_H_

#include "glm.hpp"
#include "graphics_entity.h"
#include "graphics_scene.h"
#include "graphics_camera.h"
#include "logic_camera.h"

using namespace FX;

constexpr int COMPASS_SIZE = 200;

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
    CompassArrowLine(const glm::mat4& matrix);

    void generate(void) override;
};


void syncCamera(LogicObserveCamera* pMainCamera, GraphicsCamera* pCompassCamera);


#endif
