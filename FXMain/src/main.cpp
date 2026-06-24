#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include "glad.h"
#include "glm.hpp"
#include "basic_bounding.h"
#include "graphics_window.h"
#include "graphics_scene.h"
#include "graphics_printer.h"
#include "graphics_camera.h"
#include "graphics_font_manager.h"
#include "surf_text_item.h"
#include "logic_camera.h"
#include "compass.h"

using namespace FX;

std::unique_ptr<GraphicsWindow> pMainWindow;
std::unique_ptr<GraphicsScene> pMainScene;
std::unique_ptr<CompassScene> pCompassScene;
std::unordered_map<std::string, std::unique_ptr<GraphicsPrinter>> printerList;
std::vector<std::unique_ptr<GraphicsEntity>> mainEntityList;
std::vector<std::unique_ptr<GraphicsEntity>> compassEntityList;
std::unique_ptr<LogicObserveCamera> pMainCamera;
std::unique_ptr<GraphicsCamera> pCompassCamera;
Font compassFont;
Font mainFont;
long long sumT = 0;
int nFrame = 0;
int fps = 0;

namespace {
    std::string readOneFile(const std::string& path)
    {
        std::string ret;
        std::ifstream ifs;
        ifs.open(path);
        if (ifs.is_open() == false)
        {
            return ret;
        }

        std::stringstream buffer;
        buffer << ifs.rdbuf();
        auto source = buffer.str();
        if (source.empty())
        {
            return ret;
        }

        int i = 0;
        for (; i < source.length(); i++)
        {
            if (std::isprint(static_cast<unsigned char>(source[i])))
            {
                break;
            }
        }

        if (i < source.length())
        {
            ret = source.substr(i);
        }
        return ret;
    }

    bool checkFileAvailable(void)
    {
        return true;
    }

    bool buildScene(void)
    {
        auto vert = readOneFile("./shader/normal_world.vert");
        auto frag = readOneFile("./shader/normal_world.frag");
        printerList["world/pure"].reset(new GraphicsNormalPrinter);
        printerList["world/pure"]->addShader(GPUItemType::kVtxShader, vert);
        printerList["world/pure"]->addShader(GPUItemType::kFrgShader, frag);
        frag = readOneFile("./shader/normal_world_phong.frag");
        printerList["world/phong"].reset(new GraphicsNormalPrinter);
        printerList["world/phong"]->addShader(GPUItemType::kVtxShader, vert);
        printerList["world/phong"]->addShader(GPUItemType::kFrgShader, frag);
        vert = readOneFile("./shader/normal_screen_text.vert");
        frag = readOneFile("./shader/normal_screen_text.frag");
        printerList["screen/text"].reset(new GraphicsNormalPrinter);
        printerList["screen/text"]->addShader(GPUItemType::kVtxShader, vert);
        printerList["screen/text"]->addShader(GPUItemType::kFrgShader, frag);

        pCompassScene->addPrinter(printerList["world/pure"].get(), NormalFaceStripID);
        pCompassScene->addPrinter(printerList["world/pure"].get(), NormalLineStripID);
        pCompassScene->addPrinter(printerList["world/pure"].get(), NormalFaceID);
        pCompassScene->addPrinter(printerList["world/pure"].get(), NormalLineID);
        pCompassScene->addPrinter(printerList["screen/text"].get(), ScreenTextID);
        pMainScene->addPrinter(printerList["screen/text"].get(), ScreenTextID);

        pMainCamera.reset(new LogicObserveCamera(pMainWindow.get()));
        pMainScene->bindCamera(&pMainCamera->get());

        pCompassCamera.reset(new GraphicsCamera);
        pCompassScene->bindCamera(pCompassCamera.get());
        pCompassCamera->setField(-2.5f, 2.5f, -2.5f, 2.5f);
        pCompassCamera->setNearFar(0, 10);
        return true;
    }

    bool addCompassEntity(void)
    {
        compassEntityList.emplace_back(new CompassPlane);
        compassEntityList.emplace_back(new CompassPlaneEdge);
        compassEntityList.emplace_back(new CompassArrow(glm::mat4(1.0f)));
        compassEntityList.emplace_back(new CompassArrow(glm::rotate(glm::mat4(1.0f), static_cast<float>(Math::PI / 2), glm::vec3(0, 0, 1))));
        compassEntityList.emplace_back(new CompassArrow(glm::rotate(glm::mat4(1.0f), static_cast<float>(Math::PI / 2), glm::vec3(0, -1, 0))));
        compassEntityList.emplace_back(new CompassArrow(glm::rotate(glm::mat4(1.0f), static_cast<float>(-Math::PI / 2), glm::vec3(0, 0, 1))));
        compassEntityList.emplace_back(new CompassArrow(glm::rotate(glm::mat4(1.0f), static_cast<float>(-Math::PI / 2), glm::vec3(0, -1, 0))));
        compassEntityList.emplace_back(new CompassArrow(glm::rotate(glm::mat4(1.0f), static_cast<float>(Math::PI), glm::vec3(0, 1, 0))));
        compassEntityList.emplace_back(new CompassArrowLine(glm::mat4(1.0f)));
        compassEntityList.emplace_back(new CompassArrowLine(glm::rotate(glm::mat4(1.0f), static_cast<float>(Math::PI / 2), glm::vec3(0, 0, 1))));
        compassEntityList.emplace_back(new CompassArrowLine(glm::rotate(glm::mat4(1.0f), static_cast<float>(Math::PI / 2), glm::vec3(0, -1, 0))));

        auto name = GraphicsFontManager::instance().loadFontFile("./font/BlueakaBeta-DB-GBK.ttf");
        compassFont = { name, 16 };
        compassEntityList.emplace_back(new SurfTextEntity(compassFont, "前", { 0, 0 }));
        compassEntityList.emplace_back(new SurfTextEntity(compassFont, "上", { 0, 0 }));
        compassEntityList.emplace_back(new SurfTextEntity(compassFont, "右", { 0, 0 }));

        for (auto& ptr : compassEntityList)
        {
            pCompassScene->addEntity(ptr.get());
        }
        return true;
    }

    bool addMainEntity(void)
    {
        auto pNewText = new SurfTextEntity(compassFont, "鼠标滚轮缩放，中键平移，右键旋转", { 1280 - 270, 720 - 40 });
        auto profile = pNewText->profile();
        profile.color = { 200, 100, 0, 255 };
        pNewText->setProfile(profile);
        mainEntityList.emplace_back(pNewText);
        pNewText = new SurfTextEntity(compassFont, "目前平移和旋转的手感比较怪，属正常现象", { 1280 - 245, 720 - 20 });
        profile.font.size = 12;
        profile.color = { 200, 200, 200, 255 };
        pNewText->setProfile(profile);
        mainEntityList.emplace_back(pNewText);

        auto name = GraphicsFontManager::instance().loadFontFile("./font/HarmonyOS_Sans_SC_Regular.ttf");
        mainFont = { name, 16 };

        pNewText = new SurfTextEntity(mainFont, "", { 5, 5 });
        profile = pNewText->profile();
        profile.color = { 200, 200, 200, 255 };
        pNewText->setProfile(profile);
        mainEntityList.emplace_back(pNewText);
        pNewText = new SurfTextEntity(mainFont, "", { 5, 720 - 70 });
        pNewText->setProfile(profile);
        mainEntityList.emplace_back(pNewText);

        for (auto& ptr : mainEntityList)
        {
            pMainScene->addEntity(ptr.get());
        }
        return true;
    }

    bool init(void)
    {
        if (checkFileAvailable() == false)
        {
            return false;
        }
        pMainWindow.reset(new GraphicsWindow(1280, 720));
        gladLoadGL();
        pMainScene.reset(new GraphicsScene);
        pCompassScene.reset(new CompassScene);
        if (buildScene() == false)
        {
            return false;
        }
        if (addCompassEntity() == false)
        {
            return false;
        }
        if (addMainEntity() == false)
        {
            return false;
        }
        return true;
    }

    void uninit(void)
    {
        pCompassCamera.reset(nullptr);
        pMainCamera.reset(nullptr);
        pCompassScene.reset(nullptr);
        pMainScene.reset(nullptr);
        pMainWindow.reset(nullptr);
        for (auto& pair : printerList)
        {
            pair.second.reset(nullptr);
        }
        for (auto& ptr : compassEntityList)
        {
            ptr.reset(nullptr);
        }
        for (auto& ptr : mainEntityList)
        {
            ptr.reset(nullptr);
        }
    }

    void updateCompassText(void)
    {
        auto n = compassEntityList.size();
        auto& vpMatrix = pCompassCamera->vPMatrix();
        glm::vec4 clip = vpMatrix * glm::vec4(1.9f, 0, 0, 1);
        glm::vec3 ndc = glm::vec3(clip) / clip.w;
        int x = static_cast<int>((ndc.x + 1) / 2 * COMPASS_SIZE);
        int y = static_cast<int>((1 - ndc.y) / 2 * COMPASS_SIZE);
        auto box = GraphicsFontManager::instance().queryText(compassFont, static_cast<SurfTextEntity*>(compassEntityList[n - 3].get())->text()).textBox;
        vec2i center = { (box.x + box.y) / 2, (box.z + box.w) / 2 };
        center = { x - center.x, y - center.y };
        static_cast<SurfTextEntity*>(compassEntityList[n - 3].get())->setPosition(center);
        static_cast<SurfTextEntity*>(compassEntityList[n - 3].get())->setDepth(ndc.z);
        clip = vpMatrix * glm::vec4(0, 1.9f, 0, 1);
        ndc = glm::vec3(clip) / clip.w;
        x = static_cast<int>((ndc.x + 1) / 2 * COMPASS_SIZE);
        y = static_cast<int>((1 - ndc.y) / 2 * COMPASS_SIZE);
        box = GraphicsFontManager::instance().queryText(compassFont, static_cast<SurfTextEntity*>(compassEntityList[n - 2].get())->text()).textBox;
        center = { (box.x + box.y) / 2, (box.z + box.w) / 2 };
        center = { x - center.x, y - center.y };
        static_cast<SurfTextEntity*>(compassEntityList[n - 2].get())->setPosition(center);
        static_cast<SurfTextEntity*>(compassEntityList[n - 2].get())->setDepth(ndc.z);
        clip = vpMatrix * glm::vec4(0, 0, 1.9f, 1);
        ndc = glm::vec3(clip) / clip.w;
        x = static_cast<int>((ndc.x + 1) / 2 * COMPASS_SIZE);
        y = static_cast<int>((1 - ndc.y) / 2 * COMPASS_SIZE);
        box = GraphicsFontManager::instance().queryText(compassFont, static_cast<SurfTextEntity*>(compassEntityList[n - 1].get())->text()).textBox;
        center = { (box.x + box.y) / 2, (box.z + box.w) / 2 };
        center = { x - center.x, y - center.y };
        static_cast<SurfTextEntity*>(compassEntityList[n - 1].get())->setPosition(center);
        static_cast<SurfTextEntity*>(compassEntityList[n - 1].get())->setDepth(ndc.z);
    }

    void updateMainText(void)
    {
        auto& interactor = pMainWindow->interactor();
        auto size = pMainWindow->size();
        auto flag = interactor.enventFlag();
        if (flag & WindowResizeFlag)
        {
            static_cast<SurfTextEntity*>(mainEntityList[0].get())->setPosition({ size.x - 270, size.y - 40 });
            static_cast<SurfTextEntity*>(mainEntityList[1].get())->setPosition({ size.x - 245, size.y - 20 });
            static_cast<SurfTextEntity*>(mainEntityList[3].get())->setPosition({ 5, size.y - 70 });
        }

        std::string str;
        if (sumT >= 5e5)
        {
            fps = static_cast<int>(1e6f * nFrame / sumT + 0.5f);
        }
        str = "FPS: " + std::to_string(fps) + "\n";
        str += "窗口大小: [" + std::to_string(size.x) + ", " + std::to_string(size.y) + "]\n";
        if (interactor.isCursorIn())
        {
            auto pos = interactor.cursorPos();
            str += "鼠标位置: (" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ")\n";
        }
        else
        {
            str += "\n";
        }
        if (flag & FX::MouseDragFlag)
        {
            str += "正在拖拽\n";
        }
        static_cast<SurfTextEntity*>(mainEntityList[2].get())->setText(str);

        auto& cam = pMainCamera->get();
        auto& position = cam.position();
        auto& lookAt = cam.lookAt();
        glm::vec3 dir = glm::vec3(position.x - lookAt.x, position.y - lookAt.y, position.z - lookAt.z);
        auto dis = glm::length(dir);
        dir = glm::normalize(dir);

        str = "观察中心: (" + std::to_string(lookAt.x) + ", " + std::to_string(lookAt.y) + ", " + std::to_string(lookAt.z) + ")\n";
        str += "相机方向: (" + std::to_string(dir.x) + ", " + std::to_string(dir.y) + ", " + std::to_string(dir.z) + ")\n";
        str += "相机距离: " + std::to_string(dis) + "\n";
        str += "缩放比例: " + std::to_string(pMainCamera->scale());
        static_cast<SurfTextEntity*>(mainEntityList[3].get())->setText(str);
    }
}

int main(void)
{
    if (init() == false)
    {
        uninit();
        return 1;
    }

    BasicBounding<> bounding;
    bounding.expand({ 10, 10, 10 });
    bounding.expand({ -10, -10, -10 });
    pMainCamera->observe(bounding);

    pMainWindow->use();
    auto last = std::chrono::high_resolution_clock::now();
    while (!pMainWindow->shouldClose())
    {
        auto now = std::chrono::high_resolution_clock::now();
        auto dur = std::chrono::duration_cast<std::chrono::microseconds>(now - last).count();
        last = now;
        sumT += dur;

        pMainCamera->process();
        syncCamera(pMainCamera.get(), pCompassCamera.get());
        updateCompassText();
        updateMainText();

        pMainScene->draw();
        pCompassScene->draw();
        pMainWindow->frame();

        if (sumT >= 5e5)
        {
            sumT = nFrame = 0;
        }
        nFrame++;
    }

    uninit();
}
