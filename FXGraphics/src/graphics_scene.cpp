#include "graphics_scene.h"

#include <assert.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include "glad.h"
#include "glm.hpp"
#include "basic_log.h"
#include "graphics_window.h"
#include "graphics_printer.h"
#include "graphics_camera.h"
#include "graphics_texture.h"
#include "graphics_font_manager.h"
#include "graphics_texture_manager.h"

namespace FX {

    namespace {

        struct NormalGlobalInfo {
            glm::mat4 vMatrix = glm::mat4(1.0f);
            glm::mat4 pMatrix = glm::mat4(1.0f);
            glm::mat4 vpMatrix = glm::mat4(1.0f);
            vec2i viewport = { 0, 0 };
        };

        // 并行生成的门槛：当帧待重建数据量（顶点+索引合计）达到该值时才启用多线程
        constexpr unsigned long long PARALLEL_GENERATE_VOLUME = 30000;

        // 冷列表（从未generate过，无统计数据，典型如首帧前的批量加载）中每个实体的保守数据量估算。
        // 宁大勿小：偏大只是多付一次微秒级的调度开销，偏小会让毫秒级的负载留在单线程。
        constexpr unsigned long long COLD_ENTITY_VOLUME_ESTIMATE = 32;

        // 建议的worker线程数：最多4个，且为硬件并发数减1（为主线程留一核）。
        // 返回0表示不适合多线程。
        unsigned int suggestWorkerNum(void)
        {
            auto num = std::thread::hardware_concurrency();
            if (num <= 2)
            {
                return 0;
            }

            return std::min(4u, num - 1);
        }

    }  // namespace

    // GraphicsScene::generate的数据脏任务：需要重建顶点数据的EntityList及其定位信息
    struct GraphicsSceneDirtyTask {
        EntityList* pList;
        EntityType type;
        int index;
    };

    // GraphicsSceneWorker实现GraphicsScene::generate的多线程生成。
    // 所有共享状态由m_mtx保护，worker仅在领取任务与归还计数时短暂持锁，生成工作在锁外进行。
    // 跨帧复用安全性依赖一个不变量：m_remaining减到0意味着所有已领取的任务都已归还计数，
    // 因此dispatch重置状态时不存在旧帧worker递减新帧计数的交错。
    class GraphicsSceneWorker {
    public:
        explicit GraphicsSceneWorker(unsigned int num)
        {
            assert(num >= 2);

            m_threads.reserve(num);

            try
            {
                for (unsigned int i = 0; i < num; i++)
                {
                    m_threads.emplace_back(&GraphicsSceneWorker::work, this);
                }
            }
            catch (...)
            {
                // 线程创建失败时回收已创建的线程，避免joinable线程的析构导致terminate
                shutdownAndJoin();
                throw;
            }
        }

        ~GraphicsSceneWorker(void)
        {
            shutdownAndJoin();
        }

        // 发布任务并阻塞等待全部完成。要求没有未完成的任务（fork-join，由调用方保证）
        void dispatch(const std::vector<GraphicsSceneDirtyTask>& tasks)
        {
            assert(tasks.empty() == false);

            {
                std::lock_guard<std::mutex> lock(m_mtx);
                assert(m_remaining == 0);

                m_tasks.clear();
                for (auto& task : tasks)
                {
                    m_tasks.push_back(task.pList);
                }
                m_next = 0;
                m_remaining = m_tasks.size();
            }
            m_cvWork.notify_all();

            std::unique_lock<std::mutex> lock(m_mtx);
            m_cvDone.wait(lock, [this]() { return m_remaining == 0; });
        }

    private:
        // 线程入口：循环领取任务并执行EntityList::generate，无任务时休眠
        void work(void)
        {
            std::unique_lock<std::mutex> lock(m_mtx);

            while (true)
            {
                m_cvWork.wait(lock, [this]() { return m_shutdown || m_next < m_tasks.size(); });
                if (m_shutdown)
                {
                    return;
                }

                while (m_next < m_tasks.size())
                {
                    auto pList = m_tasks[m_next++];
                    lock.unlock();

                    // generate是用户实现的虚函数，异常逃逸出线程函数会导致terminate，
                    // 此处捕获并记为错误，保证m_remaining必然归还
                    try
                    {
                        pList->generate();
                    }
                    catch (...)
                    {
                        BasicLog::out(BasicLog::kError, "Exception thrown in parallel entity generation, discard.");
                    }

                    lock.lock();
                    if (--m_remaining == 0)
                    {
                        m_cvDone.notify_one();
                    }
                }
            }
        }

        void shutdownAndJoin(void)
        {
            {
                std::lock_guard<std::mutex> lock(m_mtx);
                m_shutdown = true;
            }
            m_cvWork.notify_all();

            for (auto& thread : m_threads)
            {
                thread.join();
            }
            m_threads.clear();
        }

        std::vector<std::thread> m_threads;
        std::mutex m_mtx;
        std::condition_variable m_cvWork;      // 主线程 -> worker：有任务可领取
        std::condition_variable m_cvDone;      // worker -> 主线程：全部任务完成
        std::vector<EntityList*> m_tasks;      // 当帧任务
        size_t m_next = 0;                     // 下一个待领取的任务下标（受m_mtx保护）
        size_t m_remaining = 0;                // 未完成的任务数（受m_mtx保护）
        bool m_shutdown = false;               // 受m_mtx保护
    };

    GraphicsScene::GraphicsScene()
    {
        if (m_pEntityManager == nullptr)
        {
            m_pEntityManager = new GraphicsEntityManager(this);
        }
        if (m_pBufferManager == nullptr)
        {
            m_pBufferManager = new GraphicsBufferManager(this);
        }
    }

    GraphicsScene::~GraphicsScene()
    {
        // worker持有访问EntityList的线程，必须先于entityManager销毁（析构内部join）
        if (m_pWorker != nullptr)
        {
            delete m_pWorker;
            m_pWorker = nullptr;
        }

        if (m_pEntityManager != nullptr)
        {
            delete m_pEntityManager;
        }
        if (m_pBufferManager != nullptr)
        {
            delete m_pBufferManager;
        }
        for (auto&& pair : m_printerManager)
        {
            if (pair.second != nullptr)
            {
                pair.second->eraseScene(this);
            }
        }
        if (m_pCamera != nullptr)
        {
            m_pCamera->eraseScene(this);
        }
    }

    bool GraphicsScene::addEntity(GraphicsEntity* pEntity)
    {
        if (pEntity == nullptr)
        {
            BasicLog::out(BasicLog::kWarn, "Trying to add a null pointer as an entity, discard.");
            return false;
        }

        return m_pEntityManager->addEntity(pEntity);
    }

    bool GraphicsScene::removeEntity(GraphicsEntity* pEntity)
    {
        if (pEntity == nullptr)
        {
            BasicLog::out(BasicLog::kWarn, "Trying to remove a null pointer from entities, discard.");
            return false;
        }

        return m_pEntityManager->removeEntity(pEntity);
    }

    bool GraphicsScene::dirtyEntity(GraphicsEntity* pEntity, DirtyType type)
    {
        if (pEntity == nullptr)
        {
            BasicLog::out(BasicLog::kWarn, "Trying to set a null pointer entity dirty, discard.");
            return false;
        }

        return m_pEntityManager->dirtyEntity(pEntity, type);
    }

    bool GraphicsScene::addPrinter(GraphicsPrinter* pPrinter, EntityType type)
    {
        if (pPrinter == nullptr)
        {
            BasicLog::out(BasicLog::kWarn, "Trying to add a null pointer as a printer, discard.");
            return false;
        }

        auto itr = m_printerManager.find(type);
        if (itr != m_printerManager.end())
        {
            if (itr->second == pPrinter)
            {
                return false;
            }

            itr->second->eraseScene(this);
            pPrinter->addScene(this);
            itr->second = pPrinter;
        }
        else
        {
            m_printerManager.insert({ type, pPrinter });
            pPrinter->addScene(this);
        }

        return true;
    }

    bool GraphicsScene::removePrinter(GraphicsPrinter* pPrinter, EntityType type)
    {
        if (pPrinter == nullptr)
        {
            BasicLog::out(BasicLog::kWarn, "Trying to remove a null pointer from printers, discard.");
            return false;
        }

        auto itr = m_printerManager.find(type);
        if (itr != m_printerManager.end() && itr->second == pPrinter)
        {
            pPrinter->eraseScene(this);
            m_printerManager.erase(itr);
            return true;
        }

        return false;
    }

    bool GraphicsScene::removePrinter(GraphicsPrinter* pPrinter)
    {
        if (pPrinter == nullptr)
        {
            BasicLog::out(BasicLog::kWarn, "Trying to remove a null pointer from printers, discard.");
            return false;
        }

        bool success = false;
        for (auto itr = m_printerManager.begin(); itr != m_printerManager.end();)
        {
            if (itr->second == pPrinter)
            {
                pPrinter->eraseScene(this);
                itr = m_printerManager.erase(itr);
                success = true;
            }
            else
            {
                itr++;
            }
        }

        return success;
    }

    void GraphicsScene::bindCamera(GraphicsCamera* pCamera)
    {
        if (pCamera == nullptr)
        {
            BasicLog::out(BasicLog::kWarn, "Trying to bind a null pointer as a camera, discard.");
            return;
        }

        if (m_pCamera != nullptr)
        {
            m_pCamera->eraseScene(this);
        }
        m_pCamera = pCamera;
        pCamera->addScene(this);
    }

    void GraphicsScene::unbindCamera()
    {
        if (m_pCamera != nullptr)
        {
            m_pCamera->eraseScene(this);
            m_pCamera = nullptr;
        }
    }

    void GraphicsScene::draw()
    {
        assert(m_pEntityManager != nullptr);
        assert(m_pBufferManager != nullptr);

        generate();

        auto pWindow = GraphicsWindow::currentWindow();
        if (pWindow == nullptr)
        {
            assert(0);
            BasicLog::out(BasicLog::kWarn, "No window is used, cannot get draw a scene.");
            return;
        }

        if (pWindow->size() == vec2us{0, 0})
        {
            return;
        }

        beforeDraw();

        auto& typeMap = GraphicsEntity::entityTypeMap();
        auto& vendor = pWindow->vendor();

        auto drawImpl = [&typeMap, &vendor, this](unsigned int pipe, unsigned int command) {

            for (auto& pair : m_pEntityManager->m_container)
            {
                auto type = pair.first;
                if (typeMap.count(type) == 0)
                {
                    BasicLog::out(BasicLog::kWarn, "You should register type [", type, "] first before drawing.");
                    continue;
                }
                auto mode = typeMap.find(type)->second;

                auto pPrinter = m_printerManager[type]; // TODO
                if (pPrinter == nullptr || pPrinter->isReady() == false)
                {
                    BasicLog::out(BasicLog::kWarn, "Cannot draw entities of type [", type, "] because printer is not ready.");
                    continue;
                }
                pPrinter->setCompatible(vendor.find("Intel") != std::string::npos);
                pPrinter->use(pipe);

                for (int i = 0; i < pair.second.size(); i++)
                {
                    auto& list = pair.second[i];
                    if (list.entityList.empty())
                    {
                        continue;
                    }
                    assert(list.isDirty() == false);

                    if (isFontType(pair.first))
                    {
                        if (pipe != NormalTransCommand || list.font.valid() == false)
                        {
                            continue;
                        }

                        auto fontInfo = GraphicsFontManager::instance().queryFont(list.font);

                        if (fontInfo.pTexture == nullptr)
                        {
                            continue;
                        }

                        auto pTexture = static_cast<TextureInfo*>(const_cast<GraphicsTexture*>(fontInfo.pTexture)->getOrCreate());
                        assert(pTexture != nullptr);
                        pTexture->bind(TextTextureUnit);
                    }
                    else if (isTextureType(pair.first))
                    {
                        GraphicsTextureManager::instance().bind(list.texture);
                    }

                    auto& buffers = m_pBufferManager->generate(list, type, i);

                    assert(buffers.pDibos.size() == NormalCommandNum && buffers.pDibos[command] != nullptr);
                    auto pDibo = static_cast<DIBOInfo*>(buffers.pDibos[command]->getOrCreate());
                    assert(pDibo != nullptr);

                    if (pDibo->commandNum() == 0)
                    {
                        continue;
                    }

                    pDibo->bind();

                    auto pVao = static_cast<VAOInfo*>(buffers.pVao->getOrCreate());
                    assert(pVao != nullptr);
                    pVao->bind();
                    auto pEbo = static_cast<EBOInfo*>(buffers.pEbo->getOrCreate());
                    assert(pEbo != nullptr);
                    pEbo->bind();
                    auto pSsbo = static_cast<SSBOInfo*>(buffers.pSsbo->getOrCreate());
                    assert(pSsbo != nullptr);
                    pSsbo->bind(NormalProfileSlot);

                    pPrinter->draw(mode, pDibo->commandNum());
                }
            }
        };

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        drawImpl(NormalOpaquePipeline, NormalOpaqueCommand);

        glEnable(GL_BLEND);
        glDepthMask(GL_FALSE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        drawImpl(NormalTransPipeline, NormalTransCommand);

        afterDraw();
    }

    void GraphicsScene::generate()
    {
        assert(m_pEntityManager != nullptr);
        assert(m_pBufferManager != nullptr);

        std::vector<GraphicsSceneDirtyTask> tasks;
        unsigned long long volume = 0;

        // 相位一：收集（主线程）。arrange与clean先行，使脏量估算能读到准确状态。
        // 轻脏列表（无顶点重建工作）当场提交，与原串行路径一致。
        for (auto& pair : m_pEntityManager->m_container)
        {
            auto type = pair.first;

            for (int i = 0; i < pair.second.size(); i++)
            {
                auto& list = pair.second[i];

                list.arrange();

                if (list.entityList.empty() || list.isDirty() == false)
                {
                    continue;
                }

                list.clean();

                if (list.rebuildStart < 0)
                {
                    m_pBufferManager->accept(list, type, i);
                    list.setReady();
                    continue;
                }

                if (list.pointSum.empty())
                {
                    // 冷列表无统计数据，按实体数保守估算
                    volume += static_cast<unsigned long long>(list.entityList.size() - list.invalidNum) * COLD_ENTITY_VOLUME_ESTIMATE;
                }
                else
                {
                    const auto covered = list.pointSum.size() - 1;
                    const auto tail = list.entityList.size() > covered ? list.entityList.size() - covered : 0;

                    volume += list.pointSum.back() - list.pointSum[list.rebuildStart]    // 已提交部分的重算量（精确）
                            + list.indexSum.back() - list.indexSum[list.rebuildStart]
                            + (static_cast<unsigned long long>(list.pointAvg) + list.indexAvg) * tail;    // 未提交尾部估算
                }

                tasks.push_back({ &list, type, i });
            }
        }

        if (tasks.empty())
        {
            return;
        }

        // 相位二：生成。任务数与待重建量均达标时并行（不同列表访问的实体互不相交，无需加锁），
        // 否则主线程串行生成
        const auto workerNum = suggestWorkerNum();
        const bool parallel = tasks.size() >= 2 && volume >= PARALLEL_GENERATE_VOLUME && workerNum >= 2;

        if (parallel)
        {
            if (m_pWorker == nullptr)
            {
                m_pWorker = new GraphicsSceneWorker(workerNum);
            }
            m_pWorker->dispatch(tasks);
        }
        else
        {
            for (auto& task : tasks)
            {
                task.pList->generate();
            }
        }

        // 相位三：提交（主线程）。按收集顺序提交，与串行实现的结果一致
        for (auto& task : tasks)
        {
            m_pBufferManager->accept(*task.pList, task.type, task.index);
            task.pList->setReady();
        }
    }

    void GraphicsScene::beforeDraw()
    {
        clear();
        bindGlobal();
    }

    void GraphicsScene::clear()
    {
        glDepthMask(GL_TRUE);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClearDepth(1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void GraphicsScene::bindGlobal()
    {
        NormalGlobalInfo info;

        if (m_pCamera == nullptr)
        {
            info.vMatrix = info.pMatrix = info.vpMatrix = GraphicsCamera::defaultVPMatrix();
        }
        else
        {
            info.vMatrix = m_pCamera->vMatrix();
            info.pMatrix = m_pCamera->pMatrix();
            info.vpMatrix = m_pCamera->vPMatrix();
        }

        auto pWindow = GraphicsWindow::currentWindow();
        assert(pWindow != nullptr);
        auto size = pWindow->size();
        info.viewport = { size.x, size.y };

        auto pUbo = static_cast<UBOInfo*>(m_globalUbo.getOrCreate());
        assert(pUbo != nullptr);
        pUbo->bind();
        pUbo->setData(sizeof(info), &info);
        pUbo->bind(0);

        glViewport(0, 0, size.x, size.y);
    }

    void GraphicsScene::afterDraw()
    {
        unbind();

#if defined(_DEBUG) || defined(DEBUG)
        assert(glGetError() == 0);
#endif
    }

    void GraphicsScene::unbind()
    {
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
    }

} // namespace FX
