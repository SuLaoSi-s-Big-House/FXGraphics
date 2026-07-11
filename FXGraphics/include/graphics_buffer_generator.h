#ifndef _GRAPHICS_BUFFER_GENERATOR_H_
#define _GRAPHICS_BUFFER_GENERATOR_H_

#include <assert.h>
#include "glm.hpp"
#include "basic_vector.h"
#include "graphics_entity.h"

namespace FX {

    // vertex //////////////////////////////////////////////////////////

    struct NormalVertexData {
        vec3f position;
    };

    struct NormalNormalData {
        vec3f normal;
    };

    struct NormalUvData {
        vec3f uvw;
    };

    struct NormalRankData {
        vec2i rank;
    };

    template<class T>
    void exportVertex(const GraphicsEntity* pEntity, vec2i index, T* pDest)
    {
        static_assert(false, "Users should implement this function for custom type.");
    }

    template<>
    void exportVertex(const GraphicsEntity* pEntity, vec2i, NormalVertexData* pDest)
    {
        assert(pEntity);
        assert(pDest);
        memcpy(pDest, pEntity->vertex(), pEntity->pointNum() * 3 * sizeof(float));
    }

    template<>
    void exportVertex(const GraphicsEntity* pEntity, vec2i, NormalNormalData* pDest)
    {
        assert(pEntity);
        assert(pDest);
        memcpy(pDest, pEntity->normal(), pEntity->pointNum() * 3 * sizeof(float));
    }

    template<>
    void exportVertex(const GraphicsEntity* pEntity, vec2i, NormalUvData* pDest)
    {
        assert(pEntity);
        assert(pDest);
        memcpy(pDest, pEntity->uv(), pEntity->pointNum() * 3 * sizeof(float));
    }

    template<>
    void exportVertex(const GraphicsEntity* pEntity, vec2i index, NormalRankData* pDest)
    {
        assert(pEntity);
        assert(pDest);
        for (unsigned int i = 0; i < pEntity->pointNum(); i++)
        {
            pDest[i].rank = index;
        }
    }

    // index //////////////////////////////////////////////////////////

    template<typename T>
    void exportIndex(const GraphicsEntity* pEntity, unsigned int offset, T* pDest)
    {
        static_assert(false, "Users should implement this function for custom type.");
    }

    template<>
    void exportIndex(const GraphicsEntity* pEntity, unsigned int offset, unsigned int* pDest)
    {
        assert(pEntity);
        assert(pDest);
        for (unsigned int i = 0; i < pEntity->indexNum(); i++)
        {
            pDest[i] = pEntity->index()[i] == RestartMark ? RestartMark : pEntity->index()[i] + offset;
        }
    }

    // profile //////////////////////////////////////////////////////////

    struct NormalProfileData {
        glm::mat4 matrix = glm::mat4(1.0f);
        unsigned int materialHandle = 0;
        unsigned int reserved[3] = {0, 0, 0};
    };

    template<class T>
    void exportProfile(GraphicsEntity* pEntity, T* pDest)
    {
        static_assert(false, "Users should implement this function for custom type.");
    }

    template<>
    void exportProfile(GraphicsEntity* pEntity, NormalProfileData* pDest)
    {
        assert(pEntity);
        assert(pDest);
        pDest->matrix = pEntity->matrix();
        pDest->materialHandle = static_cast<unsigned int>(pEntity->materialHandle());
    }

    // command //////////////////////////////////////////////////////////

    struct DrawElementsCommand {
        unsigned int indexNum = 0;
        unsigned int instanceNum = 1;
        unsigned int indexStart = 0;
        int vertexOffset = 0;
        unsigned int instanceOffset = 0;
    };

} // namespace FX

#endif // _GRAPHICS_BUFFER_GENERATOR_H_
