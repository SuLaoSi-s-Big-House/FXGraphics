#include <random>
#include "graphics_window.h"
#include "graphics_entity.h"
#include "graphics_scene.h"
#include "graphics_printer.h"
#include "basic_vector.h"

#include <gp_Circ.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Trsf.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS.hxx>
#include <TopAbs.hxx>
#include <TopLoc_Location.hxx>
#include <BRep_Tool.hxx>
#include <Poly_Triangulation.hxx>
#include <Poly_Triangle.hxx>

class Box : public FX::GraphicsEntity {
public:
    Box(void) : GraphicsEntity(FX::NormalFaceStripID), m_position({ 0.0f, 0.0f, 0.0f, 1.0f }) {}
    explicit Box(float x, float y, float z, float radius) : GraphicsEntity(FX::NormalFaceStripID),
        m_position({ x, y, z, radius }) {}

    ~Box(void) = default;

    void generate(void) override
    {
        double R = std::abs(m_position.w) > 1e-6 ? static_cast<double>(m_position.w) : 2.0;
        double halfWidth = 0.5;
        float cx = m_position.x;
        float cy = m_position.y;
        float cz = m_position.z;

        const int numSections = 120;
        const double pi = std::acos(-1.0);

        // Build Möbius strip by lofting progressively rotating line segments along a circle
        BRepOffsetAPI_ThruSections loftMaker(false, false, false);

        for (int i = 0; i <= numSections; i++)
        {
            double u = 2.0 * pi * i / numSections;
            double halfU = u / 2.0;

            double centerX = R * std::cos(u);
            double centerY = R * std::sin(u);

            // Cross-section direction lies in the (radial, Z) plane, rotated by u/2
            double dirX = std::cos(halfU) * std::cos(u);
            double dirY = std::cos(halfU) * std::sin(u);
            double dirZ = std::sin(halfU);

            gp_Pnt p1(centerX + halfWidth * dirX, centerY + halfWidth * dirY, halfWidth * dirZ);
            gp_Pnt p2(centerX - halfWidth * dirX, centerY - halfWidth * dirY, -halfWidth * dirZ);

            TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(p1, p2).Edge();
            BRepBuilderAPI_MakeWire wireMaker;
            wireMaker.Add(edge);
            loftMaker.AddWire(wireMaker.Wire());
        }

        loftMaker.Build();

        if (!loftMaker.IsDone())
            return;

        TopoDS_Shape mobiusShape = loftMaker.Shape();

        // Tessellate
        BRepMesh_IncrementalMesh mesh(mobiusShape, 0.05);

        // Extract triangle data
        m_vertex.clear();
        m_normal.clear();
        m_uv.clear();
        m_index.clear();

        int vertexCount = 0;

        for (TopExp_Explorer faceExp(mobiusShape, TopAbs_FACE); faceExp.More(); faceExp.Next())
        {
            TopoDS_Face topoFace = TopoDS::Face(faceExp.Current());
            TopLoc_Location loc;
            auto& tri = BRep_Tool::Triangulation(topoFace, loc);

            if (tri.IsNull()) continue;

            gp_Trsf transform = loc.Transformation();
            int nodeCount = tri->NbNodes();

            for (int i = 1; i <= nodeCount; i++)
            {
                gp_Pnt p = tri->Node(i).Transformed(transform);
                m_vertex.push_back(static_cast<float>(p.X()) + cx);
                m_vertex.push_back(static_cast<float>(p.Y()) + cy);
                m_vertex.push_back(static_cast<float>(p.Z()) + cz);
            }

            if (tri->HasNormals())
            {
                for (int i = 1; i <= nodeCount; i++)
                {
                    gp_Dir n = tri->Normal(i);
                    n.Transform(transform);
                    m_normal.push_back(static_cast<float>(n.X()));
                    m_normal.push_back(static_cast<float>(n.Y()));
                    m_normal.push_back(static_cast<float>(n.Z()));
                }
            }
            else
            {
                for (int i = 1; i <= nodeCount; i++)
                {
                    m_normal.push_back(0.0f);
                    m_normal.push_back(0.0f);
                    m_normal.push_back(1.0f);
                }
            }

            if (tri->HasUVNodes())
            {
                for (int i = 1; i <= nodeCount; i++)
                {
                    gp_Pnt2d uv = tri->UVNode(i);
                    m_uv.push_back(static_cast<float>(uv.X()));
                    m_uv.push_back(static_cast<float>(uv.Y()));
                    m_uv.push_back(0.0f);
                }
            }
            else
            {
                for (int i = 1; i <= nodeCount; i++)
                {
                    m_uv.push_back(0.0f);
                    m_uv.push_back(0.0f);
                    m_uv.push_back(0.0f);
                }
            }

            int triCount = tri->NbTriangles();
            for (int i = 1; i <= triCount; i++)
            {
                const Poly_Triangle& triangle = tri->Triangle(i);
                int n1, n2, n3;
                triangle.Get(n1, n2, n3);
                m_index.push_back(vertexCount + n1 - 1);
                m_index.push_back(vertexCount + n2 - 1);
                m_index.push_back(vertexCount + n3 - 1);
                m_index.push_back(FX::RestartMark);
            }

            vertexCount += nodeCount;
        }
    }

private:
    FX::vec4f m_position;
};


std::random_device rDevice;
std::mt19937 rEngine(rDevice());
std::uniform_int_distribution<> range(0, 7);

int main(void)
{
    FX::GraphicsWindow window1(800, 600);
    window1.use();
    window1.frame();

    FX::GraphicsWindow window2(800, 600);
    window2.use();
    window2.frame();

    Box* boxs[8] = {};
    for (int i = 0; i < 8; i++)
    {
        boxs[i] = new Box(5 * std::sin(2 * 3.1415926f * i / 8), 5 * std::cos(2 * 3.1415926f * i / 8), 0.0f, 1.0f);
    }

    FX::GraphicsNormalPrinter printer;
    std::ifstream ifs;
    ifs.open("./shader/normal_world.vert");
    printer.addShader(FX::GPUItemType::kVtxShader, ifs);
    ifs.close();
    ifs.open("./shader/normal_world.frag");
    printer.addShader(FX::GPUItemType::kFrgShader, ifs);
    ifs.close();

    FX::GraphicsScene scene;
    scene.addPrinter(&printer, FX::NormalFaceStripID);
    for (int i = 0; i < 8; i++)
    {
        scene.addEntity(boxs[i]);
    }

    int i = 0;
    int n = 0;
    while (!window1.shouldClose() && !window2.shouldClose())
    {
        if (n % 10 == 0)
        {
            scene.addEntity(boxs[i]);
            i = range(rEngine);
            scene.removeEntity(boxs[i]);
        }

        window1.use();
        scene.draw();
        window1.frame();

        if (n % 60 == 0)
        {
            window2.use();
            scene.draw();
            window2.frame();
        }

        n++;
    }

    for (int i = 0; i < 8; i++)
    {
        delete boxs[i];
    }
}
