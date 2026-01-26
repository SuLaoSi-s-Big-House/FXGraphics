#include "graphics_gpu_item.h"

#include <assert.h>
#include "basic_log.h"
#include "graphics_window_impl.h"

namespace FX {

    GraphicsGPUItem::~GraphicsGPUItem()
    {
        for (auto&& pair : m_itemList)
        {
            pair.second->m_pOwner = nullptr;
            pair.first->addToDelete(pair.second);
            pair.first->removeItem(this);
        }
    }

    ItemInfo* GraphicsGPUItem::getOrCreate(bool bForceCreate)
    {
        auto pWindow = GraphicsWindowImpl::currentWindow();
        if (pWindow == nullptr)
        {
            assert(pWindow);
            BasicLog::out(BasicLog::kWarn, "No window is used, cannot get current gpu item.");
            return nullptr;
        }

        auto itr = m_itemList.find(pWindow);
        if (itr != m_itemList.end())
        {
            if (bForceCreate)
            {
                delete itr->second;
                itr->second = create();
                assert(itr->second != nullptr);
            }
            return itr->second;
        }
        else
        {
            auto pNewItem = create();
            assert(pNewItem != nullptr);
            pWindow->addItem(this);
            auto res = m_itemList.insert({ pWindow, pNewItem });
            return res.first->second;
        }
    }

    void GraphicsGPUItem::clearItem(GraphicsWindowImpl* pWindow)
    {
        assert(pWindow != nullptr);
        auto itr = m_itemList.find(pWindow);
        assert(itr != m_itemList.end());
        delete itr->second;
        m_itemList.erase(itr);
    }

    GPUItemType GraphicsGPUItem::type() const
    {
        return m_type;
    }

    ItemInfo::ItemInfo(const GraphicsGPUItem* pOwner)
        : m_pOwner(pOwner), m_type(pOwner->type())
    {
    }

} // namespace FX
