#ifndef _GRAPHICS_WINDOW_H_
#define _GRAPHICS_WINDOW_H_

#include <string>
#include <unordered_map>
#include <chrono>
#include <vector>
#include <unordered_set>
#include "glad.h"
#include "glfw3.h"

#include "basic_vector.h"
#include "basic_macro.h"

namespace FX {

    class GraphicsGPUItem;
    class ItemInfo;

    class GraphicsWindow {
    public:
        friend class GraphicsGPUItem;

        GraphicsWindow(unsigned short width, unsigned short height, const std::string& title = "FXGraphics", bool isMultiSample = true);
        virtual ~GraphicsWindow(void);

        DELETE_COPY_AND_MOVE_CONSTRUCT(GraphicsWindow);

    public:
        void use(void);
        void frame(void);

        vec2us size(void) const;

        bool shouldClose(void) const;

        static GraphicsWindow* currentWindow(void);

    protected:
        void addItem(GraphicsGPUItem* pItem);
        void removeItem(GraphicsGPUItem* pItem);

        void addToDelete(ItemInfo* pItem);

    protected:
        GLFWwindow* m_pWindowHandle = nullptr;
        vec2us m_windowSize = { 1280, 720 };
        vec2us m_bufferSize = { 1280, 720 };
        std::string m_title;
        const std::chrono::steady_clock::time_point m_creationTime;
        bool m_isMultiSample = true;

        std::vector<ItemInfo*> m_itemsToDelete;
        std::unordered_set<GraphicsGPUItem*> m_itemList;

        static GraphicsWindow* s_pCurrentWindow;
        using WindowMap = std::unordered_map<const GLFWwindow*, GraphicsWindow*>;
        static WindowMap s_windowMap;
    };

} // namespace FX

#endif // _GRAPHICS_WINDOW_H_
