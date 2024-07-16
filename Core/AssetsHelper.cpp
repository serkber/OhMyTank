#include "AssetsHelper.h"
#include <D3Dcompiler.h>
#include "DDSTextureLoader.h"
#include "FBXImporter.h"
#include "Utils.h"

using RenderingData::BasicVertex;

AssetsHelper::AssetsHelper(Dx11Base* dx11App)
    : m_pDx11App(dx11App)
{
}

template <typename T>
bool AssetsHelper::LoadShader(const wchar_t* file, ID3DBlob** pShaderBlob, T** pShader)
{
    enum ShaderType
    {
        VertexShader,
        PixelShader
    };
    
    ShaderType shaderType = std::is_same_v<T, ID3D11VertexShader> ? VertexShader : PixelShader;

    const char* shaderModel = shaderType == VertexShader ? "vs_5_0" : "ps_5_0";
    const char* entryPoint = shaderType == VertexShader ? "vsmain" : "psmain";
    LPCWSTR shaderError = L"";
    
    //Load vertex Shader
    if (!CompileShader(file, entryPoint, shaderModel, pShaderBlob, &shaderError))
    {
        ::MessageBox(m_pDx11App->m_hWnd, shaderError, L"Vertex Shader Compilation Error", MB_OK);
        return false;
    }
    
    ID3DBlob* blob = (*pShaderBlob);
    
    // Create vertex shader
    HRESULT hr = 0;

    if(shaderType == VertexShader)
    {
        m_pDx11App->m_pD3DDevice->CreateVertexShader(
            blob->GetBufferPointer(),
            blob->GetBufferSize(),
            nullptr,
            reinterpret_cast<ID3D11VertexShader**>(pShader));
    }
    else if(shaderType == PixelShader)
    {
        m_pDx11App->m_pD3DDevice->CreatePixelShader(
            blob->GetBufferPointer(),
            blob->GetBufferSize(),
            nullptr,
            reinterpret_cast<ID3D11PixelShader**>(pShader));
    }
    if (FAILED(hr)) {
        ::MessageBox(m_pDx11App->m_hWnd, Utils::GetMessageFromHr(hr), L"Shader Creation Error", MB_OK);
        if (pShaderBlob)
            blob->Release();
        return false;
    }
    
    return true;
}
template bool AssetsHelper::LoadShader<ID3D11VertexShader>(const wchar_t* file, ID3DBlob** pVSBuffer, ID3D11VertexShader** pVertexShader);
template bool AssetsHelper::LoadShader<ID3D11PixelShader>(const wchar_t* file, ID3DBlob** pVSBuffer, ID3D11PixelShader** pVertexShader);

bool AssetsHelper::CompileShader(const wchar_t* shaderName, const char* shaderEntryPoint, LPCSTR shaderModel, ID3DBlob** buffer, LPCWSTR* errorMessage)
{
    ID3DBlob* errorBlob = nullptr;

    D3DCompileFromFile(shaderName, nullptr, nullptr, shaderEntryPoint, shaderModel, 0, 0, buffer, &errorBlob);
    if (errorBlob) {
        *errorMessage = Utils::GetMessageFromBlob((char*)errorBlob->GetBufferPointer());
        errorBlob->Release();
        return false;
    }

    return true;
}

bool AssetsHelper::LoadTexture(const wchar_t* file, ID3D11Resource** texture, ID3D11ShaderResourceView** textureView)
{
    HRESULT hr = DirectX::CreateDDSTextureFromFile(m_pDx11App->m_pD3DDevice, m_pDx11App->m_pD3DContext, file, texture, textureView);
    if (FAILED(hr))
    {
        ::MessageBox(m_pDx11App->m_hWnd, Utils::GetMessageFromHr(hr), L"Texture Load Error", MB_OK);
        return false;
    }

    return true;
}

bool AssetsHelper::CreateInputLayout(D3D11_INPUT_ELEMENT_DESC* descriptor, UINT elementsCount, ID3DBlob* vertexShaderBlob, ID3D11InputLayout** inputLayout)
{
    // Create input layout
    HRESULT hr = m_pDx11App->m_pD3DDevice->CreateInputLayout(
        descriptor,
        elementsCount,
        vertexShaderBlob->GetBufferPointer(),
        vertexShaderBlob->GetBufferSize(),
        inputLayout);
    if (FAILED(hr)) {
        ::MessageBox(m_pDx11App->m_hWnd, Utils::GetMessageFromHr(hr), L"Input Layout Error", MB_OK);
        return false;
    }

    return true;
}

void AssetsHelper::FullfillBasicVertexArray(BasicVertex* array, UINT elementsCount, FBXImporter::FBXModel *model)
{  
    for (int i = 0; i < elementsCount; ++i)
    {
        array[i].pos = model->vertices[i].position;
        array[i].nor = model->vertices[i].normal;
        array[i].uv.x = model->vertices[i].uv.x;
        array[i].uv.y = 1 - model->vertices[i].uv.y;
    }
}

template<typename T>
bool AssetsHelper::CreateModelBuffers(FBXImporter::FBXModel* model, ID3D11Buffer** vertexBuffer, ID3D11Buffer** indexBuffer)
{
    // Vertex buffer description
    D3D11_BUFFER_DESC vertexDesc;
    ::ZeroMemory(&vertexDesc, sizeof(vertexDesc));
    vertexDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexDesc.CPUAccessFlags = 0;
    vertexDesc.ByteWidth = sizeof(T) * model->vertexCount;

    T* vertices = new T[model->vertexCount];

    if(std::is_same_v<T, BasicVertex>)
    {
        FullfillBasicVertexArray(reinterpret_cast<BasicVertex*>(vertices), model->vertexCount, model);
    }
    else
    {
        return false;
    }

    // Resource data
    D3D11_SUBRESOURCE_DATA verticesData;
    ZeroMemory(&verticesData, sizeof(verticesData));
    verticesData.pSysMem = vertices;

    // Create vertex buffer
    HRESULT hr =  m_pDx11App->m_pD3DDevice->CreateBuffer(&vertexDesc, &verticesData, vertexBuffer);
    if (FAILED(hr)) {
        ::MessageBox(m_pDx11App->m_hWnd, Utils::GetMessageFromHr(hr), L"Vertex Buffer Error", MB_OK);
        return false;
    }

    // Index buffer data
    WORD* indices = new WORD[model->indexCount];

    for (int i = 0; i < model->indexCount; ++i)
    {
        indices[i] = model->indices[i];
    }
    
    D3D11_SUBRESOURCE_DATA resourceData;
    ZeroMemory(&resourceData, sizeof(resourceData));

    // Create index buffer
    D3D11_BUFFER_DESC indexBufferDesc;
    ::ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.ByteWidth = sizeof( WORD ) * model->indexCount;
    indexBufferDesc.CPUAccessFlags = 0;
    resourceData.pSysMem = indices;
    hr = m_pDx11App->m_pD3DDevice->CreateBuffer(&indexBufferDesc, &resourceData, indexBuffer);

    delete indices;
    
    if (FAILED(hr)) {
        ::MessageBox(m_pDx11App->m_hWnd, Utils::GetMessageFromHr(hr), L"Index Buffer Error", MB_OK);
        return false;
    }

    return true;
}
template bool AssetsHelper::CreateModelBuffers<BasicVertex>(FBXImporter::FBXModel* model, ID3D11Buffer** vertexBuffer, ID3D11Buffer** indicesBuffer);