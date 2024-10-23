#include "OMTRender.h"
#include "AssetsHelper.h"
#include "Resources.h"
#include "OMTGame.h"
#include "SimpleMath.h"
#include "RenderingData.h"

using RenderingData::BasicVertex, RenderingData::basicInputLayoutDescriptor;

OMTRender::OMTRender()
{
    m_pBasicVS = nullptr;
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
    m_pCustomBlendState = nullptr;
    m_pVertexBufferGrassPatch = nullptr;
    m_pIndexBufferGrassPatch = nullptr;
    m_pFullScreenVS = nullptr;
    m_pInstantiableInputLayout = nullptr;
    m_pInstancesBufferGrassPatch = nullptr;
    m_pSilhouettePS = nullptr;
    m_pBlurPS = nullptr;
    m_pFXAAPS = nullptr;
    m_pPostpoTexture2 = nullptr;
    m_pPostpoTexture2Resource = nullptr;
    m_pTextureScrollPS = nullptr;
    m_pTextureScrollVS = nullptr;
    m_pGroundPS = nullptr;
    m_pRasterStateTwoSided = nullptr;
    m_pClearChannelsPS = nullptr;
    m_pRenderTextureDirPS = nullptr;
    
    ::ZeroMemory(&m_data, sizeof(DataCB));
}

OMTRender::~OMTRender()
{
}

bool OMTRender::SetupRenderTexture(ID3D11Texture2D** renderTexture, ID3D11ShaderResourceView** resourceView, ID3D11RenderTargetView** renderTarget)
{
    D3D11_TEXTURE2D_DESC desc;
    desc.ArraySize = 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Width = RENDER_TEXTURE_SIZE;
    desc.Height = RENDER_TEXTURE_SIZE;
    desc.Format = DXGI_FORMAT_R8G8B8A8_SNORM;
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
    
    InitGrass(OMTGame::m_pGameInstance->m_pD3DContext, renderTarget);

    return true;
}

void OMTRender::InitGrass(ID3D11DeviceContext* context, ID3D11RenderTargetView** renderTarget)
{
    float bgColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
    OMTGame::m_pGameInstance->m_pD3DContext->ClearRenderTargetView(*renderTarget, bgColor);
    
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    SetRenderViewport(RENDER_TEXTURE_SIZE, RENDER_TEXTURE_SIZE);
    context->IASetInputLayout(m_pBasicInputLayout);  
    context->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
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

bool OMTRender::CreateTwoSidedRasterState()
{
    D3D11_RASTERIZER_DESC rasterDesc;
    ZeroMemory(&rasterDesc, sizeof(D3D11_RASTERIZER_DESC));
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_NONE;
    rasterDesc.FrontCounterClockwise = FALSE;
    rasterDesc.DepthClipEnable = TRUE;

    HRESULT hr = OMTGame::m_pGameInstance->m_pD3DDevice->CreateRasterizerState(&rasterDesc, &m_pRasterStateTwoSided);
    if (FAILED(hr))
    {
        ::MessageBox(OMTGame::m_pGameInstance->m_hWnd, L"Failed to create Shader Resource View of texture.", L"Error", MB_OK);
        return false;
    }
    return true;
}

bool OMTRender::LoadGraphicContent()
{
    ID3DBlob* pShaderBlob = nullptr;
    if(!m_assetsHelper.LoadShader<ID3D11VertexShader>(L"Shaders/BasicVS.hlsl", &pShaderBlob, &m_pBasicVS))
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
    
    if (!m_assetsHelper.LoadShader<ID3D11PixelShader>(L"Shaders/GroundPS.hlsl", &pShaderBlob, &m_pGroundPS))
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
    if(!m_assetsHelper.LoadShader<ID3D11PixelShader>(L"Shaders/TextureScrollPS.hlsl", &pShaderBlob, &m_pTextureScrollPS))
    {
        return false;
    }
    if(!m_assetsHelper.LoadShader<ID3D11PixelShader>(L"Shaders/ClearChannelsPS.hlsl", &pShaderBlob, &m_pClearChannelsPS))
    {
        return false;
    }
    if(!m_assetsHelper.LoadShader<ID3D11VertexShader>(L"Shaders/TextureScrollVS.hlsl", &pShaderBlob, &m_pTextureScrollVS))
    {
        return false;
    }
    if(!m_assetsHelper.LoadShader<ID3D11PixelShader>(L"Shaders/RenderTextureDirPS.hlsl", &pShaderBlob, &m_pRenderTextureDirPS))
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
    modelImporter.LoadModel("Models/GrassPatch.fbx");
    m_grassPatchModel = modelImporter.m_model;
    if (!m_assetsHelper.CreateModelBuffers<BasicVertex>(&m_grassPatchModel, &m_pVertexBufferGrassPatch, &m_pIndexBufferGrassPatch))
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

    if (!CreateTwoSidedRasterState())
    {
        return false;
    }
    
    m_pGrassBuffers[0] = m_pVertexBufferGrassPatch;
    m_pGrassBuffers[1] = m_pInstancesBufferGrassPatch;

    m_resources.push_back((ID3D11Resource**)&m_pBasicVS);
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
    m_resources.push_back((ID3D11Resource**)&m_pCustomBlendState);
    m_resources.push_back((ID3D11Resource**)&m_pVertexBufferGrassPatch);
    m_resources.push_back((ID3D11Resource**)&m_pIndexBufferGrassPatch);
    m_resources.push_back((ID3D11Resource**)&m_pGrassInitPS);
    m_resources.push_back((ID3D11Resource**)&m_pFullScreenVS);
    m_resources.push_back((ID3D11Resource**)&m_pInstantiableInputLayout);
    m_resources.push_back((ID3D11Resource**)&m_pInstancesBufferGrassPatch);
    m_resources.push_back((ID3D11Resource**)&m_pPostpoTexture1Resource);
    m_resources.push_back((ID3D11Resource**)&m_pPostpoTexture1);
    m_resources.push_back((ID3D11Resource**)&m_pSilhouettePS);
    m_resources.push_back((ID3D11Resource**)&m_pBlurPS);
    m_resources.push_back((ID3D11Resource**)&m_pFXAAPS);
    m_resources.push_back((ID3D11Resource**)&m_pPostpoTexture2);
    m_resources.push_back((ID3D11Resource**)&m_pPostpoTexture2Resource);
    m_resources.push_back((ID3D11Resource**)&m_pTextureScrollPS);
    m_resources.push_back((ID3D11Resource**)&m_pTextureScrollVS);
    m_resources.push_back((ID3D11Resource**)&m_pGroundPS);
    m_resources.push_back((ID3D11Resource**)&m_pRasterStateTwoSided);
    m_resources.push_back((ID3D11Resource**)&m_pClearChannelsPS);
    m_resources.push_back((ID3D11Resource**)&m_pRenderTextureDirPS);

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
    
    OMTGame::m_pGameInstance->m_pD3DContext->VSSetConstantBuffers(0, 1, &m_pDataCB);
    OMTGame::m_pGameInstance->m_pD3DContext->PSSetConstantBuffers(0, 1, &m_pDataCB);
    OMTGame::m_pGameInstance->m_pD3DContext->VSSetConstantBuffers(1, 1, &m_pModelCB);
    OMTGame::m_pGameInstance->m_pD3DContext->VSSetConstantBuffers(2, 1, &m_pViewCB);
    OMTGame::m_pGameInstance->m_pD3DContext->VSSetConstantBuffers(3, 1, &m_pProjCB);
    
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

    // Custom Blend
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_BLEND_FACTOR;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_BLEND_FACTOR;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_BLEND_FACTOR;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_BLEND_FACTOR;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    OMTGame::m_pGameInstance->m_pD3DDevice->CreateBlendState(&blendDesc, &m_pCustomBlendState);
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
    m_debugTextureMatrix = DirectX::XMMatrixScaling(1.8, 1.8, 1.8);
    m_debugTextureMatrix *= DirectX::XMMatrixTranslation(0.0, 0.0, 0);
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
    instanceDesc.ByteWidth = sizeof(RenderingData::InstanceData) * GRASS_SECTION_SIZE * GRASS_SECTION_SIZE;

    // Create particles buffer
    hr = OMTGame::m_pGameInstance->m_pD3DDevice->CreateBuffer(&instanceDesc, nullptr, &m_pInstancesBufferGrassPatch);
    if (FAILED(hr)) {
        ::MessageBox(OMTGame::m_pGameInstance->m_hWnd, Utils::GetMessageFromHr(hr), L"Particles Buffer Error", MB_OK);
        return false;
    }

    return true;
}

void OMTRender::RenderGrassSection(ID3D11DeviceContext* context, DirectX::XMVECTOR sectionPosition)
{    
    auto offset = DirectX::XMVectorSet(GRASS_SECTION_SIZE / 2 - 0.5, 0, GRASS_SECTION_SIZE / 2 - 0.5, 0);
    offset = DirectX::XMVectorSubtract(sectionPosition, offset);
    UINT instanceDataCounter = 0;
    RenderingData::InstanceData instancesData[GRASS_SECTION_SIZE * GRASS_SECTION_SIZE];
    
    for (int x = 0; x < GRASS_SECTION_SIZE; ++x)
    {
        for (int y = 0; y < GRASS_SECTION_SIZE; ++y)
        {
            auto patchPos = DirectX::XMVectorSet(x, 0, y, 0);
            patchPos = DirectX::XMVectorAdd(patchPos, offset);
            auto grassPatch = DirectX::XMMatrixScaling(.1, .1, .1);
            grassPatch *= DirectX::XMMatrixTranslationFromVector(patchPos);
            grassPatch = DirectX::XMMatrixTranspose(grassPatch);    
            instancesData[instanceDataCounter].model = grassPatch;
            instanceDataCounter ++;
        }
    }
    
    context->UpdateSubresource(m_pInstancesBufferGrassPatch, 0, 0, instancesData, 0, 0);
    context->DrawIndexedInstanced(m_grassPatchModel.indexCount, GRASS_SECTION_SIZE * GRASS_SECTION_SIZE, 0, 0, 0);
}

void OMTRender::RenderGrassField(ID3D11DeviceContext* context)
{
    double spins = 0;
    double transformedCamPitch = OMTGame::m_pGameInstance->m_camRotPitch / PI / 2.f + 1.f / 16.f;

    transformedCamPitch = modf(transformedCamPitch, &spins);
    if(transformedCamPitch < 0)
    {
        transformedCamPitch += 1;
    }
    transformedCamPitch *= 8;
    auto visibleSectionsIndex = static_cast<int>(floor(transformedCamPitch));

    VisibleGrassSections visibleSectionsByTankRot[]
    {
        0, 3, 6,
        3, 6, 7,
        6, 7, 8,
        7, 8, 5,
        8, 5, 2,
        5, 1, 2,
        0, 1, 2,
        1, 0, 3
    };    
    VisibleGrassSections visibleSection = visibleSectionsByTankRot[visibleSectionsIndex];

    UINT strides[2] = { sizeof(BasicVertex), sizeof(RenderingData::InstanceData) };
    UINT offsets[2] = { 0, 0 };
    
    context->IASetInputLayout(m_pInstantiableInputLayout);
    context->VSSetShader(m_pGrassFieldVS, nullptr, 0);
    context->VSSetShaderResources(0, 1, &m_pRenderTexture1Resource);
    context->PSSetShader(m_pGrassFieldPS, nullptr, 0);
    context->IASetVertexBuffers(0, 2, m_pGrassBuffers, strides, offsets);
    context->IASetIndexBuffer(m_pIndexBufferGrassPatch, DXGI_FORMAT_R16_UINT, 0);
    context->RSSetState(m_pRasterStateTwoSided);

    UINT sectionIndex = 0;
    for (int col = 0; col < 3; ++col)
    {
        for (int row = 0; row < 3; ++row)
        {
            if(!CheckGrassSectionVisibility(visibleSection, sectionIndex) && sectionIndex != 4)
            {
                ++sectionIndex;
                continue;
            }
            auto offset = DirectX::XMVectorSet(-1, 0, 1, 0);
            auto sectionPosition = DirectX::XMVectorSet(col, 0, -row, 0);
            sectionPosition = DirectX::XMVectorAdd(sectionPosition, offset);
            
            sectionPosition = DirectX::XMVectorScale(sectionPosition, GRASS_SECTION_SIZE);
            sectionPosition = DirectX::XMVectorAdd(sectionPosition, OMTGame::m_pGameInstance->m_grassPos);
    
            RenderGrassSection(context, sectionPosition);
            ++sectionIndex;
        }
    }
    
    context->RSSetState(nullptr);
}

bool OMTRender::CheckGrassSectionVisibility(VisibleGrassSections visibleSections, int currentSection)
{
    bool isVisible = false;
    
    //std::cout << visibleSections.sections[0] << ' ' << visibleSections.sections[1] << ' ' << visibleSections.sections[2] << std::endl;

    for (int i = 0; i < 3; ++i)
    {
        if(visibleSections.sections[i] == currentSection)
        {
            isVisible = true;
            break;
        } 
    }
    return isVisible;
}

void OMTRender::UpdateCameraBuffers(ID3D11DeviceContext* context)
{
    m_viewMatrix = DirectX::XMMatrixInverse(nullptr, OMTGame::m_pGameInstance->m_cameraMatrix);
    m_viewMatrix = DirectX::XMMatrixTranspose(m_viewMatrix);    
    m_projMatrix = DirectX::XMMatrixPerspectiveFovLH(1.0f, (float)OMTGame::m_pGameInstance->m_windSize.x / OMTGame::m_pGameInstance->m_windSize.y, 0.1f, 500.0f);
    m_projMatrix = DirectX::XMMatrixTranspose(m_projMatrix);
}

void OMTRender::RenderTexture(ID3D11DeviceContext* context, matrix* grassMatrix, bool* isScrollFrame)
{
    UINT stride = sizeof(BasicVertex);
    UINT offset = 0;

    context->OMSetBlendState(m_pAlphaBlendState, nullptr, 0xFFFFFFFF);
    context->OMSetRenderTargets(1, &m_pRenderTexture1TargetView, nullptr);
    context->OMSetDepthStencilState(OMTGame::m_pGameInstance->m_pDepthStencilStateDisabled, 0);
    SetRenderViewport(RENDER_TEXTURE_SIZE, RENDER_TEXTURE_SIZE);    
    context->IASetVertexBuffers(0, 1, &m_pVertexBufferQuad, &stride, &offset);
    context->IASetIndexBuffer(m_pIndexBufferQuad, DXGI_FORMAT_R16_UINT, 0);
    
    auto inverseGrassMatrix = DirectX::XMMatrixInverse(nullptr, *grassMatrix);
    auto brushPos = DirectX::XMVector3Transform(OMTGame::m_pGameInstance->m_tankPos, inverseGrassMatrix);
    brushPos = DirectX::XMVectorScale(brushPos, 2);
    float brushWidth = 4.f;

    auto currentX = DirectX::XMVectorGetX(brushPos);
    auto currentY = DirectX::XMVectorGetY(brushPos);
    DirectX::XMVECTOR hDelta = DirectX::XMVectorSet(1, 0, 0, 0);
    DirectX::XMVECTOR vDelta = DirectX::XMVectorSet(0, 1, 0, 0);
    DirectX::XMVECTOR grassOffset = DirectX::XMVectorSet(0, 0, 0, 0);
    
    if(currentX > m_maxDeltaBeforeScroll)
    {
        grassOffset = DirectX::XMVectorAdd(grassOffset, hDelta);
        *isScrollFrame = true;
    }
    else if(currentX < -m_maxDeltaBeforeScroll)
    {
        grassOffset = DirectX::XMVectorSubtract(grassOffset, hDelta);
        *isScrollFrame = true;
    }
    if(currentY > m_maxDeltaBeforeScroll)
    {
        grassOffset = DirectX::XMVectorAdd(grassOffset, vDelta);
        *isScrollFrame = true;
    }
    else if(currentY < -m_maxDeltaBeforeScroll)
    {
        grassOffset = DirectX::XMVectorSubtract(grassOffset, vDelta);
        *isScrollFrame = true;
    }

    OMTGame::m_pGameInstance->m_nextGrassFieldScroll = grassOffset;
    
    if(*isScrollFrame)
    {
        //Copy Texture1 to Texture 2
        // context->OMSetRenderTargets(1, &m_pRenderTexture2TargetView, nullptr);
        // context->PSSetShaderResources(0, 1, &m_pRenderTexture1Resource);
        // context->VSSetShader(m_pFullScreenVS, nullptr, 0);
        // context->PSSetShader(m_pTextureDebugPS, nullptr, 0);
        // context->DrawIndexed(m_quadModel.indexCount, 0, 0);        
        // context->OMSetRenderTargets(1, &m_pRenderTexture1TargetView, nullptr);
        
        context->CopyResource(m_pRenderTexture2, m_pRenderTexture1);        
        
        context->VSSetShader(m_pFullScreenVS, nullptr, 0);
        context->PSSetShader(m_pGrassInitPS, nullptr, 0);
        context->DrawIndexed(m_quadModel.indexCount, 0, 0);
        
        context->VSSetShader(m_pTextureScrollVS, nullptr, 0);
        context->PSSetShader(m_pTextureScrollPS, nullptr, 0);
        
        float y = DirectX::XMVectorGetY(grassOffset);
        auto scrollTexOffset = DirectX::XMVectorSet(DirectX::XMVectorGetX(grassOffset), -DirectX::XMVectorGetY(grassOffset), 0, 0);
        scrollTexOffset = DirectX::XMVectorScale(scrollTexOffset, -m_maxDeltaBeforeScroll * 2);
        auto scrolledTexMatrix = DirectX::XMMatrixTranslationFromVector(scrollTexOffset);
        scrolledTexMatrix = DirectX::XMMatrixTranspose(scrolledTexMatrix);
        context->UpdateSubresource(m_pModelCB, 0, nullptr, & scrolledTexMatrix, 0, 0);
        context->PSSetShaderResources(0, 1, &m_pRenderTexture2Resource);
        context->DrawIndexed(m_quadModel.indexCount, 0, 0);
        
        grassOffset = DirectX::XMVectorScale(grassOffset, GRASS_FIELD_SIZE * m_maxDeltaBeforeScroll);
        grassOffset = DirectX::XMVectorSet(DirectX::XMVectorGetX(grassOffset), 0, DirectX::XMVectorGetY(grassOffset), 0);
        
        OMTGame::m_pGameInstance->m_grassPos = DirectX::XMVectorAdd(OMTGame::m_pGameInstance->m_grassPos, grassOffset);
        m_data.d_grassfieldPos = float2(DirectX::XMVectorGetX(OMTGame::m_pGameInstance->m_grassPos), DirectX::XMVectorGetZ(OMTGame::m_pGameInstance->m_grassPos));
        context->UpdateSubresource(m_pDataCB, 0, nullptr, &m_data, 0, 0);
        
        auto newGrassMatrix = DirectX::XMMatrixTranslationFromVector(OMTGame::m_pGameInstance->m_grassPos);
        newGrassMatrix = DirectX::XMMatrixInverse(nullptr, newGrassMatrix);
        brushPos = DirectX::XMVector3Transform(OMTGame::m_pGameInstance->m_tankPos, newGrassMatrix);
        brushPos = DirectX::XMVectorScale(brushPos, 2);
    }

    //Clear Blue and Green channels
    float blendFactor[4] = {0.f, 1.f, 1.f, 1.f};
    context->OMSetBlendState(m_pCustomBlendState, blendFactor, 0xFFFFFFFF);
    context->VSSetShader(m_pFullScreenVS, nullptr, 0);
    context->PSSetShader(m_pClearChannelsPS, nullptr, 0);
    context->DrawIndexed(m_quadModel.indexCount, 0, 0);    
    
    context->OMSetBlendState(m_pAlphaBlendState, nullptr, 0xFFFFFFFF);
    context->VSSetShader(m_pRenderTextureVS, nullptr, 0);
    context->PSSetShader(m_pRenderTexturePS, nullptr, 0);
    auto scalingFactor = brushWidth / GRASS_FIELD_SIZE;
    auto brushMatrix = DirectX::XMMatrixScaling(scalingFactor, scalingFactor, scalingFactor);
    brushMatrix *= DirectX::XMMatrixRotationZ(OMTGame::m_pGameInstance->m_tankRot);
    brushMatrix *= DirectX::XMMatrixTranslation(DirectX::XMVectorGetX(brushPos), -DirectX::XMVectorGetY(brushPos), 0);
    brushMatrix = XMMatrixTranspose(brushMatrix);
    context->UpdateSubresource(m_pModelCB, 0, nullptr, &brushMatrix, 0, 0);
    context->DrawIndexed(m_quadModel.indexCount, 0, 0);

    context->OMSetBlendState(m_pCustomBlendState, blendFactor, 0xFFFFFFFF);
    brushMatrix = DirectX::XMMatrixScaling(scalingFactor, scalingFactor, scalingFactor);
    brushMatrix *= DirectX::XMMatrixScaling(1.f, 2.5f, 1.f);
    brushMatrix *= DirectX::XMMatrixTranslation(0.f, 0.f, 0);
    brushMatrix *= DirectX::XMMatrixRotationZ(OMTGame::m_pGameInstance->m_tankRot);
    brushMatrix *= DirectX::XMMatrixTranslation(DirectX::XMVectorGetX(brushPos), -DirectX::XMVectorGetY(brushPos), 0);
    brushMatrix = XMMatrixTranspose(brushMatrix);
    context->UpdateSubresource(m_pModelCB, 0, nullptr, &brushMatrix, 0, 0);
    context->PSSetShader(m_pRenderTextureDirPS, nullptr, 0);
    context->DrawIndexed(m_quadModel.indexCount, 0, 0);
}

DirectX::XMMATRIX OMTRender::GetGrassfieldMatrix()
{
    auto grassFieldMatrix = DirectX::XMMatrixScaling(GRASS_FIELD_SIZE, GRASS_FIELD_SIZE, GRASS_FIELD_SIZE);
    grassFieldMatrix *= DirectX::XMMatrixRotationX(HALF_PI);
    grassFieldMatrix *= DirectX::XMMatrixTranslationFromVector(OMTGame::m_pGameInstance->m_grassPos);
    return grassFieldMatrix;
}

void OMTRender::Render()
{
    auto context = OMTGame::m_pGameInstance->m_pD3DContext;
    // Check if D3D is ready
    if (context == nullptr)
    {
        return;
    }
    
    UINT stride = sizeof(BasicVertex);
    UINT offset = 0;
    
    CreateDebugTextureMatrix();    
    
    //Update camera buffers
    UpdateCameraBuffers(context);
    context->UpdateSubresource(m_pViewCB, 0, nullptr, &m_viewMatrix, 0, 0);
    context->UpdateSubresource(m_pProjCB, 0, nullptr, &m_projMatrix, 0, 0);

    //Update data buffer
    m_data.d_elapsedTime = OMTGame::m_pGameInstance->m_elapsedTime;
    m_data.d_mousePos = OMTGame::m_pGameInstance->m_input.m_mousePosNorm;
    m_data.d_grassfieldSize = GRASS_FIELD_SIZE;
    m_data.d_grassfieldPos = float2(DirectX::XMVectorGetX(OMTGame::m_pGameInstance->m_grassPos), DirectX::XMVectorGetZ(OMTGame::m_pGameInstance->m_grassPos));
    m_data.d_tankPos = float2(DirectX::XMVectorGetX(OMTGame::m_pGameInstance->m_tankPos), DirectX::XMVectorGetZ(OMTGame::m_pGameInstance->m_tankPos));
    m_data.d_tankDir = float2(DirectX::XMVectorGetX(OMTGame::m_pGameInstance->m_tankDir), DirectX::XMVectorGetZ(OMTGame::m_pGameInstance->m_tankDir));
    context->UpdateSubresource(m_pDataCB, 0, nullptr, &m_data, 0, 0);
    // std::cout << m_data.mouseX << ' ' << m_data.mouseY << std::endl;
    
    // Set stuff
    context->PSSetSamplers(0, 1, &m_pTextureSampler);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //Render Grass Texture
    //////////////////////////////
    auto grassFieldMatrix = GetGrassfieldMatrix();
    bool isScrollFrame = false;
    if(renderTextureTimer < 0.01666)
    {
        renderTextureTimer += OMTGame::m_pGameInstance->m_deltaTime;
    }
    else
    {
        renderTextureTimer = 0;
        RenderTexture(context, &grassFieldMatrix, &isScrollFrame);
    }
    if(isScrollFrame)
    {
        grassFieldMatrix = GetGrassfieldMatrix();
    }
    /////////////////////////////
    
    // World painting settings
    /////////////////////////////// 
    float bgColor[4] = { 0.2f, 0.2f, 0.15f, 0.0f };
    context->ClearRenderTargetView(OMTGame::m_pGameInstance->m_pD3DRenderTargetView, bgColor);
    context->ClearDepthStencilView(OMTGame::m_pGameInstance->m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &OMTGame::m_pGameInstance->m_pD3DRenderTargetView, OMTGame::m_pGameInstance->m_pDepthStencilView);
    SetRenderViewport(OMTGame::m_pGameInstance->m_windSize.x,OMTGame::m_pGameInstance->m_windSize.y);
    context->OMSetDepthStencilState(OMTGame::m_pGameInstance->m_pDepthStencilState, 0);
    context->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
    ////////////////////////////////
    
    // Render Grass
    ///////////////////////////
    RenderGrassField(context);
    ///////////////////////////

    context->IASetInputLayout(m_pBasicInputLayout);
    context->VSSetShader(m_pBasicVS, nullptr, 0);

    // Render Ground
    ////////////////////////
    auto transposedGrassMatrix = DirectX::XMMatrixTranspose(grassFieldMatrix);
    context->UpdateSubresource(m_pModelCB, 0, nullptr, &transposedGrassMatrix, 0, 0);
    context->PSSetShader(m_pGroundPS, nullptr, 0);
    context->PSSetShaderResources(0, 1, &m_pRenderTexture1Resource);
    context->IASetVertexBuffers(0, 1, &m_pVertexBufferQuad, &stride, &offset);
    context->IASetIndexBuffer(m_pIndexBufferQuad, DXGI_FORMAT_R16_UINT, 0);
    context->DrawIndexed(m_quadModel.indexCount, 0, 0);
    ///////////////////////
    
    // Render tank
    ///////////////////////////   
    context->PSSetShader(m_pTankPS, nullptr, 0);
    context->IASetVertexBuffers(0, 1, &m_pVertexBufferTank, &stride, &offset);
    context->IASetIndexBuffer(m_pIndexBufferTank, DXGI_FORMAT_R16_UINT, 0);
    
    auto transposedTankMatrix = XMMatrixTranspose(OMTGame::m_pGameInstance->m_tankMatrix);
    context->UpdateSubresource(m_pModelCB, 0, nullptr, &transposedTankMatrix, 0, 0);
    
    context->PSSetShaderResources(0, 1, &m_pTankMapResource);
    context->DrawIndexed(m_tankModel.indexCount, 0, 0);
    //////////////////

    // if(OMTGame::m_pGameInstance->m_isFXAAEnabled)
    // {
    //     // FXAA
    //     /////////////////////
    //     context->IASetInputLayout(m_pBasicInputLayout);
    //     context->OMSetRenderTargets(1, &OMTGame::m_pGameInstance->m_pD3DRenderTargetView, OMTGame::m_pGameInstance->m_pDepthStencilView);
    //     context->OMSetDepthStencilState(OMTGame::m_pGameInstance->m_pDepthStencilStateDisabled, 0);
    //     context->IASetVertexBuffers(0, 1, &m_pVertexBufferQuad, &stride, &offset);
    //     context->IASetIndexBuffer(m_pIndexBufferQuad, DXGI_FORMAT_R16_UINT, 0);
    //     context->VSSetShader(m_pFullScreenVS, nullptr, 0);
    //
    //     //Silhouette to backBuffer and backBuffer to Texture1
    //     context->PSSetShader(m_pSilhouettePS, nullptr, 0);
    //     context->CopyResource(m_pPostpoTexture1, OMTGame::m_pGameInstance->m_pBackBuffer);
    //     context->PSSetShaderResources(0, 1, &m_pPostpoTexture1Resource);
    //     context->DrawIndexed(m_quadModel.indexCount, 0, 0);
    //
    //     //Silhouette to Texture1 and blurred Silhouette to backBuffer
    //     context->CopyResource(m_pPostpoTexture2, OMTGame::m_pGameInstance->m_pBackBuffer);
    //     context->PSSetShaderResources(0, 1, &m_pPostpoTexture2Resource);
    //     context->PSSetShader(m_pBlurPS, nullptr, 0);
    //     context->DrawIndexed(m_quadModel.indexCount, 0, 0);
    //
    //     //Blurred silhouette to Texture2
    //     context->PSSetShader(m_pFXAAPS, nullptr, 0);
    //     context->CopyResource(m_pPostpoTexture2, OMTGame::m_pGameInstance->m_pBackBuffer);    
    //     context->PSSetShaderResources(1, 1, &m_pPostpoTexture1Resource);
    //     context->DrawIndexed(m_quadModel.indexCount, 0, 0);
    //     ////////////////////
    // }
    
    if(OMTGame::m_pGameInstance->m_isDebugEnabled)
    {
        //Render Texture debug drawcall
        //////////////////////////////////
        context->IASetInputLayout(m_pBasicInputLayout);
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
