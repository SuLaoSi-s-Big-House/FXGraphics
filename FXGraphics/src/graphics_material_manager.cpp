#include "graphics_material_manager.h"

#include <assert.h>
#include "basic_log.h"
#include "graphics_window.h"

namespace FX {

    namespace {

        inline bool isLess(void)
        {
            return false;
        }

        template<typename Q>
        inline bool isLess(const vec4<Q>& left, const vec4<Q>& right)
        {
            if (left.x != right.x)
            {
                return left.x < right.x;
            }
            else if (left.y != right.y)
            {
                return left.y < right.y;
            }
            else if (left.z != right.z)
            {
                return left.z < right.z;
            }
            else
            {
                return left.w < right.w;
            }
        }

        template<typename T>
        inline bool isLess(const T& left, const T& right)
        {
            return left < right;
        }

        template<typename T, typename... Args>
        inline bool isLess(const T& left, const T& right, const Args&... args)
        {
            if (left == right)
            {
                return isLess(args...);
            }
            else
            {
                return isLess(left, right);
            }
        }

    }  // namespace

    bool Font::operator==(const Font& other) const
    {
        return this->name == other.name && this->size == other.size;
    }

    bool Font::operator<(const Font& other) const
    {
        return this->name == other.name ? this->size < other.size : this->name < other.name;
    }

    bool Font::valid() const
    {
        return name.empty() == false && size > 0;
    }

    bool Material::operator==(const Material& other) const
    {
        return this->baseColor == other.baseColor &&
            this->visible == other.visible &&
            this->selectable == other.selectable &&
            this->metallic == other.metallic &&
            this->roughness == other.roughness &&
            this->font == other.font &&
            this->custom1 == other.custom1 &&
            this->custom2 == other.custom2;
    }

    bool Material::operator<(const Material& other) const
    {
        return isLess(this->baseColor, other.baseColor,
            this->metallic, other.metallic,
            this->roughness, other.roughness,
            this->font, other.font,
            this->visible, other.visible,
            this->selectable, other.selectable,
            this->custom1, other.custom1,
            this->custom2, other.custom2);
    }


    GraphicsMaterialManager::GraphicsMaterialManager()
    {
        // 构造时添加默认材质，在生命周期内一直存在
        m_materialList.reserve(10);
        m_materialList.resize(DefaultMaterialHandle + 1);
        auto ret = m_materialMap.insert({ Material(), DefaultMaterialHandle });
        m_materialList[DefaultMaterialHandle] = { ret.first, 1 };
        m_availableList.reserve(10);
        m_dirtyList.insert(static_cast<int>(DefaultMaterialHandle));
    }

    GraphicsMaterialManager& GraphicsMaterialManager::instance()
    {
        static GraphicsMaterialManager instance;
        return instance;
    }

    MaterialHandle GraphicsMaterialManager::ref(const Material& material)
    {
        if (material == Material())
        {
            return DefaultMaterialHandle;
        }

        auto itr = m_materialMap.find(material);
        if (itr != m_materialMap.end())
        {
            MaterialHandle handle = itr->second;
            assert(m_materialList[handle].itr->second == handle);
            assert(m_materialList[handle].refCount > 0);
            m_materialList[handle].refCount++;
            return handle;
        }

        MaterialHandle handle = 0;
        if (m_availableList.empty() == false)
        {
            handle = m_availableList.back();
            m_availableList.pop_back();
            assert(m_materialList[handle].itr == m_materialMap.end());
        }
        else
        {
            handle = static_cast<MaterialHandle>(m_materialList.size());
            m_materialList.emplace_back(MaterialRef());
        }

        auto ret = m_materialMap.insert({ material, handle });
        m_materialList[handle] = { ret.first, 1 };
        m_dirtyList.insert(static_cast<int>(handle));
        return handle;
    }

    void GraphicsMaterialManager::unref(MaterialHandle handle)
    {
        if (handle == DefaultMaterialHandle)
        {
            return;
        }

        auto& data = m_materialList[handle];
        assert(data.refCount > 0);
        data.refCount--;

        if (data.refCount == 0)
        {
            m_materialMap.erase(data.itr);
            data.itr = m_materialMap.end();
            m_availableList.push_back(handle);
            m_dirtyList.erase(static_cast<int>(handle));
        }
    }

    const Material& GraphicsMaterialManager::get(MaterialHandle handle) const
    {
        if (handle >= m_materialList.size() || m_materialList[handle].itr == m_materialMap.end())
        {
            BasicLog::out(BasicLog::kWarn, "Invalid material handle when querying materials.");
            assert(0);
            assert(m_materialList[DefaultMaterialHandle].itr != m_materialMap.end());
            return m_materialList[DefaultMaterialHandle].itr->first;
        }

        return m_materialList[handle].itr->first;
    }

    void GraphicsMaterialManager::bind(BufferSlot slot)
    {
        assert(GraphicsWindow::currentWindow() != nullptr);

        auto exportMaterial = [](const Material& material) -> BufferBlock {
            return BufferBlock{
                vec4f {
                    material.baseColor.r / 255.0f,
                    material.baseColor.g / 255.0f,
                    material.baseColor.b / 255.0f,
                    material.baseColor.a / 255.0f,
                },
                vec4f { material.metallic, material.roughness, 0.0f, 0.0f },
                material.custom1,
                material.custom2
            };
        };

        m_buffer.addDirtyList(m_dirtyList);
        m_dirtyList.clear();

        auto pBuffer = static_cast<SSBOInfo*>(m_buffer.getOrCreate());

        // 如果是当前context下新建的buffer，需要同步所有material
        if (pBuffer->rebuildStart() == 0)
        {
            std::vector<BufferBlock> bufferData;
            bufferData.reserve(m_materialList.size());

            for (auto& ref : m_materialList)
            {
                if (ref.itr != m_materialMap.end())
                {
                    bufferData.emplace_back(exportMaterial(ref.itr->first));
                }
                else
                {
                    bufferData.emplace_back(BufferBlock());
                }
            }

            pBuffer->bind();
            pBuffer->setSubData(0, static_cast<unsigned int>(bufferData.size() * sizeof(BufferBlock)), bufferData.data());
            pBuffer->cleanDirty();
            pBuffer->unbind();
        }
        else
        {
            auto& dirtyList = pBuffer->dirtyList();

            if (dirtyList.empty() == false)
            {
                pBuffer->bind();

                // dirtyList为递增序列，从后向前遍历，减少buffer扩容次数
                for (auto itr = dirtyList.rbegin(); itr != dirtyList.rend(); itr++)
                {
                    auto handle = *itr;
                    assert(handle < m_materialList.size());
                    if (m_materialList[handle].itr != m_materialMap.end())
                    {
                        auto block = exportMaterial(m_materialList[handle].itr->first);
                        pBuffer->setSubData(handle * sizeof(BufferBlock), sizeof(BufferBlock), &block);
                    }
                }

                pBuffer->cleanDirty();
                pBuffer->unbind();
            }
        }

        pBuffer->bind(slot);
    }

} // namespace FX
