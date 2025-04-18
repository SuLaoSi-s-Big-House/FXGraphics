#include <fstream>
#include <thread>
#include <future>
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "basic_log.h"
#include "graphics_window.h"
#include "graphics_printer.h"

void loadShader(FX::GraphicsNormalPrinter& facePrinter, FX::GraphicsNormalPrinter& linePrinter)
{
    std::ifstream ifs;
    ifs.open("./shader/normal.vert");
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    ifs.close();
    facePrinter.addShader(FX::GPUItemType::kVtxShader, buffer.str());
    linePrinter.addShader(FX::GPUItemType::kVtxShader, buffer.str());
    ifs.open("./shader/phong.frag");
    facePrinter.addShader(FX::GPUItemType::kFrgShader, ifs);
    ifs.close();
    ifs.open("./shader/normal.frag");
    linePrinter.addShader(FX::GPUItemType::kFrgShader, ifs);
    ifs.close();
    FX::BasicLog::out(FX::BasicLog::kInfo, "shader loaded.");
}

void loadModel(const char* path, std::vector<float>& vertex, std::vector<float>& normal, glm::vec3& maxPos, glm::vec3& minPos)
{
    tinyobj::ObjReader reader;
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = "D:/";

    reader.ParseFromFile(path, reader_config);

    if (!reader.Error().empty())
    {
        FX::BasicLog::out(FX::BasicLog::kError, reader.Error());
    }

    if (!reader.Warning().empty())
    {
        FX::BasicLog::out(FX::BasicLog::kWarn, reader.Warning());
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    //auto& materials = reader.GetMaterials();

    vertex.reserve(1000);
    normal.reserve(1000);

    bool hasQuads = false;
    for (int i = 0; i < shapes.size(); i++)
    {
        int offset = 0;
        for (int j = 0; j < shapes[i].mesh.num_face_vertices.size(); j++)
        {
            if (shapes[i].mesh.num_face_vertices[j] == 3)
            {
                for (int k = 0; k < 3; k++)
                {
                    auto& idx = shapes[i].mesh.indices[offset + k];

                    vertex.push_back(attrib.vertices[3 * idx.vertex_index]);
                    vertex.push_back(attrib.vertices[3 * idx.vertex_index + 2]);
                    vertex.push_back(attrib.vertices[3 * idx.vertex_index + 1]);

                    maxPos.x = std::max(maxPos.x, vertex[vertex.size() - 3]);
                    maxPos.y = std::max(maxPos.y, vertex[vertex.size() - 2]);
                    maxPos.z = std::max(maxPos.z, vertex[vertex.size() - 1]);
                    minPos.x = std::min(minPos.x, vertex[vertex.size() - 3]);
                    minPos.y = std::min(minPos.y, vertex[vertex.size() - 2]);
                    minPos.z = std::min(minPos.z, vertex[vertex.size() - 1]);
                }

                auto norm = glm::normalize(glm::cross(
                    glm::vec3(vertex[vertex.size() - 9] - vertex[vertex.size() - 6],
                        vertex[vertex.size() - 8] - vertex[vertex.size() - 5],
                        vertex[vertex.size() - 7] - vertex[vertex.size() - 4]),
                    glm::vec3(vertex[vertex.size() - 3] - vertex[vertex.size() - 6],
                        vertex[vertex.size() - 2] - vertex[vertex.size() - 5],
                        vertex[vertex.size() - 1] - vertex[vertex.size() - 4])));

                normal.push_back(norm.x);
                normal.push_back(norm.y);
                normal.push_back(norm.z);
                normal.push_back(norm.x);
                normal.push_back(norm.y);
                normal.push_back(norm.z);
                normal.push_back(norm.x);
                normal.push_back(norm.y);
                normal.push_back(norm.z);
            }
            else
            {
                hasQuads = true;
            }

            offset += shapes[i].mesh.num_face_vertices[j];
        }
    }

    if (hasQuads)
    {
        FX::BasicLog::out(FX::BasicLog::kError, "There are quads in model.");
    }

    FX::BasicLog::out(FX::BasicLog::kInfo, "model loaded.");
}

int main(void)
{
    std::vector<float> vertex;
    std::vector<float> normal;
    glm::vec3 maxPos = glm::vec3(-1e10f);
    glm::vec3 minPos = glm::vec3(1e10f);

    auto modelBarrier = std::async(std::launch::async, loadModel, "D:/helicopter.obj", std::ref(vertex), std::ref(normal),
        std::ref(maxPos), std::ref(minPos));

    FX::GraphicsNormalPrinter printer1(FX::PrintType::kLines);
    FX::GraphicsNormalPrinter printer2(FX::PrintType::kLines);

    auto shaderBarrier = std::async(std::launch::async, loadShader, std::ref(printer1), std::ref(printer2));

    FX::GraphicsWindow window(800, 600, "FXGraphics", false);
    window.use();

    gladLoadGL();

    glClearColor(0.95f, 0.95f, 0.95f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    window.frame();

    unsigned int VAO, VBO1, VBO2, EBO, UBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    modelBarrier.get();
    FX::BasicLog::out(FX::BasicLog::kInfo, "model extent (", minPos.x, ", ", minPos.y, ", ", minPos.z, ") - (", maxPos.x, ", ", maxPos.y, ", ", maxPos.z, ")");

    glGenBuffers(1, &VBO1);
    glBindBuffer(GL_ARRAY_BUFFER, VBO1);
    glBufferData(GL_ARRAY_BUFFER, vertex.size() * sizeof(float), vertex.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &VBO2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, normal.size() * sizeof(float), normal.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &EBO);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, index.size() * sizeof(unsigned int), index.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &UBO);

    shaderBarrier.get();
    printer1.use();
    printer2.use();

    auto handle = printer1.m_programs[0]->getOrCreate()->m_handle;
    auto ii = glGetUniformBlockIndex(handle, "globalConfig");
    glUniformBlockBinding(handle, ii, 0);
    handle = printer2.m_programs[0]->getOrCreate()->m_handle;
    ii = glGetUniformBlockIndex(handle, "globalConfig");
    glUniformBlockBinding(handle, ii, 0);

    struct GlobalConfig {
        glm::mat4 mvpMatrix;
        glm::mat4 mvMatrix;
        glm::vec4 color;
        glm::vec4 lightPos;
    };

    GlobalConfig config;

    float fov = 45.0f;
    glm::vec3 cameraPos = glm::vec3(400.0f, 0.0f, 0.0f);
    glm::mat4 project = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    float scale = 100 / std::max(std::max(std::max(maxPos.x, maxPos.y), maxPos.z), std::max(std::max(-minPos.x, -minPos.y), -minPos.z));
    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(scale));

    int i = 0;
    while (!window.shouldClose())
    {
        auto flag = window.interator().flag();
        if (flag & FX::GraphicsInteractor::kMouseScroll)
        {
            auto value = window.interator().mouseScroll();
            fov -= 3 * value;
            fov = std::min(std::max(fov, 1.0f), 90.0f);
        }

        if (flag & FX::GraphicsInteractor::kMouseMove)
        {
            auto& pos = window.interator().mousePos();
            cameraPos.x = std::sin(glm::radians(pos.x - 400));
            cameraPos.y = std::cos(glm::radians(pos.x - 400));
            cameraPos.z = (pos.y - 300) / 100;
            cameraPos = 400.0f * glm::normalize(cameraPos);
        }

        project = glm::perspective(glm::radians(fov), 4 / 3.0f, 10.0f, 1000.0f);
        view = glm::lookAt(cameraPos, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        config.mvpMatrix = project * view * model;
        config.mvMatrix = view * model;
        config.color = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
        config.lightPos = view * glm::vec4(cameraPos - 100.0f * glm::normalize(glm::cross(cameraPos, glm::vec3(0.0f, 0.0f, 1.0f))) + glm::vec3(0.0f, 0.0f, 100.0f), 1.0f);

        glBindBuffer(GL_UNIFORM_BUFFER, UBO);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(config), &config, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO);

        glDepthMask(GL_TRUE);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(VAO);
        //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        //glCullFace(GL_BACK);
        glDisable(GL_BLEND);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        //glDisable(GL_POLYGON_OFFSET_LINE);
        //glEnable(GL_POLYGON_OFFSET_FILL);
        //glPolygonOffset(-1.0f, 10.0f);

        printer1.use();
        glDrawArrays(GL_TRIANGLES, 0, vertex.size());
        //glDrawElements(GL_TRIANGLES, index.size(), GL_UNSIGNED_INT, 0);

        config.color = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
        glBindBuffer(GL_UNIFORM_BUFFER, UBO);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(config), &config, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO);

        //glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //glEnable(GL_LINE_SMOOTH);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glEnable(GL_CULL_FACE);
        //glCullFace(GL_FRONT);
        glLineWidth(2.0f);
        //glDisable(GL_POLYGON_OFFSET_FILL);
        //glEnable(GL_POLYGON_OFFSET_LINE);
        //glPolygonOffset(1.0f, 10.0f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        printer2.use();
        //glDrawArrays(GL_TRIANGLES, 0, vertex.size());
        //glDrawElements(GL_LINE_STRIP, index.size(), GL_UNSIGNED_INT, 0);

        window.frame();
        i++;
    }

    glDeleteBuffers(1, &VBO1);
    glDeleteBuffers(1, &VBO2);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &UBO);
    glDeleteVertexArrays(1, &VAO);
}
