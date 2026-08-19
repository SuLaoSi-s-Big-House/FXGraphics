#ifndef _GRAPHICS_BUFFER_GENERATOR_H_
#define _GRAPHICS_BUFFER_GENERATOR_H_

#include <assert.h>
#include "glm.hpp"
#include "basic_vector.h"
#include "graphics_entity.h"
#include "graphics_texture_manager.h"

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
        glm::mat4 matrix;
        vec4f color;
        vec4f custom1;
        vec4f custom2;
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
        auto& profile = pEntity->profile();
        pDest->matrix = profile.matrix;
        pDest->color = {
            profile.color.r / 255.0f,
            profile.color.g / 255.0f,
            profile.color.b / 255.0f,
            profile.color.a / 255.0f,
        };
        pDest->custom1 = profile.custom1;
        pDest->custom2 = profile.custom2;
    }

    struct NormalTextureProfileData {
        glm::mat4 matrix;
        vec4f color;
        vec4f uvScale1;    // (scaleX, scaleY, slice, 0)，base color slot，与normal_texture_profile.h保持一致
        vec4f uvScale2;    // normal slot
        vec4f uvScale3;    // orm slot
        vec4f custom1;
        vec4f custom2;
    };

    // C++与GLSL双端结构布局契约，normal_texture_profile.h中的NormalTextureProfileData需要与此保持一致
    static_assert(sizeof(NormalTextureProfileData) == sizeof(glm::mat4) + 6 * sizeof(vec4f), "NormalTextureProfileData must match normal_texture_profile.h");

    template<>
    void exportProfile(GraphicsEntity* pEntity, NormalTextureProfileData* pDest)
    {
        assert(pEntity);
        assert(pDest);
        auto& profile = pEntity->profile();
        pDest->matrix = profile.matrix;
        pDest->color = {
            profile.color.r / 255.0f,
            profile.color.g / 255.0f,
            profile.color.b / 255.0f,
            profile.color.a / 255.0f,
        };
        auto& manager = GraphicsTextureManager::instance();
        auto info1 = manager.query(profile.texture.handle(BaseColorTextureSlot));
        auto info2 = manager.query(profile.texture.handle(NormalTextureSlot));
        auto info3 = manager.query(profile.texture.handle(ORMTextureSlot));
        pDest->uvScale1 = { info1.scale.x, info1.scale.y, static_cast<float>(info1.slice), 0 };
        pDest->uvScale2 = { info2.scale.x, info2.scale.y, static_cast<float>(info2.slice), 0 };
        pDest->uvScale3 = { info3.scale.x, info3.scale.y, static_cast<float>(info3.slice), 0 };
        pDest->custom1 = profile.custom1;
        pDest->custom2 = profile.custom2;
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
