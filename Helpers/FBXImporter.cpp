#include "FBXImporter.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include "Typedefs.h"
#include "Utils.h"
#include <numeric>

FBXImporter::~FBXImporter()
{
    delete[] m_uvIndices;
    delete[] m_positions;
    delete[] m_normals;
    delete[] m_posIndices;
    delete[] m_uvs;
}

void FBXImporter::EvaluateWord(const std::string& text)
{    
    if(text._Equal("Vertices:"))
    {
        uint32_t numElements;
        const auto vertices = GetNextRawData(numElements);
        m_positions = new float3[numElements / 3];
        for (int i = 0, j = 0; i < numElements / 3; ++i, j += 3)
        {
            m_positions[i].x = std::stof(vertices[j]);
            m_positions[i].y = std::stof(vertices[j + 1]);
            m_positions[i].z = std::stof(vertices[j + 2]);
        }
    }

    if (text._Equal("PolygonVertexIndex:"))
    {
        uint32_t posIndicesCount;
        const auto inds = GetNextRawData(posIndicesCount);
        m_posIndices = new int[posIndicesCount];

        for (int i = 0; i < posIndicesCount; ++i)
        {
            auto index = std::stoi(inds[i]);    
            if(index < 0)
            {
                index = abs(index) - 1;
            }            
            m_posIndices[i] = index;
        }
    }

    if(text._Equal("MappingInformationType:") && m_normals_mode == Undefined)
    {
        std::string nextWord;
        m_file >> nextWord;
        if(nextWord._Equal(R"("ByVertice")"))
        {
            m_normals_mode = ByVertex;
        }
    
        if(nextWord._Equal(R"("ByPolygonVertex")"))
        {
            m_normals_mode = ByPolygonVertex;
        };
    }

    if (text._Equal("Normals:"))
    {
        uint32_t numNormals;
        const auto norms = GetNextRawData(numNormals);
        m_normals = new float3[numNormals / 3];
        for (int i = 0, j = 0; i < numNormals / 3; ++i, j += 3)
        {
            m_normals[i].x = std::stof(norms[j]);
            m_normals[i].y = std::stof(norms[j + 1]);
            m_normals[i].z = std::stof(norms[j + 2]);
        }
    }
    
    if (text._Equal("UV:"))
    {
        uint32_t numUvs;
        const auto uvs = GetNextRawData(numUvs);
        m_uvs = new float2[numUvs / 2];
        for (int i = 0, j = 0; i < numUvs / 2; ++i, j += 2)
        {            
            m_uvs[i].x = std::stof(uvs[j]);
            m_uvs[i].y = std::stof(uvs[j + 1]);
        }
    }
    
    if (text._Equal("UVIndex:"))
    {
        const auto uvsIndices = GetNextRawData(m_indicesCount);
        m_uvIndices = new int[m_indicesCount];
        for (int i = 0; i < m_indicesCount; ++i)
        {
            m_uvIndices[i] = std::stof(uvsIndices[i]);
        }
    }
}

std::vector<std::string> FBXImporter::GetNextRawData(uint32_t& numElements)
{
    std::string numElementsString;
    m_file >> numElementsString;
    numElementsString.erase(0, 1);
    numElements = std::stoi(numElementsString);
    
    std::string elementsBuffer;
    std::string elementsRaw;
    while (!m_file.eof())
    {
        m_file >> elementsBuffer;

        if(elementsBuffer._Equal("}"))
        {
            break;
        }

        if(elementsBuffer._Equal("a:") || elementsBuffer._Equal("{"))
        {
            continue;
        }

        elementsRaw = elementsRaw + elementsBuffer;
    }

    return Utils::Split(elementsRaw, ',');
}

void FBXImporter::DecodeMesh()
{
    std::vector<float3> allPositions(m_indicesCount);
    std::vector<float3> allNormals(m_indicesCount);
    std::vector<float2> allUvs(m_indicesCount);

    Utils::StartTimeMeasure();
    for (int i = 0; i < m_indicesCount; ++i)
    {
        // std::cout << m_positions[m_posIndices[i]].x << ' ' << m_positions[m_posIndices[i]].y << "      " << m_uvs[m_uvIndices[i]].x << ' ' << m_uvs[m_uvIndices[i]].y << std::endl;
        // std::cout << m_positions[m_posIndices[i]].x << ' ' << m_positions[m_posIndices[i]].y << ' ' << m_positions[m_posIndices[i]].z << "      " << m_normals[i].x << ' ' << m_normals[i].y << ' ' << m_normals[i].z << std::endl;
        allPositions[i] = m_positions[m_posIndices[i]];
        allUvs[i]= m_uvs[m_uvIndices[i]];

        if(m_normals_mode == ByPolygonVertex)
        {
            allNormals[i] = m_normals[i];
        }
        else
        {
            allNormals[i] = m_normals[m_posIndices[i]];
        }
    }
    Utils::StopTimeMeasure("Get All Data");

    std::vector<int> allIndices(m_indicesCount);
    std::iota (std::begin(allIndices), std::end(allIndices), 0);

    std::vector<std::pair<int, int>> replacements;
    std::vector<int> replacedRaw;
    int replacementsCount = 0;

    Utils::StartTimeMeasure();
    for (int i1 = 0; i1 < m_indicesCount; ++i1)
    {
        auto indexOne = allIndices[i1];
        auto posOne = allPositions[allIndices[i1]];
        auto uvOne = allUvs[allIndices[i1]];
        auto norOne = allNormals[allIndices[i1]];

        if(std::find(replacedRaw.begin(), replacedRaw.end(), indexOne) != replacedRaw.end())
        {
            continue;
        }
        
        for (int i2 = i1 + 1; i2 < m_indicesCount; ++i2)
        {            
            auto indexTwo = allIndices[i2];
            auto posTwo = allPositions[allIndices[i2]];
            auto uvTwo = allUvs[allIndices[i2]];
            auto norTwo = allNormals[allIndices[i2]];
    
            if(CompareFloat3(posOne, posTwo) && CompareFloat2(uvOne, uvTwo) && CompareFloat3(norOne, norTwo))
            {
                replacements.push_back(std::make_pair(indexOne, indexTwo));
                replacedRaw.push_back(indexTwo);
                ++replacementsCount;
            }
        }
    }
    Utils::StopTimeMeasure("Find repeated vertices");

    //std::sort(replacements.begin(), replacements.end(), [] (std::pair<int, int> one, std::pair<int, int> two) {return one.second < two.second;});

    Utils::StartTimeMeasure();
    for (int i = 0; i < replacements.size(); ++i)
    {
        ReplaceInIndices(&allIndices, replacements[i].second, replacements[i].first);
        UpdateReplacements(&replacements, replacements[i].second, replacements[i].first);
    }
    Utils::StopTimeMeasure("Reorder triangles");
    
    m_model.indexCount = allIndices.size();
    m_model.vertexCount = allPositions.size() - replacementsCount;
    m_model.vertices = new Vertex[m_model.vertexCount];

    Utils::StartTimeMeasure();
    int vertexIndex = 0;
    for (int i = 0; i < allPositions.size(); ++i)
    {
        if(std::find(replacedRaw.begin(), replacedRaw.end(), i) != replacedRaw.end())
        {
            continue;
        }
    
        m_model.vertices[vertexIndex].position = allPositions[i];
        m_model.vertices[vertexIndex].normal = allNormals[i];
        m_model.vertices[vertexIndex].uv = allUvs[i];
    
        // std::cout << allPositions[i].x << ' ' << allPositions[i].y << "     ";
        // std::cout << allUvs[i].x << ' ' << allUvs[i].y << std::endl;
        // std::cout << allPositions[i].x << ' ' << allPositions[i].y << ' ' << allPositions[i].z << "      " << allNormals[i].x << ' ' << allNormals[i].y << ' ' << allNormals[i].z << std::endl;

        ++vertexIndex;
    }
    Utils::StopTimeMeasure("Save final data");

    Utils::StartTimeMeasure();
    m_model.indices = new int[m_model.indexCount];
    for (int i = 0; i < m_model.indexCount; ++i)
    {
        m_model.indices[i] = allIndices[i];
    }
    Utils::StopTimeMeasure("Save indices");
}

void FBXImporter::UpdateReplacements(std::vector<std::pair<int, int>> *replacements, int oldIndex, int newIndex)
{
    for (int i = 0; i < replacements->size(); ++i)
    {
        if((*replacements)[i].second > oldIndex)
        {
            (*replacements)[i].second = (*replacements)[i].second - 1;
        }

        if((*replacements)[i].first > oldIndex)
        {
            (*replacements)[i].first = (*replacements)[i].first - 1;
        }
    }
}

void FBXImporter::ReplaceInIndices(std::vector<int> *triIndices, int oldIndex, int newIndex)
{
    for (auto &ind : *triIndices)
    {
        if (ind == oldIndex)
        {
            ind = newIndex;
        }

        if (ind > oldIndex)
        {
            --ind;
        }
    }
}

bool FBXImporter::CompareFloat3(float3 one, float3 two)
{
    return one.x == two.x && one.y == two.y && one.z == two.z;
}

bool FBXImporter::CompareFloat2(float2 one, float2 two)
{
    return one.x == two.x && one.y == two.y;
}

FBXImporter::FBXModel* FBXImporter::LoadModel(std::string file)
{    
    m_file.open(file);

    if(m_file)
    {

        Utils::StartTimeMeasure();
        std::string text;
        while (m_file >> text)
        {
            EvaluateWord(text);
        }
        Utils::StopTimeMeasure("Parse file ");

        DecodeMesh();
        
        m_file.close();
    }
    
    return &m_model;
}
