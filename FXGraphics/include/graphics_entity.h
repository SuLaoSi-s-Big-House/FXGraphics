#ifndef _GRAPHICS_ENTITY_H_
#define _GRAPHICS_ENTITY_H_

#include <vector>
#include <unordered_map>
#include <string>
#include <limits>
#include "glm.hpp"

#include "basic_vector.h"
#include "basic_macro.h"

namespace FX {

    using EntityType = unsigned int;

    constexpr EntityType NormalLineID = 0;
    constexpr EntityType NormalFaceID = 1;
    constexpr EntityType NormalLineStripID = 2;
    constexpr EntityType NormalFaceStripID = 3;
    constexpr EntityType NormalPointID = 6;
    // For users:
    // 用户可以添加自己的EntityType，并创建自己的GraphicsEntity派生类
    // 添加新的类型后需要调用GraphicsEntity::registerType()以注册

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

    // OpenGL图元类型
    enum class PrimitiveMode : unsigned int {
        kPoints = 0x0000,
        kLines = 0x0001,
        kLineStrip = 0x0003,
        kLineStripAdj = 0x000B,
        kTriangles = 0x0004,
        kTriangleStrip = 0x0005,
        kTriangleStripAdj = 0x000D,
    };

    struct GroupPos {
        int first = -1;
        int second = -1;

        bool valid(void) const;
    };

    struct Font {
        std::string name = "Arial";
        unsigned char size = 16;

        bool operator==(const Font& other) const;
        bool valid(void) const;
    };

    // 实体属性
    struct EntityProfile {
        glm::mat4 matrix = glm::mat4(1.0f);
        Font font;
        vec4uc color = { 255, 255, 255, 255 };
        bool visible = true;
        vec4f custom1;    // 预留属性，修改后用户需要自行调用setDirty通知图形系统
        vec4f custom2;
    };

    class GraphicsEntityManager;

    // 此文件定义了GraphicsEntity
    // GraphicsEntity是可绘制对象，在图形系统中绘制物体需要从GraphicsEntity派生，并实现顶点数据与属性等接口。
    // 实现GraphicsEntity后，需要将其加入到GraphicsScene以绘制。
    // 一个GraphicsEntity可以加入多个GraphicsScene中，但无法重复加入到同一个GraphicsScene中。

    class GraphicsEntity {
    protected:
        friend class GraphicsEntityManager;
        friend struct EntityList;

        explicit GraphicsEntity(EntityType type) : m_type(type) {}
        virtual ~GraphicsEntity(void);

        DELETE_COPY_AND_MOVE_CONSTRUCT(GraphicsEntity);

    public:
        // 派生类需要实现generate函数以生成顶点数据，包括顶点、法向、uv、索引等。
        // 图形系统会在绘制前调用此函数，用户通常不需要主动调用。
        // 此函数仅在m_dataDirty为true时被调用，如果用户需要更新GraphicsEntity的顶点数据，应当将m_dataDirty设置为true。
        virtual void generate(void) = 0;

        // 以下函数是获取顶点数据的函数，约定每个顶点需要3个float值表示坐标，3个float值表示法向，3个float值表示uv。
        // 如果GraphicsEntity不需要某一项数据，可以返回无效值，但不应返回空指针。
        virtual const float* vertex(void) const;
        virtual const float* normal(void) const;
        virtual const float* uv(void) const;
        virtual unsigned int pointNum(void) const;

        virtual const unsigned int* index(void) const;
        virtual unsigned int indexNum(void) const;

        // 预留接口，用户可以重写用来对接上层业务。默认返回空指针。图形系统不会主动调用。
        virtual void* owner(void) const;

        void setProfile(const EntityProfile& profile);
        virtual const EntityProfile& profile(void) const;

        EntityType type(void) const;

        void setDirty(DirtyType type);

        static bool registerType(EntityType type, PrimitiveMode mode);
        static const std::unordered_map<EntityType, PrimitiveMode>& entityTypeMap(void);

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
        EntityProfile m_profile;
        const EntityType m_type = 0;

        static std::unordered_map<EntityType, PrimitiveMode> s_entityTypeMap;

    private:
        bool m_dataDirty = true;
        std::unordered_map<GraphicsEntityManager*, GroupPos> m_managerList;
    };

} // namespace FX

#endif // _GRAPHICS_ENTITY_H_
