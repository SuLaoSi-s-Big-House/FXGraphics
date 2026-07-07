#include "graphics_highlight_manager.h"

#include <limits>
#include "basic_log.h"

namespace FX {

    namespace {

    }  // namespace

    GraphicsHighlightManager::GraphicsHighlightManager(GraphicsScene* pScene) : m_pScene(pScene)
    {
        static HighlightStyle defaultStyle = { 255, 255, 255, 255 };

        // TODO
    }

    void GraphicsHighlightManager::setHighlightStyle(HighlightType type, const HighlightStyle& style)
    {
        // 如果type为0或255是没意义的，但并不会产生问题
        m_highlightStyles[type] = style;
    }

    const HighlightStyle& GraphicsHighlightManager::highlightStyle(HighlightType type)
    {
        auto itr = m_highlightStyles.find(type);
        if (itr != m_highlightStyles.end())
        {
            return itr->second;
        }
        return s_defaultStyle;
    }

    bool GraphicsHighlightManager::addHighlight(GraphicsEntity* pEntity, HighlightType type)
    {
        if (pEntity == nullptr)
        {
            return false;
        }
        m_entityHighlightMask[pEntity] |= type;
        return true;
    }

    bool GraphicsHighlightManager::removeHighlight(GraphicsEntity* pEntity, HighlightType type)
    {
        if (pEntity == nullptr)
        {
            return false;
        }
        auto itr = m_entityHighlightMask.find(pEntity);
        if (itr != m_entityHighlightMask.end())
        {
            itr->second &= ~type;
            if (itr->second == HighlightType::kNone)
            {
                m_entityHighlightMask.erase(itr);
            }
            return true;
        }
        return false;
    }

    bool GraphicsHighlightManager::removeHighlight(GraphicsEntity* pEntity)
    {
        if (pEntity == nullptr)
        {
            return false;
        }
        return m_entityHighlightMask.erase(pEntity) > 0;
    }

    bool GraphicsHighlightManager::removeAllHighlight(HighlightType type)
    {
        bool success = false;
        for (auto itr = m_entityHighlightMask.begin(); itr != m_entityHighlightMask.end();)
        {
            auto oldMask = itr->second;
            itr->second &= ~type;
            if (itr->second != oldMask)
            {
                success = true;
            }
            if (itr->second == HighlightType::kNone)
            {
                itr = m_entityHighlightMask.erase(itr);
            }
            else
            {
                ++itr;
            }
        }
        return success;
    }

    bool GraphicsHighlightManager::removeAllHighlight()
    {
        if (m_entityHighlightMask.empty())
        {
            return false;
        }
        m_entityHighlightMask.clear();
        return true;
    }

} // namespace FX
