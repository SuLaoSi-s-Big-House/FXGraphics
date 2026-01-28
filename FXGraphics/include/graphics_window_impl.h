#ifndef _GRAPHICS_WINDOW_IMPL_H_
#define _GRAPHICS_WINDOW_IMPL_H_

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "glad.h"
#include "glfw3.h"

#include "basic_vector.h"
#include "basic_macro.h"
#include "graphics_interactor.h"

namespace FX {

    class GraphicsGPUItem;
    class ItemInfo;
    class GraphicsWindow;

    class GraphicsWindowImpl {
    protected:
        friend class GraphicsWindow;

        GraphicsWindowImpl(GraphicsWindow* pApi, unsigned short width, unsigned short height, const std::string& title, bool isMultiSample);
        virtual ~GraphicsWindowImpl(void);

        DELETE_COPY_AND_MOVE_CONSTRUCT(GraphicsWindowImpl);

    public:
        void use(void);
        void frame(void);

        void setSize(unsigned short width, unsigned short height);
        vec2us size(void) const;

        bool shouldClose(void) const;

        GraphicsInteractor& interactor(void);

        const std::string& vendor(void) const;

        void addItem(GraphicsGPUItem* pItem);
        void removeItem(GraphicsGPUItem* pItem);
        void addToDelete(ItemInfo* pItem);

        static GraphicsWindowImpl* currentWindow(void);
        using WindowMap = std::unordered_map<const GLFWwindow*, GraphicsWindowImpl*>;
        static const WindowMap& windowMap(void);

    protected:
        GLFWwindow* m_pWindowHandle = nullptr;
        vec2us m_windowSize = { 1280, 720 };
        std::string m_title;
        std::string m_vendor;
        GraphicsInteractor m_interactor;
        bool m_isMultiSample = true;

        std::vector<ItemInfo*> m_itemsToDelete;
        std::unordered_set<GraphicsGPUItem*> m_itemList;

        GraphicsWindow* const m_pApi = nullptr;

        static GraphicsWindowImpl* s_pCurrentWindow;
        static WindowMap s_windowMap;
    };

} // namespace FX

#endif // _GRAPHICS_WINDOW_IMPL_H_
