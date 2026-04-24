#include "graphics_buffer_manager.h"

#include <assert.h>
#include <array>
#include "glad.h"
#include "basic_log.h"
#include "graphics_window.h"
#include "graphics_entity_manager.h"
#include "graphics_buffer_generator.h"

namespace FX {

    namespace {
        constexpr unsigned int PositionBufferIndex = 0;
        constexpr unsigned int NormalBufferIndex = 1;
        constexpr unsigned int UvBufferIndex = 2;
        constexpr unsigned int RankBufferIndex = 3;
        constexpr unsigned int VertexBufferNum = 4;
    }  // namespace

    void GraphicsBufferManager::accept(const EntityList& list, EntityType type, int index)
    {
        if (list.entityList.empty() || list.isDirty() == false)
        {
            return;
        }

        auto& group = m_container[type];
        if (group.size() < index + 1)
        {
            group.resize(index + 1);
        }

        auto& buffers = group[index];
        if (buffers.pVbos.empty())
        {
            buffers.pVao.reset(new GraphicsVAO);
            buffers.pVbos.resize(VertexBufferNum);
            for (unsigned int i = 0; i < VertexBufferNum; i++)
            {
                buffers.pVbos[i].reset(new GraphicsVBO);
            }
            buffers.pEbo.reset(new GraphicsEBO);
            buffers.pSsbo.reset(new GraphicsSSBO);
            buffers.pDibos.resize(NormalCommandNum);
            for (unsigned int i = 0; i < NormalCommandNum; i++)
            {
                buffers.pDibos[i].reset(new GraphicsDIBO);
            }
        }
#if defined(_DEBUG) || defined(DEBUG)
        else
        {
            assert(buffers.pVao != nullptr);
            assert(buffers.pEbo != nullptr);
            assert(buffers.pSsbo != nullptr);
            assert(buffers.pDibos.empty() == false);
        }
#endif

        for (auto& pVbo : buffers.pVbos)
        {
            pVbo->setRebuildStart(list.rebuildStart);
        }
        buffers.pEbo->setRebuildStart(list.rebuildStart);
        buffers.pSsbo->setRebuildStart(list.rebuildStart);
        buffers.pSsbo->addDirtyList(list.profileList);
        buffers.pSsbo->addDirtyList(list.matrixList);
        for (auto& pDibo : buffers.pDibos)
        {
            pDibo->setRebuildStart(list.rebuildStart);
            pDibo->addDirtyList(list.commandList);
        }
    }

    const BufferSet& GraphicsBufferManager::generate(const EntityList& list, EntityType type, int index)
    {
        assert(list.entityList.empty() == false);
        assert(list.isDirty() == false);
        assert(GraphicsWindow::currentWindow() != nullptr);
        auto& group = m_container[type];
        assert(group.size() > index);
        auto& buffers = group[index];
        assert(buffers.pVbos.empty() == false);

        // 获取当前context下所有buffer的指针
        struct BufferInfoSet {
            VAOInfo* pVao = nullptr;
            std::array<VBOInfo*, VertexBufferNum> pVbos;
            EBOInfo* pEbo = nullptr;
            SSBOInfo* pSsbo = nullptr;
            std::array<DIBOInfo*, NormalCommandNum> pDibos;
        } infos;

        infos.pVao = static_cast<VAOInfo*>(buffers.pVao->getOrCreate());
        assert(infos.pVbos.size() == buffers.pVbos.size());
        for (unsigned int i = 0; i < VertexBufferNum; i++)
        {
            infos.pVbos[i] = static_cast<VBOInfo*>(buffers.pVbos[i]->getOrCreate());
            assert(infos.pVbos[i] != nullptr);
        }
        infos.pEbo = static_cast<EBOInfo*>(buffers.pEbo->getOrCreate());
        infos.pSsbo = static_cast<SSBOInfo*>(buffers.pSsbo->getOrCreate());
        assert(infos.pDibos.size() == buffers.pDibos.size());
        for (unsigned int i = 0; i < NormalCommandNum; i++)
        {
            infos.pDibos[i] = static_cast<DIBOInfo*>(buffers.pDibos[i]->getOrCreate());
            assert(infos.pDibos[i] != nullptr);
        }
        assert(infos.pVao != nullptr && infos.pEbo != nullptr && infos.pSsbo != nullptr);

        // 确保dirty信息一致
        auto rebuildStart = infos.pVbos[PositionBufferIndex]->rebuildStart();
#if defined(_DEBUG) || defined(DEBUG)
        assert(infos.pEbo->rebuildStart() == rebuildStart && infos.pSsbo->rebuildStart() == rebuildStart);
        for (auto pVbo : infos.pVbos)
        {
            assert(pVbo->rebuildStart() == rebuildStart);
        }
        for (auto pDibo : infos.pDibos)
        {
            assert(pDibo->rebuildStart() == rebuildStart);
        }
#endif

        // 处理rebuild dirty
        if (rebuildStart >= 0)
        {
            std::vector<NormalVertexData> vertex;
            std::vector<NormalNormalData> normal;
            std::vector<NormalUvData> uv;
            std::vector<NormalRankData> rank;
            std::vector<unsigned int> indexs;
            std::vector<NormalProfileData> profile;

            vertex.resize(list.pointSum.back() - list.pointSum[rebuildStart]);
            normal.resize(list.pointSum.back() - list.pointSum[rebuildStart]);
            uv.resize(list.pointSum.back() - list.pointSum[rebuildStart]);
            rank.resize(list.pointSum.back() - list.pointSum[rebuildStart]);
            indexs.resize(list.indexSum.back() - list.indexSum[rebuildStart], RestartMark);
            profile.resize(list.entityList.size() - rebuildStart);

            for (auto i = static_cast<unsigned int>(rebuildStart); i < list.entityList.size(); i++)
            {
                auto pEntity = list.entityList[i];
                if (pEntity != nullptr)
                {
                    exportVertex(pEntity, FX::vec2i{index, static_cast<int>(i)}, &vertex[list.pointSum[i] - list.pointSum[rebuildStart]]);
                    exportVertex(pEntity, FX::vec2i{index, static_cast<int>(i)}, &normal[list.pointSum[i] - list.pointSum[rebuildStart]]);
                    exportVertex(pEntity, FX::vec2i{index, static_cast<int>(i)}, &uv[list.pointSum[i] - list.pointSum[rebuildStart]]);
                    exportVertex(pEntity, FX::vec2i{index, static_cast<int>(i)}, &rank[list.pointSum[i] - list.pointSum[rebuildStart]]);
                    exportIndex(pEntity, list.pointSum[i], &indexs[list.indexSum[i] - list.indexSum[rebuildStart]]);
                    exportProfile(pEntity, &profile[i - rebuildStart]);
                }
            }

            // 只有所有顶点数据均不为空时有效
            if (vertex.empty() == false && normal.empty() == false && uv.empty() == false && rank.empty() == false &&
                indexs.empty() == false && profile.empty() == false)
            {
                infos.pVao->bind();

                infos.pVbos[PositionBufferIndex]->bind();
                infos.pVbos[PositionBufferIndex]->setSubData(list.pointSum[rebuildStart] * sizeof(NormalVertexData),
                    static_cast<unsigned int>(vertex.size()) * sizeof(NormalVertexData), vertex.data());

                glVertexAttribPointer(PositionBufferIndex, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
                glEnableVertexAttribArray(PositionBufferIndex);

                infos.pVbos[NormalBufferIndex]->bind();
                infos.pVbos[NormalBufferIndex]->setSubData(list.pointSum[rebuildStart] * sizeof(NormalNormalData),
                    static_cast<unsigned int>(normal.size()) * sizeof(NormalNormalData), normal.data());

                glVertexAttribPointer(NormalBufferIndex, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
                glEnableVertexAttribArray(NormalBufferIndex);

                infos.pVbos[UvBufferIndex]->bind();
                infos.pVbos[UvBufferIndex]->setSubData(list.pointSum[rebuildStart] * sizeof(NormalUvData),
                    static_cast<unsigned int>(uv.size()) * sizeof(NormalUvData), uv.data());

                glVertexAttribPointer(UvBufferIndex, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
                glEnableVertexAttribArray(UvBufferIndex);

                infos.pVbos[RankBufferIndex]->bind();
                infos.pVbos[RankBufferIndex]->setSubData(list.pointSum[rebuildStart] * sizeof(NormalRankData),
                    static_cast<unsigned int>(rank.size()) * sizeof(NormalRankData), rank.data());

                glVertexAttribIPointer(RankBufferIndex, 2, GL_INT, 2 * sizeof(int), 0);
                glEnableVertexAttribArray(RankBufferIndex);

                infos.pVbos[RankBufferIndex]->unbind();
                infos.pVao->unbind();

                infos.pEbo->bind();
                infos.pEbo->setSubData(list.indexSum[rebuildStart] * sizeof(unsigned int),
                    static_cast<unsigned int>(indexs.size()) * sizeof(unsigned int), indexs.data());
                infos.pEbo->unbind();

                infos.pSsbo->bind();
                infos.pSsbo->setSubData(rebuildStart * sizeof(NormalProfileData),
                    static_cast<unsigned int>(profile.size()) * sizeof(NormalProfileData), profile.data());
                infos.pSsbo->unbind();
            }
            else
            {
                BasicLog::out(BasicLog::kWarn, "There are empty vertex data in entity list, please check your data.");
            }
        }

        // 处理list dirty
        if (infos.pSsbo->dirtyList().empty() == false)
        {
            infos.pSsbo->bind();
            
            NormalProfileData profile;

            for (auto i : infos.pSsbo->dirtyList())
            {
                if (i >= list.entityList.size() || list.entityList[i] == nullptr)
                {
                    assert(0);
                    continue;
                }

                exportProfile(list.entityList[i], &profile);

                infos.pSsbo->setSubData(i * sizeof(NormalProfileData), sizeof(profile), &profile);
            }

            infos.pSsbo->unbind();
        }

        // 有dirty就重新生成全部command
        // TODO 优化
        if (rebuildStart >= 0 || infos.pDibos[NormalOpaqueCommand]->dirtyList().empty() == false ||
            infos.pDibos[NormalTransCommand]->dirtyList().empty() == false)
        {
            std::vector<DrawElementsCommand> opaqueCommand;
            std::vector<DrawElementsCommand> transCommand;
            opaqueCommand.reserve(10);
            transCommand.reserve(10);

            auto lastCommand = NormalCommandNum;

            for (int i = 0; i < list.entityList.size(); i++)
            {
                auto pEntity = list.entityList[i];
                if (pEntity != nullptr && true) // visible
                {
                    auto num = list.indexSum[i + 1] - list.indexSum[i];
                    if (true) // opaque
                    {
                        if (lastCommand == NormalOpaqueCommand)
                        {
                            assert(opaqueCommand.empty() == false);
                            assert(opaqueCommand.back().indexStart + opaqueCommand.back().indexNum == list.indexSum[i]);
                            opaqueCommand.back().indexNum += num;
                        }
                        else
                        {
                            opaqueCommand.emplace_back(DrawElementsCommand{ num, 1, list.indexSum[i], 0, 0 });
                            lastCommand = NormalOpaqueCommand;
                        }
                    }
                    else
                    {
                        if (lastCommand == NormalTransCommand)
                        {
                            assert(transCommand.empty() == false);
                            assert(transCommand.back().indexStart + transCommand.back().indexNum == list.indexSum[i]);
                            transCommand.back().indexNum += num;
                        }
                        else
                        {
                            transCommand.emplace_back(DrawElementsCommand{ num, 1, list.indexSum[i], 0, 0 });
                            lastCommand = NormalTransCommand;
                        }
                    }
                }
                else
                {
                    lastCommand = NormalCommandNum;
                }
            }

            infos.pDibos[NormalOpaqueCommand]->setCommandNum(static_cast<unsigned int>(opaqueCommand.size()));
            infos.pDibos[NormalTransCommand]->setCommandNum(static_cast<unsigned int>(transCommand.size()));

            if (opaqueCommand.empty() == false)
            {
                infos.pDibos[NormalOpaqueCommand]->bind();
                infos.pDibos[NormalOpaqueCommand]->setData(static_cast<unsigned int>(opaqueCommand.size()) * sizeof(DrawElementsCommand), opaqueCommand.data());
                infos.pDibos[NormalOpaqueCommand]->unbind();
            }
            if (transCommand.empty() == false)
            {
                infos.pDibos[NormalTransCommand]->bind();
                infos.pDibos[NormalTransCommand]->setData(static_cast<unsigned int>(transCommand.size()) * sizeof(DrawElementsCommand), transCommand.data());
                infos.pDibos[NormalTransCommand]->unbind();
            }
        }

        infos.pEbo->setReady();
        infos.pSsbo->setReady();
        for (auto pVbo : infos.pVbos)
        {
            pVbo->setReady();
        }
        for (auto pDibo : infos.pDibos)
        {
            pDibo->setReady();
        }

        return buffers;
    }

} // namespace FX
