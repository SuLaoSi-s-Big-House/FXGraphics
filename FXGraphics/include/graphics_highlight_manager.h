#ifndef _GRAPHICS_HIGHLIGHT_MANAGER_H_
#define _GRAPHICS_HIGHLIGHT_MANAGER_H_

#include <vector>
#include <unordered_map>
#include "basic_vector.h"

namespace FX {

    using HighlightType = unsigned char;
    constexpr HighlightType NoneHighlight = 1;
    constexpr HighlightType HoverHighlight = 1 << 0;
    constexpr HighlightType PickHighlight = 1 << 1;
    constexpr HighlightType SelectHighlight = 1 << 2;

    inline HighlightType operator|(HighlightType a, HighlightType b) {
        return static_cast<HighlightType>(static_cast<unsigned char>(a) | static_cast<unsigned char>(b));
    }
    inline HighlightType operator&(HighlightType a, HighlightType b) {
        return static_cast<HighlightType>(static_cast<unsigned char>(a) & static_cast<unsigned char>(b));
    }
    inline HighlightType operator~(HighlightType a) {
        return static_cast<HighlightType>(~static_cast<unsigned char>(a));
    }
    inline HighlightType& operator|=(HighlightType& a, HighlightType b) {
        a = a | b; return a;
    }
    inline HighlightType& operator&=(HighlightType& a, HighlightType b) {
        a = a & b; return a;
    }

    struct HighlightStyle {
        vec4uc color = { 255, 255, 255, 255 };
    };

    class GraphicsEntity;

    class GraphicsHighlightManager {
    protected:
        friend class GraphicsScene;

        explicit GraphicsHighlightManager(GraphicsScene* pScene);
        virtual ~GraphicsHighlightManager(void) = default;

    public:
        void setHighlightStyle(HighlightType type, const HighlightStyle& style);
        const HighlightStyle& highlightStyle(HighlightType type);

        bool addHighlight(GraphicsEntity* pEntity, HighlightType type);
        bool removeHighlight(GraphicsEntity* pEntity, HighlightType type);
        bool removeHighlight(GraphicsEntity* pEntity);
        bool removeAllHighlight(HighlightType type);
        bool removeAllHighlight(void);

    protected:
        struct HighlightData {
            HighlightStyle style;
            bool dirty = true;
        };

        std::vector<HighlightData> m_highlightStyles;
        std::unordered_map<GraphicsEntity*, HighlightType> m_entityHighlightMask;
        GraphicsScene* m_pScene = nullptr;
    };

} // namespace FX

#endif // _GRAPHICS_HIGHLIGHT_MANAGER_H_
