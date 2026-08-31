#include "graphics_entity_manager.h"

#include <assert.h>
#include "basic_log.h"
#include "graphics_texture_manager.h"

namespace FX {

    namespace {

        // 估算EntityList容量，避免EntityList过短导致绘制效率低，和EntityList过长导致修改效率低
        constexpr unsigned int MIN_LIST_ENTITY_NUM = 1000;
        constexpr unsigned int MAX_LIST_ENTITY_NUM = 10000;

        // 当EntityList中（估计）累计顶点数或索引数超出上限时，不再接纳新实体
        constexpr unsigned int MAX_LIST_POINT_NUM = 1000000;
        constexpr unsigned int MAX_LIST_INDEX_NUM = 1000000;

        // 判断EntityList是否应接纳pEntity。实体数处于[MIN_LIST_ENTITY_NUM, MAX_LIST_ENTITY_NUM)时，
        // 已提交部分用pointSum/indexSum精确读取，未提交部分用均值估算，据此判断总顶点数与索引数是否超限。
        bool shouldAcceptEntity(const EntityList& list, const GraphicsEntity* pEntity)
        {
            assert(pEntity != nullptr);

            const auto slots = list.entityList.size();
            const auto live = slots - list.invalidNum;

            if (live >= MAX_LIST_ENTITY_NUM)
            {
                return false;
            }

            if (live < MIN_LIST_ENTITY_NUM)
            {
                return true;
            }

            // 列表从未generate过，没有可用的尺寸统计（典型如首帧前的批量加载），保守拒绝
            if (list.pointSum.empty())
            {
                return false;
            }

            // pointSum/indexSum精确覆盖[0, covered)槽位，与当前buffer内容一致：已删除实体的
            // 残留数据仍占据buffer，计入是保守的正确行为。列表被整体清空后pointSum会残留旧值
            // （covered超过当前槽位数），此时弃用已提交部分。
            const auto covered = list.pointSum.size() - 1;
            const auto committedP = covered <= slots ? list.pointSum.back() : 0;
            const auto committedI = covered <= slots ? list.indexSum.back() : 0;

            // 自上次generate以来追加的槽位：dirty时rebuildStart即已提交边界，clean时无未提交部分
            const auto tailSlots = list.rebuildStart >= 0 ? slots - list.rebuildStart : 0;

            // 候选实体：非data-dirty时pointNum精确；data-dirty但已生成过时旧数据仍是不错的估计；
            // 从未生成过（pointNum为0）退化为列表均值
            const bool candReady = pEntity->isDataDirty() == false || pEntity->pointNum() > 0;
            const auto candP = candReady ? pEntity->pointNum() : list.pointAvg;
            const auto candI = candReady ? pEntity->indexNum() + 1 : list.indexAvg;    // +1为图元重启标志，与indexSum口径一致

            const auto estP = committedP + static_cast<unsigned long long>(list.pointAvg) * tailSlots + candP;
            const auto estI = committedI + static_cast<unsigned long long>(list.indexAvg) * tailSlots + candI;

            return estP <= MAX_LIST_POINT_NUM && estI <= MAX_LIST_INDEX_NUM;
        }

        inline bool isSameList(const TextureKey& left, const TextureKey& right)
        {
            auto& manager = GraphicsTextureManager::instance();
            for (unsigned int i = 0; i < TextureSlotNum; i++)
            {
                auto imageHandle1 = left.handle(i);
                auto imageHandle2 = right.handle(i);

                if (imageHandle1 != imageHandle2)
                {
                    auto textureHandle1 = manager.query(imageHandle1).textureHandle;
                    auto textureHandle2 = manager.query(imageHandle2).textureHandle;
                    if (textureHandle1 != textureHandle2)
                    {
                        return false;
                    }
                }
            }

            return true;
        }

    }  // namespace

    void EntityList::setRebuildStart(int start)
    {
        if (rebuildStart < 0 || start < rebuildStart)
        {
            rebuildStart = start;
        }
    }

    void EntityList::arrange()
    {
        if (entityList.empty() || invalidNum == 0)
        {
            return;
        }

        if (entityList.size() == invalidNum)
        {
            entityList.clear();
            rebuildStart = 0;
            invalidNum = 0;
            return;
        }

        if (invalidNum < 50 || (invalidNum < 300 && invalidNum < entityList.size() * 0.5))
        {
            return;
        }

        int i = 0;

        while (i < entityList.size() && entityList[i] != nullptr)
        {
            i++;
        }

        if (i == entityList.size())
        {
            assert(0);
            invalidNum = 0;
            return;
        }

        setRebuildStart(i);

        int j = i + 1;

        while (j < entityList.size() && entityList[j] == nullptr)
        {
            j++;
        }

        if (j == entityList.size())
        {
            entityList.resize(i);
            invalidNum = 0;
            return;
        }

        while (true)
        {
            assert(entityList[i] == nullptr);
            assert(entityList[j] != nullptr);

            entityList[i] = entityList[j];
            entityList[j] = nullptr;
            entityList[i]->setGroupPos(pOwner, { index, i });

            while (i < entityList.size() && entityList[i] != nullptr)
            {
                i++;
            }

            while (j < entityList.size() && entityList[j] == nullptr)
            {
                j++;
            }

            if (j == entityList.size())
            {
                break;
            }
        }

        entityList.resize(i);
        invalidNum = 0;
    }

    void EntityList::clean()
    {
        if (rebuildStart < 0 || (matrixList.empty() && profileList.empty() && commandList.empty()))
        {
            return;
        }

        auto itr = matrixList.lower_bound(rebuildStart);
        matrixList.erase(itr, matrixList.end());
        itr = profileList.lower_bound(rebuildStart);
        profileList.erase(itr, profileList.end());
        itr = commandList.lower_bound(rebuildStart);
        commandList.erase(itr, commandList.end());
    }

    void EntityList::generate()
    {
        if (entityList.empty() || rebuildStart < 0)
        {
            return;
        }

        pointSum.resize(entityList.size() + 1, 0);
        indexSum.resize(entityList.size() + 1, 0);

        for (auto i = static_cast<unsigned int>(rebuildStart); i < entityList.size(); i++)
        {
            auto pEntity = entityList[i];
            if (pEntity != nullptr)
            {
                if (pEntity->isDataDirty())
                {
                    pEntity->generate();
                    pEntity->setDataReady();
                }

                pointSum[i + 1] = pointSum[i] + pEntity->pointNum();
                indexSum[i + 1] = indexSum[i] + pEntity->indexNum() + 1;
            }
            else
            {
                pointSum[i + 1] = pointSum[i];
                indexSum[i + 1] = indexSum[i];
            }
        }

        // 同步尺寸统计，供shouldAcceptEntity估算未提交部分。分母用活实体数而非槽位数，
        // 使残留数据只增大均值（保守方向）而不被死槽位稀释。
        const auto live = entityList.size() - invalidNum;
        if (live > 0)
        {
            pointAvg = static_cast<unsigned int>(pointSum.back() / live);
            indexAvg = static_cast<unsigned int>(indexSum.back() / live);
        }
    }

    bool EntityList::isDirty() const
    {
        return rebuildStart >= 0 || !matrixList.empty() || !profileList.empty() || !commandList.empty();
    }

    void EntityList::setReady()
    {
        rebuildStart = -1;
        matrixList.clear();
        profileList.clear();
        commandList.clear();
    }

    GraphicsEntityManager::~GraphicsEntityManager()
    {
        for (auto&& pair : m_container)
        {
            for (auto&& list : pair.second)
            {
                for (auto pEntity : list.entityList)
                {
                    if (pEntity != nullptr)
                    {
                        pEntity->eraseGroup(this);
                    }
                }
            }
        }
    }

    bool GraphicsEntityManager::addEntity(GraphicsEntity* pEntity)
    {
        assert(pEntity != nullptr);

        if (pEntity->groupPos(this).valid())
        {
            return false;
        }

        auto& list = findBestListForEntity(pEntity);
        int j = static_cast<int>(list.entityList.size());

        list.entityList.push_back(pEntity);
        list.setRebuildStart(j);

        pEntity->setGroupPos(this, { list.index, j });

        return true;
    }

    bool GraphicsEntityManager::removeEntity(GraphicsEntity* pEntity)
    {
        assert(pEntity != nullptr);

        auto pos = pEntity->groupPos(this);
        if (!pos.valid())
        {
            return false;
        }

        auto& list = m_container[pEntity->type()][pos.first];
        assert(list.entityList[pos.second] == pEntity);
        list.entityList[pos.second] = nullptr;
        list.invalidNum++;
        list.commandList.insert(pos.second);
        list.matrixList.erase(pos.second);
        list.profileList.erase(pos.second);
        pEntity->eraseGroup(this);

        return true;
    }

    bool GraphicsEntityManager::dirtyEntity(GraphicsEntity* pEntity, DirtyType type)
    {
        assert(pEntity != nullptr);

        if (type == 0)
        {
            return false;
        }

        auto pos = pEntity->groupPos(this);
        if (!pos.valid())
        {
            return false;
        }

        auto& list = m_container[pEntity->type()][pos.first];
        assert(list.entityList[pos.second] == pEntity);

        if (type & FontDirty && isFontType(pEntity->type()))
        {
            list.entityList[pos.second] = nullptr;
            list.invalidNum++;
            list.commandList.insert(pos.second);
            list.matrixList.erase(pos.second);
            list.profileList.erase(pos.second);
            pEntity->eraseGroup(this);

            addEntity(pEntity);
            return true;
        }

        if (isTextureType(pEntity->type()))
        {
            if (type & TextureDirty)
            {
                list.entityList[pos.second] = nullptr;
                list.invalidNum++;
                list.commandList.insert(pos.second);
                list.matrixList.erase(pos.second);
                list.profileList.erase(pos.second);
                pEntity->eraseGroup(this);

                addEntity(pEntity);
                return true;
            }
            else if (type & ImageDirty)
            {
                list.commandList.insert(pos.second);
                list.profileList.insert(pos.second);
            }
        }

        if (list.rebuildStart >= 0 && list.rebuildStart <= pos.second)
        {
            return true;
        }

        if (type & DataDirty)
        {
            if (pos.second == list.entityList.size() - 1)
            {
                list.setRebuildStart(pos.second);
            }
            else
            {
                auto j = static_cast<int>(list.entityList.size());
                list.setRebuildStart(j);
                list.entityList.push_back(pEntity);
                list.entityList[pos.second] = nullptr;
                list.invalidNum++;
                pEntity->setGroupPos(this, { pos.first, j });
            }
            return true;
        }

        if (type & TransparencyDirty)
        {
            list.commandList.insert(pos.second);
            list.profileList.insert(pos.second);
        }

        if (type & MatrixDirty)
        {
            list.matrixList.insert(pos.second);
        }

        if (type & ColorDirty)
        {
            list.profileList.insert(pos.second);
        }

        if (type & VisibleDirty)
        {
            list.commandList.insert(pos.second);
        }

        return true;
    }

    EntityList& GraphicsEntityManager::findBestListForEntity(GraphicsEntity* pEntity)
    {
        assert(pEntity != nullptr);
        assert(pEntity->groupPos(this).valid() == false);

        auto type = pEntity->type();
        auto& group = m_container[type];
        int i = 0;

        if (isFontType(type))
        {
            if (pEntity->profile().font.valid() == false)
            {
                BasicLog::out(BasicLog::kWarn, "Trying to add a text entity with invalid font, which may lead to unexpected behavior.");
            }

            for (; i < group.size(); i++)
            {
                if (group[i].font == pEntity->profile().font && shouldAcceptEntity(group[i], pEntity))
                {
                    break;
                }
            }

            if (i == group.size())
            {
                EntityList newList;
                newList.pOwner = this;
                newList.index = i;
                newList.font = pEntity->profile().font;
                group.emplace_back(std::move(newList));
            }
        }
        else if (isTextureType(type))
        {
            for (; i < group.size(); i++)
            {
                if (isSameList(group[i].texture, pEntity->profile().texture) && shouldAcceptEntity(group[i], pEntity))
                {
                    break;
                }
            }

            if (i == group.size())
            {
                EntityList newList;
                newList.pOwner = this;
                newList.index = i;
                newList.texture = pEntity->profile().texture;
                group.emplace_back(std::move(newList));
            }
        }
        else
        {
            for (; i < group.size(); i++)
            {
                if (shouldAcceptEntity(group[i], pEntity))
                {
                    break;
                }
            }

            if (i == group.size())
            {
                EntityList newList;
                newList.pOwner = this;
                newList.index = i;
                group.emplace_back(std::move(newList));
            }
        }

        return group[i];
    }

} // namespace FX
