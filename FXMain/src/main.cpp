#include "graphics_window.h"
#include "graphics_printer.h"
#include <fstream>
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "basic_log.h"

int main(void)
{
    FX::GraphicsNormalPrinter printer(FX::PrintType::kLines);
    std::ifstream ifs;
    ifs.open("./shader/normal.vert");
    printer.addShader(FX::GPUItemType::kVtxShader, ifs);
    ifs.close();
    ifs.open("./shader/normal.frag");
    printer.addShader(FX::GPUItemType::kFrgShader, ifs);
    ifs.close();

    std::vector<float> vertex;
    std::vector<float> normal;
    //std::vector<unsigned int> index;

    //vertex = {
    //    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f,
    //    -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f,
    //};

    //normal = {
    //    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f,
    //    -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f,
    //};

    //index = {
    //    0, 3, 1, 0, 2, 3, 0, 4, 2, 4, 6, 2, 1, 5, 4, 1, 4, 0, 4, 5, 7, 4, 7, 6, 1, 3, 7, 1, 7, 5, 2, 6, 7, 2, 7, 3
    //};

    tinyobj::ObjReader reader;
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = "D:/";

    reader.ParseFromFile("D:/helicopter.obj", reader_config);

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
                    vertex.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
                    vertex.push_back(attrib.vertices[3 * idx.vertex_index + 2]);

                    if (idx.normal_index >= 0)
                    {
                        normal.push_back(attrib.normals[3 * idx.normal_index]);
                        normal.push_back(attrib.normals[3 * idx.normal_index] + 1);
                        normal.push_back(attrib.normals[3 * idx.normal_index] + 2);
                    }
                }
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

    FX::GraphicsWindow window(800, 600, "FXGraphics", false);
    window.use();

    gladLoadGL();

    glClearColor(0.95f, 0.95f, 0.95f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    window.frame();

    printer.use();

    unsigned int VAO, VBO1, VBO2, EBO, UBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

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

    auto handle = printer.m_programs[0]->getOrCreate()->m_handle;
    auto ii = glGetUniformBlockIndex(handle, "globalConfig");
    glUniformBlockBinding(handle, ii, 0);

    glGenBuffers(1, &UBO);

    glm::mat4 project = glm::perspective(30.0f, 4 / 3.0f, 0.1f, 100.0f);

    int i = 0;
    while (!window.shouldClose())
    {
        struct GlobalConfig {
            glm::mat4 vpMatrix;
            glm::vec4 color;
        };

        GlobalConfig config;

        config.vpMatrix = glm::lookAt(glm::vec3(3.0f * std::sin(0.005f * i), 3.0f * std::cos(0.005f * i), 3.0 * std::sin(0.005f * i)), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        config.vpMatrix = project * config.vpMatrix;
        config.color = glm::vec4(0.2, 0.5, 0.8, 1.0);

        glBindBuffer(GL_UNIFORM_BUFFER, UBO);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(config), &config, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO);

        glDepthMask(GL_TRUE);
        glClearColor(0.95f, 0.95f, 0.95f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(VAO);
        //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        //glCullFace(GL_BACK);
        glDisable(GL_BLEND);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_POLYGON_OFFSET_LINE);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(-1.0f, 3.0f);

        glDrawArrays(GL_TRIANGLES, 0, vertex.size());
        //glDrawElements(GL_TRIANGLES, index.size(), GL_UNSIGNED_INT, 0);

        config.color = glm::vec4(1.0, 0.5, 0.0, 1.0);
        glBindBuffer(GL_UNIFORM_BUFFER, UBO);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(config), &config, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO);

        //glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glDepthMask(GL_FALSE);
        //glEnable(GL_LINE_SMOOTH);
        glLineWidth(5.0f);
        glDisable(GL_POLYGON_OFFSET_FILL);
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(1.0f, 3.0f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glDrawArrays(GL_TRIANGLES, 0, vertex.size());
        //glDrawElements(GL_LINE_STRIP, index.size(), GL_UNSIGNED_INT, 0);

        window.frame();
        i++;
    }

    glDeleteBuffers(1, &VBO1);
    glDeleteBuffers(1, &VBO2);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}
