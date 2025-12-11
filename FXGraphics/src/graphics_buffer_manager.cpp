#include "graphics_buffer_manager.h"

#include <assert.h>
#include "basic_log.h"
#include "graphics_window.h"
#include "graphics_entity_manager.h"
#include "graphics_buffer_generator.h"

namespace FX {

    GraphicsBufferManager::~GraphicsBufferManager()
    {
        for (auto&& pair : m_container)
        {
            for (auto&& buffers : pair.second)
            {
                if (buffers.pVao != nullptr)
                {
                    delete buffers.pVao;
                }
                for (auto pVbo : buffers.pVbos)
                {
                    if (pVbo != nullptr)
                    {
                        delete pVbo;
                    }
                }
                if (buffers.pEbo != nullptr)
                {
                    delete buffers.pEbo;
                }
                if (buffers.pSsbo != nullptr)
                {
                    delete buffers.pSsbo;
                }
                for (auto pDibo : buffers.pDibos)
                {
                    if (pDibo != nullptr)
                    {
                        delete pDibo;
                    }
                }
            }
        }
    }

    void GraphicsBufferManager::accept(const EntityList& list, EntityType type, int index)
    {
        if (list.isDirty() == false)
        {
            return;
        }

        auto& group = m_container[type];
        if (group.size() < index + 1)
        {
            group.resize(index + 1);
        }

        auto& buffers = group[index];
        if (buffers.init == false)
        {
            // 以下buffer指针将在析构函数中释放
            buffers.pVao = new GraphicsVAO;
            buffers.pVbos.resize(4);
            buffers.pVbos[0] = new GraphicsVBO;
            buffers.pVbos[1] = new GraphicsVBO;
            buffers.pVbos[2] = new GraphicsVBO;
            buffers.pVbos[3] = new GraphicsVBO;
            buffers.pEbo = new GraphicsEBO;
            buffers.pSsbo = new GraphicsSSBO;
            buffers.pDibos.resize(2);
            buffers.pDibos[0] = new GraphicsDIBO;
            buffers.pDibos[1] = new GraphicsDIBO;
            buffers.init = true;
        }

        for (auto&& pVbo : buffers.pVbos)
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
        assert(GraphicsWindow::currentWindow() != nullptr);
        auto& group = m_container[type];
        assert(group.size() > index);
        auto& buffers = group[index];
        assert(buffers.init == true);

        // 获取当前context下所有buffer的指针
        auto pVao = static_cast<VAOInfo*>(buffers.pVao->getOrCreate());
        auto pEbo = static_cast<EBOInfo*>(buffers.pEbo->getOrCreate());
        auto pSsbo = static_cast<SSBOInfo*>(buffers.pSsbo->getOrCreate());
        assert(pVao != nullptr && pEbo != nullptr && pSsbo != nullptr);

        std::vector<VBOInfo*> pVbos;
        pVbos.resize(buffers.pVbos.size());
        assert(buffers.pVbos.size() > 0);
        for (int i = 0; i < pVbos.size(); i++)
        {
            pVbos[i] = static_cast<VBOInfo*>(buffers.pVbos[i]->getOrCreate());
            assert(pVbos[i] != nullptr);
        }
        std::vector<DIBOInfo*> pDibos;
        pDibos.resize(buffers.pDibos.size());
        assert(buffers.pDibos.size() > 0);
        for (int i = 0; i < pDibos.size(); i++)
        {
            pDibos[i] = static_cast<DIBOInfo*>(buffers.pDibos[i]->getOrCreate());
            assert(pDibos[i] != nullptr);
        }

        // 确保dirty信息一致
        auto rebuildStart = pVbos[0]->rebuildStart();
#if defined(_DEBUG) || defined(DEBUG)
        assert(pEbo->rebuildStart() == rebuildStart && pSsbo->rebuildStart() == rebuildStart);
        for (auto pVbo : pVbos)
        {
            assert(pVbo->rebuildStart() == rebuildStart);
        }
        for (auto pDibo : pDibos)
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
                    exportVertex(pEntity, FX::vec2i{index, static_cast<int>(i)}, & vertex[list.pointSum[i] - list.pointSum[rebuildStart]]);
                    exportVertex(pEntity, FX::vec2i{index, static_cast<int>(i)}, & normal[list.pointSum[i] - list.pointSum[rebuildStart]]);
                    exportVertex(pEntity, FX::vec2i{index, static_cast<int>(i)}, & uv[list.pointSum[i] - list.pointSum[rebuildStart]]);
                    exportVertex(pEntity, FX::vec2i{index, static_cast<int>(i)}, & rank[list.pointSum[i] - list.pointSum[rebuildStart]]);
                    exportIndex(pEntity, list.pointSum[i], &indexs[list.indexSum[i] - list.indexSum[rebuildStart]]);
                    exportProfile(pEntity, &profile[i - rebuildStart]);
                }
            }

            // 只有所有顶点数据均不为空时有效
            if (vertex.empty() == false && normal.empty() == false && uv.empty() == false && rank.empty() == false &&
                indexs.empty() == false && profile.empty() == false)
            {
                pVao->bind();

                pVbos[0]->bind();
                pVbos[0]->setSubData(list.pointSum[rebuildStart] * sizeof(NormalVertexData),
                    static_cast<unsigned int>(vertex.size()) * sizeof(NormalVertexData), vertex.data());

                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
                glEnableVertexAttribArray(0);

                pVbos[1]->bind();
                pVbos[1]->setSubData(list.pointSum[rebuildStart] * sizeof(NormalNormalData),
                    static_cast<unsigned int>(normal.size()) * sizeof(NormalNormalData), normal.data());

                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
                glEnableVertexAttribArray(1);

                pVbos[2]->bind();
                pVbos[2]->setSubData(list.pointSum[rebuildStart] * sizeof(NormalUvData),
                    static_cast<unsigned int>(uv.size()) * sizeof(NormalUvData), uv.data());

                glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
                glEnableVertexAttribArray(2);

                pVbos[3]->bind();
                pVbos[3]->setSubData(list.pointSum[rebuildStart] * sizeof(NormalRankData),
                    static_cast<unsigned int>(rank.size()) * sizeof(NormalRankData), rank.data());

                glVertexAttribIPointer(3, 2, GL_INT, 2 * sizeof(int), 0);
                glEnableVertexAttribArray(3);

                pVbos[3]->unbind();
                pVao->unbind();

                pEbo->bind();
                pEbo->setSubData(list.indexSum[rebuildStart] * sizeof(unsigned int),
                    static_cast<unsigned int>(indexs.size()) * sizeof(unsigned int), indexs.data());
                pEbo->unbind();

                pSsbo->bind();
                pSsbo->setSubData(rebuildStart * sizeof(NormalProfileData),
                    static_cast<unsigned int>(profile.size()) * sizeof(NormalProfileData), profile.data());
                pSsbo->unbind();
            }
            else
            {
                BasicLog::out(BasicLog::kWarn, "There are empty vertex data in entity list, please check your data.");
            }
        }

        // 处理list dirty
        if (pSsbo->dirtyList().empty() == false)
        {
            pSsbo->bind();

            for (auto i : pSsbo->dirtyList())
            {
                if (i >= list.entityList.size() || list.entityList[i] == nullptr)
                {
                    assert(0);
                    continue;
                }

                NormalProfileData profile;
                exportProfile(list.entityList[i], &profile);

                pSsbo->setSubData(i * sizeof(NormalProfileData), sizeof(profile), &profile);
            }

            pSsbo->unbind();
        }

        // 有dirty就重新生成全部command
        // TODO 优化
        if (rebuildStart >= 0 || pDibos[0]->dirtyList().empty() == false || pDibos[1]->dirtyList().empty() == false)
        {
            std::vector<DrawElementsCommand> opaqueCommand;
            std::vector<DrawElementsCommand> transCommand;
            opaqueCommand.reserve(10);
            transCommand.reserve(10);

            enum class CommandType : unsigned char {
                kNone = 0,
                kOpaque,
                kTrans
            };
            CommandType lastCommand = CommandType::kNone;

            for (int i = 0; i < list.entityList.size(); i++)
            {
                auto pEntity = list.entityList[i];
                if (pEntity != nullptr && pEntity->profile().visible)
                {
                    auto num = list.indexSum[i + 1] - list.indexSum[i];
                    if (pEntity->profile().color.a == 255 && list.pFont == nullptr)
                    {
                        if (lastCommand == CommandType::kOpaque)
                        {
                            assert(opaqueCommand.empty() == false);
                            assert(opaqueCommand.back().indexStart + opaqueCommand.back().indexNum == list.indexSum[i]);
                            opaqueCommand.back().indexNum += num;
                        }
                        else
                        {
                            opaqueCommand.emplace_back(DrawElementsCommand{ num, 1, list.indexSum[i], 0, 0 });
                            lastCommand = CommandType::kOpaque;
                        }
                    }
                    else
                    {
                        if (lastCommand == CommandType::kTrans)
                        {
                            assert(transCommand.empty() == false);
                            assert(transCommand.back().indexStart + transCommand.back().indexNum == list.indexSum[i]);
                            transCommand.back().indexNum += num;
                        }
                        else
                        {
                            transCommand.emplace_back(DrawElementsCommand{ num, 1, list.indexSum[i], 0, 0 });
                            lastCommand = CommandType::kTrans;
                        }
                    }
                }
                else
                {
                    lastCommand = CommandType::kNone;
                }
            }

            pDibos[0]->setCommandNum(static_cast<unsigned int>(opaqueCommand.size()));
            pDibos[1]->setCommandNum(static_cast<unsigned int>(transCommand.size()));

            if (opaqueCommand.empty() == false)
            {
                pDibos[0]->bind();
                pDibos[0]->setData(static_cast<unsigned int>(opaqueCommand.size()) * sizeof(DrawElementsCommand), opaqueCommand.data());
                pDibos[0]->unbind();
            }
            if (transCommand.empty() == false)
            {
                pDibos[1]->bind();
                pDibos[1]->setData(static_cast<unsigned int>(transCommand.size()) * sizeof(DrawElementsCommand), transCommand.data());
                pDibos[1]->unbind();
            }
        }

        pEbo->setReady();
        pSsbo->setReady();
        for (auto&& pVbo : pVbos)
        {
            pVbo->setReady();
        }
        for (auto&& pDibo : pDibos)
        {
            pDibo->setReady();
        }

        return buffers;
    }

} // namespace FX
