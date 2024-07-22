#include "OMTRender.h"
#include "AssetsHelper.h"
#include "Resources.h"
#include "OMTGame.h"
#include "SimpleMath.h"
#include "RenderingData.h"

using RenderingData::BasicVertex, RenderingData::basicInputLayoutDescriptor;

OMTRender::OMTRender()
{
    m_pTankVS = nullptr;
    m_pTankPS = nullptr;
    m_pBasicInputLayout = nullptr;
    m_pVertexBufferTank = nullptr;
    m_pIndexBufferTank = nullptr;
    m_pVertexBufferQuad = nullptr;
    m_pIndexBufferQuad = nullptr;
    m_pTankMap = nullptr;
    m_pTextureSampler = nullptr;
    m_pTankMapResource = nullptr;
    m_pAlphaBlendState = nullptr;
    m_pViewCB = nullptr;
    m_pProjCB = nullptr;
    m_pModelCB = nullptr;
    m_pDataCB = nullptr;
    m_pGrassInitPS = nullptr;
    m_pRenderTexture1 = nullptr;
    m_pRenderTexture1Resource = nullptr;
    m_pRenderTexture1TargetView = nullptr;
    m_pRenderTexture2 = nullptr;
    m_pRenderTexture2TargetView = nullptr;
    m_pRenderTexture2Resource = nullptr;
    m_pTextureDebugVS = nullptr;
    m_pRenderTextureVS = nullptr;
    m_pRenderTexturePS = nullptr;
    m_pGrassFieldVS = nullptr;
    m_pGrassFieldPS = nullptr;
    m_pPostpoTexture1 = nullptr;
    m_pPostpoTexture1Resource = nullptr;
    m_pMultiplyBlendState = nullptr;
    m_pVertexBufferGrassField = nullptr;
    m_pIndexBufferGrassField = nullptr;
    m_pFullScreenVS = nullptr;
    m_pInstantiableInputLayout = nullptr;
    m_pInstancesBufferGrassField = nullptr;
    m_pSilhouettePS = nullptr;
    m_pBlurPS = nullptr;
    m_pFXAAPS = nullptr;
    m_pPostpoTexture2 = nullptr;
    m_pPostpoTexture2Resource = nullptr;
    
    ::ZeroMemory(&m_data, sizeof(DataCB));
}

OMTRender::~OMTRender()
{
}

#define RENDER_TEXTURE_SIZE 128
bool OMTRender::SetupRenderTexture(ID3D11Texture2D** renderTexture, ID3D11ShaderResourceView** resourceView, ID3D11RenderTargetView** renderTarget)
{
    D3D11_TEXTURE2D_DESC desc;
    desc.ArraySize = 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Width = RENDER_TEXTURE_SIZE;
    desc.Height = RENDER_TEXTURE_SIZE * floor(GRASS_FIELD_ASPECT);
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.MipLevels = 1;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    HRESULT hr = OMTGame::m_pGameInstance->m_pD3DDevice->CreateTexture2D(&desc, nullptr, renderTexture);
    if (FAILED(hr)) {
        ::MessageBox(OMTGame::m_pGameInstance->m_hWnd, L"Failed to create destination texture.", L"Error", MB_OK);
        return false;
    }

    hr = OMTGame::m_pGameInstance->m_pD3DDevice->CreateShaderResourceView(*renderTexture, nullptr, resourceView);
    if (FAILED(hr)) {
        ::MessageBox(OMTGame::m_pGameInstance->m_hWnd, L"Failed to create Shader Resource View from destination texture.", L"Error", MB_OK);
        return false;
    }

    // Create the render target view
    hr = OMTGame::m_pGameInstance->m_pD3DDevice->CreateRenderTargetView(*renderTexture, nullptr, renderTarget);
    // Check render target view
    if (FAILED(hr)) {
        MessageBox(OMTGame::m_pGameInstance->m_hWnd, Utils::GetMessageFromHr(hr), TEXT("Create render target ERROR"), MB_OK);
        return false;
    }

    return true;
}

void OMTRender::InitGrass(ID3D11RenderTargetView** renderTarget)
{
    auto context = OMTGame::m_pGameInstance->m_pD3DContext;
    
    float bgColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f }; 
    OMTGame::m_pGameInstance->m_pD3DContext->ClearRenderTargetView(*renderTarget, bgColor);

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    SetRenderViewport(RENDER_TEXTURE_SIZE, RENDER_TEXTURE_SIZE * floor(GRASS_FIELD_ASPECT));
    context->IASetInputLayout(m_pBasicInputLayout);  
    float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    context->OMSetBlendState(m_pMultiplyBlendState, blendFactor, 0xFFFFFFFF);
    context->VSSetShader(m_pFullScreenVS, nullptr, 0);
    context->PSSetShader(m_pGrassInitPS, nullptr, 0);
    context->OMSetRenderTargets(1, renderTarget, nullptr);
    UINT stride = sizeof(BasicVertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &m_pVertexBufferQuad, &stride, &offset);
    context->IASetIndexBuffer(m_pIndexBufferQuad, DXGI_FORMAT_R16_UINT, 0);

    context->DrawIndexed(m_quadModel.indexCount, 0, 0);
}

bool OMTRender::CreateTextureAndResource(UINT width, UINT height, ID3D11Texture2D** texture, ID3D11ShaderResourceView** resource)
{
    D3D11_TEXTURE2D_DESC desc;
    desc.ArraySize = 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Width = width;
    desc.Height = height;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.MipLevels = 1;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    HRESULT hr = OMTGame::m_pGameInstance->m_pD3DDevice->CreateTexture2D(&desc, nullptr, texture);
    if (FAILED(hr)) {
        ::MessageBox(OMTGame::m_pGameInstance->m_hWnd, L"Failed to create texture.", L"Error", MB_OK);
        return false;
    }

    hr = OMTGame::m_pGameInstance->m_pD3DDevice->CreateShaderResourceView(*texture, nullptr, resource);
    if (FAILED(hr)) {
        ::MessageBox(OMTGame::m_pGameInstance->m_hWnd, L"Failed to create Shader Resource View of texture.", L"Error", MB_OK);
        return false;
    }
    return true;
}

bool OMTRender::LoadGraphicContent()
{
    ID3DBlob* pShaderBlob = nullptr;
    if(!m_assetsHelper.LoadShader<ID3D11VertexShader>(L"Shaders/TankVS.hlsl", &pShaderBlob, &m_pTankVS))
    {
        return false;
    }
    
    if (!m_assetsHelper.CreateInputLayout(basicInputLayoutDescriptor.data(), basicInputLayoutDescriptor.size(), pShaderBlob, &m_pBasicInputLayout))
    {
        return false;
    }
    
    if (!m_assetsHelper.LoadShader<ID3D11PixelShader>(L"Shaders/TankPS.hlsl", &pShaderBlob, &m_pTankPS))
    {
        return false;
    }
    
    if (!m_assetsHelper.LoadShader<ID3D11PixelShader>(L"Shaders/TextureDebugPS.hlsl", &pShaderBlob, &m_pTextureDebugPS))
    {
        return false;
    }    
    if(!m_assetsHelper.LoadShader<ID3D11VertexShader>(L"Shaders/TextureDebugVS.hlsl", &pShaderBlob, &m_pTextureDebugVS))
    {
        return false;
    }
    
    if (!m_assetsHelper.LoadShader<ID3D11PixelShader>(L"Shaders/RenderTexturePS.hlsl", &pShaderBlob, &m_pRenderTexturePS))
    {
        return false;
    }    
    if(!m_assetsHelper.LoadShader<ID3D11VertexShader>(L"Shaders/RenderTextureVS.hlsl", &pShaderBlob, &m_pRenderTextureVS))
    {
        return false;
    }
    
    if (!m_assetsHelper.LoadShader<ID3D11PixelShader>(L"Shaders/GrassPS.hlsl", &pShaderBlob, &m_pGrassFieldPS))
    {
        return false;
    }

    if(!m_assetsHelper.LoadShader<ID3D11VertexShader>(L"Shaders/GrassVS.hlsl", &pShaderBlob, &m_pGrassFieldVS))
    {
        return false;
    }    
    if (!m_assetsHelper.CreateInputLayout(
        RenderingData::instantiableInputLayoutDescriptor.data(),
        RenderingData::instantiableInputLayoutDescriptor.size(),
        pShaderBlob,
        &m_pInstantiableInputLayout)
        )
    {
        return false;
    }
    
    if(!m_assetsHelper.LoadShader<ID3D11VertexShader>(L"Shaders/FullScreenVS.hlsl", &pShaderBlob, &m_pFullScreenVS))
    {
        return false;
    }
    if(!m_assetsHelper.LoadShader<ID3D11PixelShader>(L"Shaders/GrassInitPS.hlsl", &pShaderBlob, &m_pGrassInitPS))
    {
        return false;
    }
    if(!m_assetsHelper.LoadShader<ID3D11PixelShader>(L"Shaders/SilhouettePS.hlsl", &pShaderBlob, &m_pSilhouettePS))
    {
        return false;
    }
    if(!m_assetsHelper.LoadShader<ID3D11PixelShader>(L"Shaders/FXAAPS.hlsl", &pShaderBlob, &m_pFXAAPS))
    {
        return false;
    }
    if(!m_assetsHelper.LoadShader<ID3D11PixelShader>(L"Shaders/BlurPS.hlsl", &pShaderBlob, &m_pBlurPS))
    {
        return false;
    }

    pShaderBlob->Release();
    pShaderBlob = nullptr;

    FBXImporter modelImporter;
    modelImporter.LoadModel("Models/Tank.fbx");
    m_tankModel = modelImporter.m_model;
    if (!m_assetsHelper.CreateModelBuffers<BasicVertex>(&m_tankModel, &m_pVertexBufferTank, &m_pIndexBufferTank))
    {
        return false;
    }
    modelImporter.LoadModel("Models/Quad.fbx");
    m_quadModel = modelImporter.m_model;
    if (!m_assetsHelper.CreateModelBuffers<BasicVertex>(&m_quadModel, &m_pVertexBufferQuad, &m_pIndexBufferQuad))
    {
        return false;
    }
    modelImporter.LoadModel("Models/GrassField.fbx");
    m_grassFieldModel = modelImporter.m_model;
    if (!m_assetsHelper.CreateModelBuffers<BasicVertex>(&m_grassFieldModel, &m_pVertexBufferGrassField, &m_pIndexBufferGrassField))
    {
        return false;
    }

    if(!CreateGrassInstancesBuffer())
    {
        return false;
    }
    
    if (!CreateConstantBuffers())
    {
        return false;
    }

    if (!m_assetsHelper.LoadTexture(Textures[Tex::Tank], &m_pTankMap, &m_pTankMapResource))
    {
        return false;
    }
    
    if (!SetupRenderTexture(&m_pRenderTexture1, &m_pRenderTexture1Resource, &m_pRenderTexture1TargetView))
    {
        return false;
    }
    if (!SetupRenderTexture(&m_pRenderTexture2, &m_pRenderTexture2Resource, &m_pRenderTexture2TargetView))
    {
        return false;
    }
    
    if (!CreateTextureAndResource(OMTGame::m_pGameInstance->m_windSize.x, OMTGame::m_pGameInstance->m_windSize.y, &m_pPostpoTexture1, &m_pPostpoTexture1Resource))
    {
        return false;
    }    
    if (!CreateTextureAndResource(OMTGame::m_pGameInstance->m_windSize.x, OMTGame::m_pGameInstance->m_windSize.y, &m_pPostpoTexture2, &m_pPostpoTexture2Resource))
    {
        return false;
    }

    CreateBlendingResources();

    CreateTextureSampler();

    InitGrass(&m_pRenderTexture1TargetView);
    InitGrass(&m_pRenderTexture2TargetView);
    
    m_pGrassBuffers[0] = m_pVertexBufferGrassField;
    m_pGrassBuffers[1] = m_pInstancesBufferGrassField;

    m_resources.push_back((ID3D11Resource**)&m_pTankVS);
    m_resources.push_back((ID3D11Resource**)&m_pTankPS);
    m_resources.push_back((ID3D11Resource**)&m_pBasicInputLayout);
    m_resources.push_back((ID3D11Resource**)&m_pVertexBufferTank);
    m_resources.push_back((ID3D11Resource**)&m_pIndexBufferTank);
    m_resources.push_back((ID3D11Resource**)&m_pVertexBufferQuad);
    m_resources.push_back((ID3D11Resource**)&m_pIndexBufferQuad);
    m_resources.push_back((ID3D11Resource**)&m_pAlphaBlendState);
    m_resources.push_back((ID3D11Resource**)&m_pTextureSampler);
    m_resources.push_back((ID3D11Resource**)&m_pTankMapResource);
    m_resources.push_back(&m_pTankMap);
    m_resources.push_back((ID3D11Resource**)&m_pTextureDebugPS);
    m_resources.push_back((ID3D11Resource**)&m_pViewCB);
    m_resources.push_back((ID3D11Resource**)&m_pProjCB);
    m_resources.push_back((ID3D11Resource**)&m_pModelCB);
    m_resources.push_back((ID3D11Resource**)&m_pDataCB);
    m_resources.push_back((ID3D11Resource**)&m_pRenderTexture1Resource);
    m_resources.push_back((ID3D11Resource**)&m_pRenderTexture1);
    m_resources.push_back((ID3D11Resource**)&m_pRenderTexture1TargetView);
    m_resources.push_back((ID3D11Resource**)&m_pRenderTexture2Resource);
    m_resources.push_back((ID3D11Resource**)&m_pRenderTexture2);
    m_resources.push_back((ID3D11Resource**)&m_pRenderTexture2TargetView);
    m_resources.push_back((ID3D11Resource**)&m_pTextureDebugVS);
    m_resources.push_back((ID3D11Resource**)&m_pRenderTexturePS);
    m_resources.push_back((ID3D11Resource**)&m_pRenderTextureVS);
    m_resources.push_back((ID3D11Resource**)&m_pGrassFieldVS);
    m_resources.push_back((ID3D11Resource**)&m_pGrassFieldPS);
    m_resources.push_back((ID3D11Resource**)&m_pMultiplyBlendState);
    m_resources.push_back((ID3D11Resource**)&m_pVertexBufferGrassField);
    m_resources.push_back((ID3D11Resource**)&m_pIndexBufferGrassField);
    m_resources.push_back((ID3D11Resource**)&m_pGrassInitPS);
    m_resources.push_back((ID3D11Resource**)&m_pFullScreenVS);
    m_resources.push_back((ID3D11Resource**)&m_pInstantiableInputLayout);
    m_resources.push_back((ID3D11Resource**)&m_pInstancesBufferGrassField);
    m_resources.push_back((ID3D11Resource**)&m_pPostpoTexture1Resource);
    m_resources.push_back((ID3D11Resource**)&m_pPostpoTexture1);
    m_resources.push_back((ID3D11Resource**)&m_pSilhouettePS);
    m_resources.push_back((ID3D11Resource**)&m_pBlurPS);
    m_resources.push_back((ID3D11Resource**)&m_pFXAAPS);
    m_resources.push_back((ID3D11Resource**)&m_pPostpoTexture2);
    m_resources.push_back((ID3D11Resource**)&m_pPostpoTexture2Resource);

    return true;
}

bool OMTRender::CreateConstantBuffers()
{
    // Create world matrix buffer
    D3D11_BUFFER_DESC constDesc;
    ::ZeroMemory(&constDesc, sizeof(constDesc));
    constDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constDesc.ByteWidth = sizeof(matrix);
    constDesc.Usage = D3D11_USAGE_DEFAULT;
    HRESULT hr = OMTGame::m_pGameInstance->m_pD3DDevice->CreateBuffer(&constDesc, nullptr, &m_pViewCB);
    if (FAILED(hr)) {
        ::MessageBox(OMTGame::m_pGameInstance->m_hWnd, Utils::GetMessageFromHr(hr), L"View Constant Buffer Error", MB_OK);
        return false;
    }
    hr = OMTGame::m_pGameInstance->m_pD3DDevice->CreateBuffer(&constDesc, nullptr, &m_pProjCB);
    if (FAILED(hr)) {
        ::MessageBox(OMTGame::m_pGameInstance->m_hWnd, Utils::GetMessageFromHr(hr), L"Projection Constant Buffer Error", MB_OK);
        return false;
    }
    hr = OMTGame::m_pGameInstance->m_pD3DDevice->CreateBuffer(&constDesc, nullptr, &m_pModelCB);
    if (FAILED(hr)) {
        ::MessageBox(OMTGame::m_pGameInstance->m_hWnd, Utils::GetMessageFromHr(hr), L"Model Constant Buffer Error", MB_OK);
        return false;
    }
    
    D3D11_BUFFER_DESC dataDesc;
    ::ZeroMemory(&dataDesc, sizeof(dataDesc));
    dataDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    dataDesc.ByteWidth = sizeof(DataCB);
    dataDesc.Usage = D3D11_USAGE_DEFAULT;
    
    hr = OMTGame::m_pGameInstance->m_pD3DDevice->CreateBuffer(&dataDesc, nullptr, &m_pDataCB);
    if (FAILED(hr)) {
        ::MessageBox(OMTGame::m_pGameInstance->m_hWnd, Utils::GetMessageFromHr(hr), L"Data Constant Buffer Error", MB_OK);
        return false;
    }
    
    OMTGame::m_pGameInstance->m_pD3DContext->VSSetConstantBuffers(0, 1, &m_pModelCB);
    OMTGame::m_pGameInstance->m_pD3DContext->VSSetConstantBuffers(1, 1, &m_pViewCB);
    OMTGame::m_pGameInstance->m_pD3DContext->VSSetConstantBuffers(2, 1, &m_pProjCB);
    OMTGame::m_pGameInstance->m_pD3DContext->VSSetConstantBuffers(3, 1, &m_pDataCB);
    OMTGame::m_pGameInstance->m_pD3DContext->PSSetConstantBuffers(0, 1, &m_pDataCB);
    
    return true;
}

void OMTRender::CreateBlendingResources()
{
    D3D11_BLEND_DESC blendDesc;
    ::ZeroMemory(&blendDesc, sizeof(blendDesc));
    // Alpha blend
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;    
    OMTGame::m_pGameInstance->m_pD3DDevice->CreateBlendState(&blendDesc, &m_pAlphaBlendState);

    // Multiply
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_COLOR;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    OMTGame::m_pGameInstance->m_pD3DDevice->CreateBlendState(&blendDesc, &m_pMultiplyBlendState);
}

bool OMTRender::CreateTextureSampler()
{
    D3D11_SAMPLER_DESC textureDesc;
    ::ZeroMemory(&textureDesc, sizeof(textureDesc));
    textureDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    textureDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    textureDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    textureDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    textureDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    textureDesc.MaxLOD = D3D11_FLOAT32_MAX;
    HRESULT hr = OMTGame::m_pGameInstance->m_pD3DDevice->CreateSamplerState(&textureDesc, &m_pTextureSampler);
    if (FAILED(hr)) {
        ::MessageBox(OMTGame::m_pGameInstance->m_hWnd, Utils::GetMessageFromHr(hr), L"Texture Sampler Error", MB_OK);
        return false;
    }

    return true;
}

void OMTRender::SetRenderViewport(float width, float height)
{
    D3D11_VIEWPORT viewPort;
    viewPort.Width = width;
    viewPort.Height = height;
    viewPort.MinDepth = 0.0f;
    viewPort.MaxDepth = 1.0f;
    viewPort.TopLeftX = 0;
    viewPort.TopLeftY = 0;
    OMTGame::m_pGameInstance->m_pD3DContext->RSSetViewports(1, &viewPort);
}

void OMTRender::CreateDebugTextureMatrix()
{
    m_debugTextureMatrix = DirectX::XMMatrixScaling(0.4, 0.4 * GRASS_FIELD_ASPECT, 0.4);
    m_debugTextureMatrix *= DirectX::XMMatrixTranslation(0.7, 0.7, 0);
}

bool OMTRender::CreateGrassInstancesBuffer()
{
    HRESULT hr;

    // Instance buffer description
    D3D11_BUFFER_DESC instanceDesc;
    ::ZeroMemory(&instanceDesc, sizeof(instanceDesc));
    instanceDesc.Usage = D3D11_USAGE_DEFAULT;
    instanceDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    instanceDesc.CPUAccessFlags = 0;
    instanceDesc.ByteWidth = sizeof(RenderingData::InstanceData) * GRASS_FIELD_SIZE * GRASS_FIELD_SIZE;

    // Create particles buffer
    hr = OMTGame::m_pGameInstance->m_pD3DDevice->CreateBuffer(&instanceDesc, nullptr, &m_pInstancesBufferGrassField);
    if (FAILED(hr)) {
        ::MessageBox(OMTGame::m_pGameInstance->m_hWnd, Utils::GetMessageFromHr(hr), L"Particles Buffer Error", MB_OK);
        return false;
    }

    return true;
}

void OMTRender::RenderGrass(float grassPos, ID3D11ShaderResourceView** grassTexture)
{
    auto context = OMTGame::m_pGameInstance->m_pD3DContext;
    context->VSSetShader(m_pGrassFieldVS, nullptr, 0);
    context->PSSetShader(m_pGrassFieldPS, nullptr, 0);
    UINT strides[2] = { sizeof(BasicVertex), sizeof(RenderingData::InstanceData) };
    UINT offsets[2] = { 0, 0 };
    context->IASetVertexBuffers(0, 2, m_pGrassBuffers, strides, offsets);
    context->IASetIndexBuffer(m_pIndexBufferGrassField, DXGI_FORMAT_R16_UINT, 0);
    context->VSSetShaderResources(0, 1, grassTexture);

    m_data.grassPos = grassPos;
    m_data.grassAspect = GRASS_FIELD_ASPECT;
    context->UpdateSubresource(m_pDataCB, 0, nullptr, &m_data, 0, 0);

    const UINT instancesCount = GRASS_FIELD_SIZE * GRASS_FIELD_SIZE;
    UINT instanceDataCounter = 0;
    
    RenderingData::InstanceData instancesData[instancesCount];

    float xOffset = GRASS_FIELD_SIZE / 2.0f - 0.5f;
    float yOffset = (GRASS_FIELD_SIZE * GRASS_FIELD_ASPECT / 2.0f);

    for (int x = 0; x < GRASS_FIELD_SIZE; ++x)
    {
        for (int y = 0; y < GRASS_FIELD_SIZE; ++y)
        {
            auto grassPatch = DirectX::XMMatrixScaling(.1, .1, .1 * GRASS_FIELD_ASPECT);
            grassPatch *= DirectX::XMMatrixTranslation(x - xOffset, 0 , grassPos - yOffset + y * GRASS_FIELD_ASPECT);            
            grassPatch = DirectX::XMMatrixTranspose(grassPatch);

            instancesData[instanceDataCounter].model = grassPatch;
            instanceDataCounter ++;
        }
    }
    
    context->UpdateSubresource(m_pInstancesBufferGrassField, 0, 0, instancesData, 0, 0);

    context->DrawIndexedInstanced(m_grassFieldModel.indexCount, instancesCount, 0, 0, 0);
}

void OMTRender::RenderGrassTexture(ID3D11RenderTargetView** renderTarget, matrix grassMatrix, bool isCurrentGrass)
{
    auto context = OMTGame::m_pGameInstance->m_pD3DContext;
    
    UINT stride = sizeof(BasicVertex);
    UINT offset = 0;
    context->OMSetDepthStencilState(OMTGame::m_pGameInstance->m_pDepthStencilStateDisabled, 0);
    SetRenderViewport(RENDER_TEXTURE_SIZE, RENDER_TEXTURE_SIZE * floor(GRASS_FIELD_ASPECT));
    context->IASetVertexBuffers(0, 1, &m_pVertexBufferQuad, &stride, &offset);
    context->IASetIndexBuffer(m_pIndexBufferQuad, DXGI_FORMAT_R16_UINT, 0);
    float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    context->OMSetBlendState(m_pMultiplyBlendState, blendFactor, 0xFFFFFFFF);
    context->OMSetRenderTargets(1, renderTarget, nullptr);
    context->VSSetShader(m_pRenderTextureVS, nullptr, 0);
    context->PSSetShader(m_pRenderTexturePS, nullptr, 0);
    
    auto inverseGrassMatrix = DirectX::XMMatrixInverse(nullptr, grassMatrix);
    auto matrixScalingFactor = 1.0f / GRASS_FIELD_SIZE * 2.0f;
    inverseGrassMatrix *= DirectX::XMMatrixScaling(matrixScalingFactor, matrixScalingFactor, matrixScalingFactor / GRASS_FIELD_ASPECT);

    if(OMTGame::m_pGameInstance->m_isPainting)
    {
        // Mouse painting
        auto inverseDebugTextureMatrix = DirectX::XMMatrixInverse(nullptr, m_debugTextureMatrix);
        auto brushPos = DirectX::XMVector2Transform(
            DirectX::XMVectorSet(OMTGame::m_pGameInstance->m_input.m_mousePosNorm.x, OMTGame::m_pGameInstance->m_input.m_mousePosNorm.y, 0, 0),
            inverseDebugTextureMatrix
        );

        auto brushMatrix = DirectX::XMMatrixScaling(0.2, 0.2, 1);
        brushMatrix *= DirectX::XMMatrixTranslation(brushPos.m128_f32[0] * 2, -brushPos.m128_f32[1] * 2, 0);            
        brushMatrix = XMMatrixTranspose(brushMatrix);
        context->UpdateSubresource(m_pModelCB, 0, nullptr, &brushMatrix, 0, 0);
        
        m_data.square = 0;
        context->UpdateSubresource(m_pDataCB, 0, nullptr, &m_data, 0, 0);
        context->DrawIndexed(m_quadModel.indexCount, 0, 0);
    }
        
    // tankPainting
    auto brushPos = DirectX::XMVector3Transform(OMTGame::m_pGameInstance->m_tankPos, inverseGrassMatrix);

    if(isCurrentGrass && brushPos.m128_f32[2] > 1.25)
    {
        if(m_currentGrass == 0)
        {
            m_currentGrass = 1;
            OMTGame::m_pGameInstance->m_grass1Pos += GRASS_FIELD_SIZE * GRASS_FIELD_ASPECT * 2;
            InitGrass(&m_pRenderTexture1TargetView);
        }
        else
        {
            m_currentGrass = 0;
            OMTGame::m_pGameInstance->m_grass2Pos += GRASS_FIELD_SIZE * GRASS_FIELD_ASPECT * 2;
            InitGrass(&m_pRenderTexture2TargetView);
        }
    }

    auto brushScalingFactor = 1.0f / GRASS_FIELD_SIZE * 5.5f;
    auto brushMatrix = DirectX::XMMatrixScaling(brushScalingFactor, brushScalingFactor, 1);
    brushMatrix *= DirectX::XMMatrixRotationZ(OMTGame::m_pGameInstance->m_tankRot);
    brushMatrix *= DirectX::XMMatrixTranslation(brushPos.m128_f32[0], -brushPos.m128_f32[2], 0);        
    brushMatrix = XMMatrixTranspose(brushMatrix);
    context->UpdateSubresource(m_pModelCB, 0, nullptr, &brushMatrix, 0, 0);
        
    m_data.square = 1;
    context->UpdateSubresource(m_pDataCB, 0, nullptr, &m_data, 0, 0);
    
    context->DrawIndexed(m_quadModel.indexCount, 0, 0);
}

void OMTRender::UpdateCameraBuffers(ID3D11DeviceContext* context)
{
    m_viewMatrix = DirectX::XMMatrixInverse(nullptr, OMTGame::m_pGameInstance->m_cameraMatrix);
    m_viewMatrix = DirectX::XMMatrixTranspose(m_viewMatrix);    
    m_projMatrix = DirectX::XMMatrixPerspectiveFovLH(1.0f, (float)OMTGame::m_pGameInstance->m_windSize.x / OMTGame::m_pGameInstance->m_windSize.y, 0.1f, 500.0f);
    m_projMatrix = DirectX::XMMatrixTranspose(m_projMatrix);
}

void OMTRender::Render()
{
    UINT stride = sizeof(BasicVertex);
    UINT offset = 0;
    
    CreateDebugTextureMatrix();
    
    auto context = OMTGame::m_pGameInstance->m_pD3DContext;
    // Check if D3D is ready
    if (context == nullptr)
        return;
    
    //Update camera buffers
    UpdateCameraBuffers(context);
    context->UpdateSubresource(m_pViewCB, 0, nullptr, &m_viewMatrix, 0, 0);
    context->UpdateSubresource(m_pProjCB, 0, nullptr, &m_projMatrix, 0, 0);

    //Update data buffer
    m_data.elapsedTime = OMTGame::m_pGameInstance->m_elapsedTime;
    m_data.mouseX = OMTGame::m_pGameInstance->m_input.m_mousePosNorm.x;
    m_data.mouseY = OMTGame::m_pGameInstance->m_input.m_mousePosNorm.y;
    m_data.grassfieldSize = GRASS_FIELD_SIZE;
    m_data.camPos = DirectX::XMVectorGetZ(OMTGame::m_pGameInstance->m_camPos);
    context->UpdateSubresource(m_pDataCB, 0, nullptr, &m_data, 0, 0);    
    std::cout << m_data.mouseX << ' ' << m_data.mouseY << std::endl;
    
    // Set stuff
    context->PSSetSamplers(0, 1, &m_pTextureSampler);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    //Render Texture drawcall
    context->IASetInputLayout(m_pBasicInputLayout);
    //////////////////////////////////
    if(renderTextureTimer < 0.01666)
    {
        renderTextureTimer += OMTGame::m_pGameInstance->m_deltaTime;
    }
    else
    {
        renderTextureTimer = 0;
        RenderGrassTexture(&m_pRenderTexture1TargetView, OMTGame::m_pGameInstance->m_grass1Matrix, m_currentGrass == 0);
        RenderGrassTexture(&m_pRenderTexture2TargetView, OMTGame::m_pGameInstance->m_grass2Matrix, m_currentGrass == 1);
    }
    
    // World painting settings
    /////////////////////////////// 
    float bgColor[4] = { 0.2f, 0.2f, 0.15f, 0.0f };
    context->ClearRenderTargetView(OMTGame::m_pGameInstance->m_pD3DRenderTargetView, bgColor);
    context->ClearDepthStencilView(OMTGame::m_pGameInstance->m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &OMTGame::m_pGameInstance->m_pD3DRenderTargetView, OMTGame::m_pGameInstance->m_pDepthStencilView);
    SetRenderViewport(OMTGame::m_pGameInstance->m_windSize.x,OMTGame::m_pGameInstance->m_windSize.y);
    context->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(OMTGame::m_pGameInstance->m_pDepthStencilState, 0);
    ////////////////////////////////

    // Render grass
    ////////////////////////
    context->IASetInputLayout(m_pInstantiableInputLayout);
    RenderGrass(OMTGame::m_pGameInstance->m_grass1Pos, &m_pRenderTexture1Resource);
    RenderGrass(OMTGame::m_pGameInstance->m_grass2Pos, &m_pRenderTexture2Resource);
    ////////////////////
    
    // Render tank
    ///////////////////////////   
    context->OMSetDepthStencilState(OMTGame::m_pGameInstance->m_pDepthStencilState, 0);
    context->IASetInputLayout(m_pBasicInputLayout);     
    context->VSSetShader(m_pTankVS, nullptr, 0);
    context->PSSetShader(m_pTankPS, nullptr, 0);
    context->IASetVertexBuffers(0, 1, &m_pVertexBufferTank, &stride, &offset);
    context->IASetIndexBuffer(m_pIndexBufferTank, DXGI_FORMAT_R16_UINT, 0);
    
    auto transposedTankMatrix = XMMatrixTranspose(OMTGame::m_pGameInstance->m_tankMatrix);
    context->UpdateSubresource(m_pModelCB, 0, nullptr, &transposedTankMatrix, 0, 0);
    
    context->PSSetShaderResources(0, 1, &m_pTankMapResource);
    context->DrawIndexed(m_tankModel.indexCount, 0, 0);
    //////////////////

    if(OMTGame::m_pGameInstance->m_isFXAAEnabled)
    {
        // FXAA
        /////////////////////
        context->IASetInputLayout(m_pBasicInputLayout);
        context->OMSetRenderTargets(1, &OMTGame::m_pGameInstance->m_pD3DRenderTargetView, OMTGame::m_pGameInstance->m_pDepthStencilView);
        context->OMSetDepthStencilState(OMTGame::m_pGameInstance->m_pDepthStencilStateDisabled, 0);
        context->IASetVertexBuffers(0, 1, &m_pVertexBufferQuad, &stride, &offset);
        context->IASetIndexBuffer(m_pIndexBufferQuad, DXGI_FORMAT_R16_UINT, 0);
        context->VSSetShader(m_pFullScreenVS, nullptr, 0);

        //Silhouette to backBuffer and backBuffer to Texture1
        context->PSSetShader(m_pSilhouettePS, nullptr, 0);
        context->CopyResource(m_pPostpoTexture1, OMTGame::m_pGameInstance->m_pBackBuffer);
        context->PSSetShaderResources(0, 1, &m_pPostpoTexture1Resource);
        context->DrawIndexed(m_quadModel.indexCount, 0, 0);

        //Silhouette to Texture1 and blurred Silhouette to backBuffer
        context->CopyResource(m_pPostpoTexture2, OMTGame::m_pGameInstance->m_pBackBuffer);
        context->PSSetShaderResources(0, 1, &m_pPostpoTexture2Resource);
        context->PSSetShader(m_pBlurPS, nullptr, 0);
        context->DrawIndexed(m_quadModel.indexCount, 0, 0);

        //Blurred silhouette to Texture2
        context->PSSetShader(m_pFXAAPS, nullptr, 0);
        context->CopyResource(m_pPostpoTexture2, OMTGame::m_pGameInstance->m_pBackBuffer);    
        context->PSSetShaderResources(1, 1, &m_pPostpoTexture1Resource);
        context->DrawIndexed(m_quadModel.indexCount, 0, 0);
        ////////////////////
    }
    
    if(OMTGame::m_pGameInstance->m_isDebugEnabled)
    {
        //Render Texture debug drawcall
        //////////////////////////////////
        context->IASetInputLayout(m_pBasicInputLayout);
        float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        context->OMSetBlendState(m_pAlphaBlendState, blendFactor, 0xFFFFFFFF);
        context->OMSetRenderTargets(1, &OMTGame::m_pGameInstance->m_pD3DRenderTargetView, nullptr);
        SetRenderViewport(OMTGame::m_pGameInstance->m_windSize.x,OMTGame::m_pGameInstance->m_windSize.y);
        context->VSSetShader(m_pTextureDebugVS, nullptr, 0);
        context->PSSetShader(m_pTextureDebugPS, nullptr, 0);
        context->IASetVertexBuffers(0, 1, &m_pVertexBufferQuad, &stride, &offset);
        context->IASetIndexBuffer(m_pIndexBufferQuad, DXGI_FORMAT_R16_UINT, 0);
    
        m_debugTextureMatrix = XMMatrixTranspose(m_debugTextureMatrix);
        context->UpdateSubresource(m_pModelCB, 0, nullptr, &m_debugTextureMatrix, 0, 0);
    
        context->PSSetShaderResources(0, 1, &m_pRenderTexture1Resource);
        context->DrawIndexed(m_quadModel.indexCount, 0, 0);
    }

    // Present back buffer to display
    OMTGame::m_pGameInstance->m_pSwapChain->Present(0, 0);
}