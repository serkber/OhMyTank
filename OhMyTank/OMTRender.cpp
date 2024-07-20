#include "OMTRender.h"
#include "AssetsHelper.h"
#include "Resources.h"
#include "OMTGame.h"
#include "SimpleMath.h"

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
    m_pRenderTexture = nullptr;
    m_pRenderTextureResource = nullptr;
    m_pRenderTextureTargetView = nullptr;
    m_pTextureDebugVS = nullptr;
    m_pRenderTextureVS = nullptr;
    m_pRenderTexturePS = nullptr;
    m_pGrassFieldVS = nullptr;
    m_pGrassFieldPS = nullptr;
    m_pMultiplyBlendState = nullptr;
    m_pVertexBufferGrassField = nullptr;
    m_pIndexBufferGrassField = nullptr;
    m_viewMatrix = DirectX::XMMatrixIdentity();
    m_viewMatrix = XMMatrixInverse(nullptr, m_viewMatrix);
    m_projMatrix = DirectX::XMMatrixIdentity();
}

OMTRender::~OMTRender()
{
}

void OMTRender::CreateCameraMatrix()
{
    m_viewMatrix = DirectX::XMMatrixRotationX(OMTGame::m_pGameInstance->m_camRotY);
    m_viewMatrix *= DirectX::XMMatrixRotationY(OMTGame::m_pGameInstance->m_camRotX);
    m_viewMatrix *= DirectX::XMMatrixTranslationFromVector(OMTGame::m_pGameInstance->m_camPos);
    
    m_viewMatrix = XMMatrixInverse(nullptr, m_viewMatrix);
    
    m_projMatrix = DirectX::XMMatrixPerspectiveFovLH(1.0f, (float)OMTGame::m_pGameInstance->m_windSize.x / OMTGame::m_pGameInstance->m_windSize.y, 0.05f, 100.0f);
    
    m_viewMatrix = DirectX::XMMatrixTranspose(m_viewMatrix);
    m_projMatrix = DirectX::XMMatrixTranspose(m_projMatrix);
}

#define RENDER_TEXTURE_SIZE 128
bool OMTRender::SetupRenderTexture()
{
    D3D11_TEXTURE2D_DESC desc;
    desc.ArraySize = 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Width = RENDER_TEXTURE_SIZE;
    desc.Height = RENDER_TEXTURE_SIZE;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.MipLevels = 1;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    HRESULT hr = OMTGame::m_pGameInstance->m_pD3DDevice->CreateTexture2D(&desc, nullptr, &m_pRenderTexture);
    if (FAILED(hr)) {
        ::MessageBox(OMTGame::m_pGameInstance->m_hWnd, L"Failed to create destination texture.", L"Error", MB_OK);
        return false;
    }

    hr = OMTGame::m_pGameInstance->m_pD3DDevice->CreateShaderResourceView(m_pRenderTexture, nullptr, &m_pRenderTextureResource);
    if (FAILED(hr)) {
        ::MessageBox(OMTGame::m_pGameInstance->m_hWnd, L"Failed to create Shader Resource View from destination texture.", L"Error", MB_OK);
        return false;
    }

    // Create the render target view
    hr = OMTGame::m_pGameInstance->m_pD3DDevice->CreateRenderTargetView(m_pRenderTexture, nullptr, &m_pRenderTextureTargetView);
    // Check render target view
    if (FAILED(hr)) {
        MessageBox(OMTGame::m_pGameInstance->m_hWnd, Utils::GetMessageFromHr(hr), TEXT("Create render target ERROR"), MB_OK);
        return false;
    }
    
    float bgColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f }; 
    OMTGame::m_pGameInstance->m_pD3DContext->ClearRenderTargetView(m_pRenderTextureTargetView, bgColor);
    
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
    
    if (!CreateConstantBuffers())
    {
        return false;
    }

    if (!m_assetsHelper.LoadTexture(Textures[Tex::Tank], &m_pTankMap, &m_pTankMapResource))
    {
        return false;
    }
    
    if (!SetupRenderTexture())
    {
        return false;
    }

    CreateBlendingResources();

    CreateTextureSampler();

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
    m_resources.push_back((ID3D11Resource**)&m_pRenderTextureResource);
    m_resources.push_back((ID3D11Resource**)&m_pRenderTexture);
    m_resources.push_back((ID3D11Resource**)&m_pRenderTextureTargetView);
    m_resources.push_back((ID3D11Resource**)&m_pTextureDebugVS);
    m_resources.push_back((ID3D11Resource**)&m_pRenderTexturePS);
    m_resources.push_back((ID3D11Resource**)&m_pRenderTextureVS);
    m_resources.push_back((ID3D11Resource**)&m_pGrassFieldVS);
    m_resources.push_back((ID3D11Resource**)&m_pGrassFieldPS);
    m_resources.push_back((ID3D11Resource**)&m_pMultiplyBlendState);
    m_resources.push_back((ID3D11Resource**)&m_pVertexBufferGrassField);
    m_resources.push_back((ID3D11Resource**)&m_pIndexBufferGrassField );

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

#define GRASS_FIELD_SIZE 100

void OMTRender::RenderGrass()
{
    UINT stride = sizeof(BasicVertex);
    UINT offset = 0;
    auto context = OMTGame::m_pGameInstance->m_pD3DContext;
    //Grass drawcall
    ////////////////////////////////////
    context->VSSetShader(m_pGrassFieldVS, nullptr, 0);
    context->PSSetShader(m_pGrassFieldPS, nullptr, 0);
    context->IASetVertexBuffers(0, 1, &m_pVertexBufferGrassField, &stride, &offset);
    context->IASetIndexBuffer(m_pIndexBufferGrassField, DXGI_FORMAT_R16_UINT, 0);
    context->VSSetShaderResources(0, 1, &m_pRenderTextureResource);

    float grassOffset = GRASS_FIELD_SIZE / 2 - 0.5;
    for (int x = 0; x < GRASS_FIELD_SIZE; ++x)
    {
        for (int y = 0; y < GRASS_FIELD_SIZE; ++y)
        {    
            auto grassMatrix = DirectX::XMMatrixScaling(.1, .1, .1);
            grassMatrix *= DirectX::XMMatrixTranslation(x - grassOffset, 0 ,y - grassOffset);
            grassMatrix = DirectX::XMMatrixTranspose(grassMatrix);
            context->UpdateSubresource(m_pModelCB, 0, nullptr, &grassMatrix, 0, 0);
    
            context->DrawIndexed(m_grassFieldModel.indexCount, 0, 0);
        }
    }
}

void OMTRender::Render()
{
    CreateCameraMatrix();
    
    auto context = OMTGame::m_pGameInstance->m_pD3DContext;
    // Check if D3D is ready
    if (context == nullptr)
        return;

    DataCB cb;
    ::ZeroMemory(&cb, sizeof(DataCB));
    cb.elapsedTime = OMTGame::m_pGameInstance->m_elapsedTime;
    cb.mouseX = OMTGame::m_pGameInstance->m_input.m_mousePosNorm.x;
    cb.grassfieldSize = GRASS_FIELD_SIZE;
    context->UpdateSubresource(m_pDataCB, 0, nullptr, &cb, 0, 0);
    
    // Set stuff
    context->PSSetSamplers(0, 1, &m_pTextureSampler);
    context->IASetInputLayout(m_pBasicInputLayout);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    UINT stride = sizeof(BasicVertex);
    UINT offset = 0;

    //Render Texture drawcall
    //////////////////////////////////
    auto debugTexMatrix = DirectX::XMMatrixScaling(0.4, 0.4, 0.4);
    debugTexMatrix *= DirectX::XMMatrixTranslation(0.7, 0.7, 0);
    auto inversDebugTexMatrix = XMMatrixInverse(nullptr, debugTexMatrix);

    if(OMTGame::m_pGameInstance->m_isPainting)
    {
        if(renderTextureTimer < 0.0166666)
        {
            renderTextureTimer += OMTGame::m_pGameInstance->m_deltaTime;
        }
        else
        {
            renderTextureTimer = 0;
        
            float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            context->OMSetBlendState(m_pMultiplyBlendState, blendFactor, 0xFFFFFFFF);
            //context->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
            context->OMSetRenderTargets(1, &m_pRenderTextureTargetView, nullptr);
            context->OMSetDepthStencilState(OMTGame::m_pGameInstance->m_pDepthStencilStateDisabled, 0);
            SetRenderViewport(RENDER_TEXTURE_SIZE, RENDER_TEXTURE_SIZE);
            context->IASetVertexBuffers(0, 1, &m_pVertexBufferQuad, &stride, &offset);
            context->IASetIndexBuffer(m_pIndexBufferQuad, DXGI_FORMAT_R16_UINT, 0);

            auto relativeMouse = DirectX::XMVector2Transform(
                DirectX::XMVectorSet(OMTGame::m_pGameInstance->m_input.m_mousePosNorm.x, OMTGame::m_pGameInstance->m_input.m_mousePosNorm.y, 0, 0),
                inversDebugTexMatrix
                );
        
            auto brushMatrix = DirectX::XMMatrixScaling(0.2, 0.2, 0.2);
            brushMatrix *= DirectX::XMMatrixTranslation(relativeMouse.m128_f32[0] * 2, -relativeMouse.m128_f32[1] * 2, 0);
            brushMatrix = XMMatrixTranspose(brushMatrix);
            context->UpdateSubresource(m_pModelCB, 0, nullptr, &brushMatrix, 0, 0);
            context->VSSetShader(m_pRenderTextureVS, nullptr, 0);
            context->PSSetShader(m_pRenderTexturePS, nullptr, 0);
            context->DrawIndexed(m_quadModel.indexCount, 0, 0);
        }
    }

    // World painting
    ///////////////////////////////
    float bgColor[4] = { 0.2f, 0.2f, 0.15f, 1.0f };
    context->ClearRenderTargetView(OMTGame::m_pGameInstance->m_pD3DRenderTargetView, bgColor);
    context->ClearDepthStencilView(OMTGame::m_pGameInstance->m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    // Set the render target
    context->OMSetRenderTargets(1, &OMTGame::m_pGameInstance->m_pD3DRenderTargetView, OMTGame::m_pGameInstance->m_pDepthStencilView);
    SetRenderViewport(OMTGame::m_pGameInstance->m_windSize.x,OMTGame::m_pGameInstance->m_windSize.y);
    context->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(OMTGame::m_pGameInstance->m_pDepthStencilState, 0);
    //Update camera buffers
    context->UpdateSubresource(m_pViewCB, 0, nullptr, &m_viewMatrix, 0, 0);
    context->UpdateSubresource(m_pProjCB, 0, nullptr, &m_projMatrix, 0, 0);
    
    //Tank drawcall
    ///////////////////////////    
    context->VSSetShader(m_pTankVS, nullptr, 0);
    context->PSSetShader(m_pTankPS, nullptr, 0);
    context->IASetVertexBuffers(0, 1, &m_pVertexBufferTank, &stride, &offset);
    context->IASetIndexBuffer(m_pIndexBufferTank, DXGI_FORMAT_R16_UINT, 0);

    auto transposedTankMatrix = XMMatrixTranspose(OMTGame::m_pGameInstance->m_tankMatrix);
    
    context->UpdateSubresource(m_pModelCB, 0, nullptr, &transposedTankMatrix, 0, 0);
    
    context->PSSetShaderResources(0, 1, &m_pTankMapResource);
    context->DrawIndexed(m_tankModel.indexCount, 0, 0);

    RenderGrass();

    //Render Texture debug drawcall
    //////////////////////////////////
    float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    context->OMSetBlendState(m_pAlphaBlendState, blendFactor, 0xFFFFFFFF);
    context->OMSetRenderTargets(1, &OMTGame::m_pGameInstance->m_pD3DRenderTargetView, nullptr);
    SetRenderViewport(OMTGame::m_pGameInstance->m_windSize.x,OMTGame::m_pGameInstance->m_windSize.y);
    context->VSSetShader(m_pTextureDebugVS, nullptr, 0);
    context->PSSetShader(m_pTextureDebugPS, nullptr, 0);
    context->IASetVertexBuffers(0, 1, &m_pVertexBufferQuad, &stride, &offset);
    context->IASetIndexBuffer(m_pIndexBufferQuad, DXGI_FORMAT_R16_UINT, 0);
    debugTexMatrix = XMMatrixTranspose(debugTexMatrix);
    context->UpdateSubresource(m_pModelCB, 0, nullptr, &debugTexMatrix, 0, 0);
    
    context->PSSetShaderResources(0, 1, &m_pRenderTextureResource);
    context->DrawIndexed(m_quadModel.indexCount, 0, 0);

    // Present back buffer to display
    OMTGame::m_pGameInstance->m_pSwapChain->Present(0, 0);
}