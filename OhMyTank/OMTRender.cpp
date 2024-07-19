#include "OMTRender.h"
#include "AssetsHelper.h"
#include "Resources.h"
#include "OMTGame.h"
#include "SimpleMath.h"

using RenderingData::BasicVertex, RenderingData::basicInputLayoutDescriptor;

OMTRender::OMTRender()
{
    m_pVertexShader = nullptr;
    m_pPixelShader = nullptr;
    m_pBasicInputLayout = nullptr;
    m_pVertexBufferTank = nullptr;
    m_pIndexBufferTank = nullptr;
    m_pVertexBufferQuad = nullptr;
    m_pIndexBufferQuad = nullptr;
    m_pColorMap = nullptr;
    m_pColorMapSampler = nullptr;
    m_pColorMapResource = nullptr;
    m_pBlendState = nullptr;
    m_pViewCB = nullptr;
    m_pProjCB = nullptr;
    m_pModelCB = nullptr;
    m_assetsHelper = nullptr;
    m_pRenderTexture = nullptr;
    m_pRenderTextureResource = nullptr;
    m_pRenderTextureTargetView = nullptr;
    m_pTextureDebugVS = nullptr;
    m_pRenderTextureVS = nullptr;
    m_pRenderTexturePS = nullptr;
    m_viewMatrix = DirectX::XMMatrixIdentity();
    m_viewMatrix = XMMatrixInverse(nullptr, m_viewMatrix);
    m_projMatrix = DirectX::XMMatrixIdentity();
}

OMTRender::~OMTRender()
{    
    if(m_assetsHelper)
    {
        delete m_assetsHelper;
        m_assetsHelper = nullptr;
    }
}

void OMTRender::CreateCameraMatrix()
{
    m_viewMatrix = DirectX::XMMatrixRotationX(OMTGame::m_gameInstance->m_camRotY);
    m_viewMatrix *= DirectX::XMMatrixRotationY(OMTGame::m_gameInstance->m_camRotX);
    m_viewMatrix *= DirectX::XMMatrixTranslationFromVector(OMTGame::m_gameInstance->m_camPos);
    
    m_viewMatrix = XMMatrixInverse(nullptr, m_viewMatrix);
    
    m_projMatrix = DirectX::XMMatrixPerspectiveFovLH(1.0f, (float)OMTGame::m_gameInstance->m_windSize.x / OMTGame::m_gameInstance->m_windSize.y, 0.05f, 100.0f);
    
    m_viewMatrix = DirectX::XMMatrixTranspose(m_viewMatrix);
    m_projMatrix = DirectX::XMMatrixTranspose(m_projMatrix);
}

bool OMTRender::SetupRenderTexture()
{
    D3D11_TEXTURE2D_DESC desc;
    desc.ArraySize = 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Width = 512;
    desc.Height = 512;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.MipLevels = 1;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    HRESULT hr = OMTGame::m_gameInstance->m_pD3DDevice->CreateTexture2D(&desc, nullptr, &m_pRenderTexture);
    if (FAILED(hr)) {
        ::MessageBox(OMTGame::m_gameInstance->m_hWnd, L"Failed to create destination texture.", L"Error", MB_OK);
        return false;
    }

    hr = OMTGame::m_gameInstance->m_pD3DDevice->CreateShaderResourceView(m_pRenderTexture, nullptr, &m_pRenderTextureResource);
    if (FAILED(hr)) {
        ::MessageBox(OMTGame::m_gameInstance->m_hWnd, L"Failed to create Shader Resource View from destination texture.", L"Error", MB_OK);
        return false;
    }

    // Create the render target view
    hr = OMTGame::m_gameInstance->m_pD3DDevice->CreateRenderTargetView(m_pRenderTexture, nullptr, &m_pRenderTextureTargetView);
    // Check render target view
    if (FAILED(hr)) {
        MessageBox(OMTGame::m_gameInstance->m_hWnd, Utils::GetMessageFromHr(hr), TEXT("Create render target ERROR"), MB_OK);
        return false;
    }
    
    return true;
}

bool OMTRender::LoadGraphicContent()
{
    m_assetsHelper = new AssetsHelper();
    
    ID3DBlob* pShaderBlob = nullptr;
    if(!m_assetsHelper->LoadShader<ID3D11VertexShader>(L"Shaders/VertexShader.hlsl", &pShaderBlob, &m_pVertexShader))
    {
        return false;
    }
    
    if (!m_assetsHelper->CreateInputLayout(basicInputLayoutDescriptor.data(), basicInputLayoutDescriptor.size(), pShaderBlob, &m_pBasicInputLayout))
    {
        return false;
    }
    
    if (!m_assetsHelper->LoadShader<ID3D11PixelShader>(L"Shaders/PixelShader.hlsl", &pShaderBlob, &m_pPixelShader))
    {
        return false;
    }
    
    if (!m_assetsHelper->LoadShader<ID3D11PixelShader>(L"Shaders/TextureDebugPS.hlsl", &pShaderBlob, &m_pTextureDebugPS))
    {
        return false;
    }    
    if(!m_assetsHelper->LoadShader<ID3D11VertexShader>(L"Shaders/TextureDebugVS.hlsl", &pShaderBlob, &m_pTextureDebugVS))
    {
        return false;
    }
    
    if (!m_assetsHelper->LoadShader<ID3D11PixelShader>(L"Shaders/RenderTexturePS.hlsl", &pShaderBlob, &m_pRenderTexturePS))
    {
        return false;
    }    
    if(!m_assetsHelper->LoadShader<ID3D11VertexShader>(L"Shaders/RenderTextureVS.hlsl", &pShaderBlob, &m_pRenderTextureVS))
    {
        return false;
    }

    pShaderBlob->Release();
    pShaderBlob = nullptr;

    FBXImporter modelImporter;
    modelImporter.LoadModel("Models/Tank.fbx");
    m_TankModel = modelImporter.m_model;
    if (!m_assetsHelper->CreateModelBuffers<BasicVertex>(&m_TankModel, &m_pVertexBufferTank, &m_pIndexBufferTank))
    {
        return false;
    }
    modelImporter.LoadModel("Models/Quad.fbx");
    m_QuadModel = modelImporter.m_model;
    if (!m_assetsHelper->CreateModelBuffers<BasicVertex>(&m_QuadModel, &m_pVertexBufferQuad, &m_pIndexBufferQuad))
    {
        return false;
    }
    
    if (!CreateConstantBuffers())
    {
        return false;
    }

    if (!m_assetsHelper->LoadTexture(Textures[Tex::Tank], &m_pColorMap, &m_pColorMapResource))
    {
        return false;
    }
    
    if (!SetupRenderTexture())
    {
        return false;
    }

    SetBlendingMode();

    CreateTextureSampler();

    m_resources.push_back((ID3D11Resource**)&m_pVertexShader);
    m_resources.push_back((ID3D11Resource**)&m_pPixelShader);
    m_resources.push_back((ID3D11Resource**)&m_pBasicInputLayout);
    m_resources.push_back((ID3D11Resource**)&m_pVertexBufferTank);
    m_resources.push_back((ID3D11Resource**)&m_pIndexBufferTank);
    m_resources.push_back((ID3D11Resource**)&m_pVertexBufferQuad);
    m_resources.push_back((ID3D11Resource**)&m_pIndexBufferQuad);
    m_resources.push_back((ID3D11Resource**)&m_pBlendState);
    m_resources.push_back((ID3D11Resource**)&m_pColorMapSampler);
    m_resources.push_back((ID3D11Resource**)&m_pColorMapResource);
    m_resources.push_back(&m_pColorMap);
    m_resources.push_back((ID3D11Resource**)&m_pTextureDebugPS);
    m_resources.push_back((ID3D11Resource**)&m_pViewCB);
    m_resources.push_back((ID3D11Resource**)&m_pProjCB);
    m_resources.push_back((ID3D11Resource**)&m_pModelCB);
    m_resources.push_back((ID3D11Resource**)&m_pRenderTextureResource);
    m_resources.push_back((ID3D11Resource**)&m_pRenderTexture);
    m_resources.push_back((ID3D11Resource**)&m_pRenderTextureTargetView);
    m_resources.push_back((ID3D11Resource**)&m_pTextureDebugVS);
    m_resources.push_back((ID3D11Resource**)&m_pRenderTexturePS);
    m_resources.push_back((ID3D11Resource**)&m_pRenderTextureVS);

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
    HRESULT hr = OMTGame::m_gameInstance->m_pD3DDevice->CreateBuffer(&constDesc, nullptr, &m_pViewCB);
    if (FAILED(hr)) {
        ::MessageBox(OMTGame::m_gameInstance->m_hWnd, Utils::GetMessageFromHr(hr), L"View Constant Buffer Error", MB_OK);
        return false;
    }
    hr = OMTGame::m_gameInstance->m_pD3DDevice->CreateBuffer(&constDesc, nullptr, &m_pProjCB);
    if (FAILED(hr)) {
        ::MessageBox(OMTGame::m_gameInstance->m_hWnd, Utils::GetMessageFromHr(hr), L"Projection Constant Buffer Error", MB_OK);
        return false;
    }
    hr = OMTGame::m_gameInstance->m_pD3DDevice->CreateBuffer(&constDesc, nullptr, &m_pModelCB);
    if (FAILED(hr)) {
        ::MessageBox(OMTGame::m_gameInstance->m_hWnd, Utils::GetMessageFromHr(hr), L"Model Constant Buffer Error", MB_OK);
        return false;
    }
    
    OMTGame::m_gameInstance->m_pD3DContext->VSSetConstantBuffers(0, 1, &m_pModelCB);
    OMTGame::m_gameInstance->m_pD3DContext->VSSetConstantBuffers(1, 1, &m_pViewCB);
    OMTGame::m_gameInstance->m_pD3DContext->VSSetConstantBuffers(2, 1, &m_pProjCB);
    
    return true;
}

void OMTRender::SetBlendingMode()
{
    D3D11_BLEND_DESC blendDesc;
    ::ZeroMemory(&blendDesc, sizeof(blendDesc));
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;//D3D11_BLEND_ONE
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F;
    OMTGame::m_gameInstance->m_pD3DDevice->CreateBlendState(&blendDesc, &m_pBlendState);
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
    HRESULT hr = OMTGame::m_gameInstance->m_pD3DDevice->CreateSamplerState(&textureDesc, &m_pColorMapSampler);
    if (FAILED(hr)) {
        ::MessageBox(OMTGame::m_gameInstance->m_hWnd, Utils::GetMessageFromHr(hr), L"Texture Sampler Error", MB_OK);
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
    OMTGame::m_gameInstance->m_pD3DContext->RSSetViewports(1, &viewPort);
}

void OMTRender::Render()
{
    CreateCameraMatrix();
    
    auto context = OMTGame::m_gameInstance->m_pD3DContext;
    // Check if D3D is ready
    if (context == nullptr)
        return;
    
    // Set stuff
    context->PSSetSamplers(0, 1, &m_pColorMapSampler);
    context->IASetInputLayout(m_pBasicInputLayout);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //context->OMSetRenderTargets(1, &m_pRenderTextureTargetView, nullptr);
    UINT stride = sizeof(BasicVertex);
    UINT offset = 0;

    //World drawcall
    ///////////////////////////
    
    // Clear back buffer
    float bgColor[4] = { 0.2f, 0.2f, 0.3f, 1.0f };
    context->ClearRenderTargetView(OMTGame::m_gameInstance->m_pD3DRenderTargetView, bgColor);
    context->ClearDepthStencilView(OMTGame::m_gameInstance->m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    
    // Set the render target
    context->OMSetRenderTargets(1, &OMTGame::m_gameInstance->m_pD3DRenderTargetView, OMTGame::m_gameInstance->m_pDepthStencilView);
    SetRenderViewport(OMTGame::m_gameInstance->m_windSize.x,OMTGame::m_gameInstance->m_windSize.y);
    
    context->VSSetShader(m_pVertexShader, nullptr, 0);
    context->PSSetShader(m_pPixelShader, nullptr, 0);
    context->IASetVertexBuffers(0, 1, &m_pVertexBufferTank, &stride, &offset);
    context->IASetIndexBuffer(m_pIndexBufferTank, DXGI_FORMAT_R16_UINT, 0);

    auto transposedTankMatrix = XMMatrixTranspose(OMTGame::m_gameInstance->m_tankMatrix);
    
    context->UpdateSubresource(m_pModelCB, 0, nullptr, &transposedTankMatrix, 0, 0);
    context->UpdateSubresource(m_pViewCB, 0, nullptr, &m_viewMatrix, 0, 0);
    context->UpdateSubresource(m_pProjCB, 0, nullptr, &m_projMatrix, 0, 0);
    
    context->PSSetShaderResources(0, 1, &m_pRenderTextureResource);
    context->DrawIndexed(m_TankModel.indexCount, 0, 0);

    //Render Texture drawcall
    //////////////////////////////////
    float textureBG[4] = { 0.3f, 0.5f, 0.1f, 1.0f };
    context->ClearRenderTargetView(m_pRenderTextureTargetView, textureBG);
    context->OMSetRenderTargets(1, &m_pRenderTextureTargetView, nullptr);
    SetRenderViewport(512, 512);
    context->IASetVertexBuffers(0, 1, &m_pVertexBufferQuad, &stride, &offset);
    context->IASetIndexBuffer(m_pIndexBufferQuad, DXGI_FORMAT_R16_UINT, 0);
    auto rtObjectMatrix = DirectX::XMMatrixTranslation(sin(OMTGame::m_gameInstance->m_elapsedTime), 0, 0);
    rtObjectMatrix = XMMatrixTranspose(rtObjectMatrix);
    context->UpdateSubresource(m_pModelCB, 0, nullptr, &rtObjectMatrix, 0, 0);
    context->VSSetShader(m_pRenderTextureVS, nullptr, 0);
    context->PSSetShader(m_pRenderTexturePS, nullptr, 0);
    context->DrawIndexed(m_QuadModel.indexCount, 0, 0);

    //Render Texture debug drawcall
    //////////////////////////////////
    context->OMSetRenderTargets(1, &OMTGame::m_gameInstance->m_pD3DRenderTargetView, nullptr);
    SetRenderViewport(OMTGame::m_gameInstance->m_windSize.x,OMTGame::m_gameInstance->m_windSize.y);
    context->VSSetShader(m_pTextureDebugVS, nullptr, 0);
    context->PSSetShader(m_pTextureDebugPS, nullptr, 0);
    context->IASetVertexBuffers(0, 1, &m_pVertexBufferQuad, &stride, &offset);
    context->IASetIndexBuffer(m_pIndexBufferQuad, DXGI_FORMAT_R16_UINT, 0);
    auto quadMatrix = DirectX::XMMatrixScaling(0.4, 0.4, 0.4);
    quadMatrix *= DirectX::XMMatrixTranslation(0.7, 0.7, 0);
    quadMatrix = XMMatrixTranspose(quadMatrix);
    context->UpdateSubresource(m_pModelCB, 0, nullptr, &quadMatrix, 0, 0);
    
    context->PSSetShaderResources(0, 1, &m_pRenderTextureResource);
    context->DrawIndexed(m_QuadModel.indexCount, 0, 0);

    // Present back buffer to display
    OMTGame::m_gameInstance->m_pSwapChain->Present(0, 0);
}