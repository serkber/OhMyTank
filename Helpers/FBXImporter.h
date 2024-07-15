#pragma once
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <xstring>
#include "Typedefs.h"
#include "Utils.h"

class FBXImporter
{    
public:
    struct Vertex
    {
        float3 position;
        float3 normal;
        float2 uv;
    };
    struct FBXModel
    {
        Vertex* vertices;
        int* indices{nullptr};
        uint32_t vertexCount;
        uint32_t indexCount;

        FBXModel& operator=(const FBXModel& other) {
            if (this == &other) {
                return *this;
            }

            delete[] vertices;
            delete[] indices;

            vertexCount = other.vertexCount;
            indexCount = other.indexCount;

            vertices = Utils::CopyArray<Vertex>(other.vertices, other.vertexCount);
            indices = Utils::CopyArray<int>(other.indices, other.indexCount);

            return *this;
        }

        ~FBXModel()
        {
            delete[] vertices;
            delete[] indices;
        }
    };

    ~FBXImporter();
    FBXModel* LoadModel(std::string file);

    FBXModel m_model;

private:
    
    enum NormalsMode
    {
        Undefined,
        ByVertex,
        ByPolygonVertex
    };

    void EvaluateWord(const std::string& text);
    std::vector<std::string> GetNextRawData(uint32_t& numElements);
    void DecodeMesh();
    void UpdateReplacements(std::vector<std::pair<int, int>>* replacements, int oldIndex, int newIndex);
    void ReplaceInIndices(std::vector<int>* triIndices, int oldIndex, int newIndex);
    static bool CompareFloat3(float3 one, float3 two);
    static bool CompareFloat2(float2 one, float2 two);
    
    std::ifstream m_file;
    int m_vertexCount;
    uint32_t m_indicesCount;
    int* m_posIndices;
    int* m_uvIndices;
    float3* m_normals;
    float2* m_uvs;
    float3* m_positions;
    NormalsMode m_normals_mode = Undefined;
};
