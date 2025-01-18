#ifndef _GRAPHICS_ENTITY_H_
#define _GRAPHICS_ENTITY_H_

#include <vector>
#include <map>

#include "basic_macro.h"

namespace FX {

    using EntityType = unsigned short;

    constexpr EntityType NormalLineID = 0;
    constexpr EntityType NormalFaceID = 1;
    constexpr EntityType NormalLineStripID = 2;
    constexpr EntityType NormalFaceStripID = 3;
    // For users: 用户可以添加自己的EntityType，并创建自己的GraphicsEntity派生类

    using DirtyType = unsigned int;

    constexpr DirtyType DataDirty = 1 << 0;
    constexpr DirtyType ColorDirty = 1 << 1;
    constexpr DirtyType TransparencyDirty = 1 << 2;
    constexpr DirtyType MatrixDirty = 1 << 3;
    constexpr DirtyType VisibleDirty = 1 << 4;
    // For users:
    // 用户可以添加自己的DirtyType，用于表示GraphicsEntity的变更情况。
    // 请注意DirtyType的常量值需要满足位运算的要求，同时需要在GraphicsEntityManager中响应自定义的DirtyType。

    constexpr unsigned int RestartMark = std::numeric_limits<unsigned int>::max();    // OpenGL图元重启标志

    struct GroupPos {
        int first = -1;
        int second = -1;

        bool valid(void) const
        {
            return first >= 0 && second >= 0;
        }
    };

    class GraphicsEntityManager;

    class GraphicsEntity {
    protected:
        friend class GraphicsEntityManager;
        friend struct EntityList;

        explicit GraphicsEntity(EntityType type) : m_type(type) {}
        virtual ~GraphicsEntity(void);

        DELETE_COPY_AND_MOVE_CONSTRUCT(GraphicsEntity);

    public:
        virtual void generate(void) = 0;

        virtual const float* vertex(void) const;
        virtual const float* normal(void) const;
        virtual const float* uv(void) const;
        virtual unsigned int pointNum(void) const;

        virtual const unsigned int* index(void) const;
        virtual unsigned int indexNum(void) const;

        EntityType type(void) const;

        void setDirty(DirtyType type);

    protected:
        bool isDataDirty(void) const;
        void setDataReady(void);

    private:
        GroupPos groupPos(GraphicsEntityManager* pManager) const;
        void setGroupPos(GraphicsEntityManager* pManager, GroupPos pos);
        void eraseGroup(GraphicsEntityManager* pManager);

    protected:
        std::vector<float> m_vertex;
        std::vector<float> m_normal;
        std::vector<float> m_uv;
        std::vector<unsigned int> m_index;
        const EntityType m_type = 0;

    private:
        bool m_dataDirty = true;
        std::map<GraphicsEntityManager*, GroupPos> m_managerList;
    };

} // namespace FX

#endif // _GRAPHICS_ENTITY_H_
