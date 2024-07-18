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
    m_pVertexBuffer = nullptr;
    m_pIndexBuffer = nullptr;
    m_pColorMapResource = nullptr;
    m_pColorMapSampler = nullptr;
    m_pColorMapOne = nullptr;
    m_pBlendState = nullptr;
    m_pViewCB = nullptr;
    m_pProjCB = nullptr;
    m_pModelCB = nullptr;
    m_assetsHelper = nullptr;
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

bool OMTRender::LoadGraphicContent()
{
    m_assetsHelper = new AssetsHelper();

    FBXImporter modelImporter;
    modelImporter.LoadModel("Models/Tank.fbx");
    m_model = modelImporter.m_model;
    
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
    if (!m_assetsHelper->LoadShader<ID3D11PixelShader>(L"Shaders/PixelShaderWire.hlsl", &pShaderBlob, &m_pPixelShaderWire))
    {
        return false;
    }

    pShaderBlob->Release();
    pShaderBlob = nullptr;

    if (!m_assetsHelper->CreateModelBuffers<BasicVertex>(&m_model, &m_pVertexBuffer, &m_pIndexBuffer))
    {
        return false;
    }
    
    if (!CreateConstantBuffers())
    {
        return false;
    }

    if (!m_assetsHelper->LoadTexture(Textures[Tex::Tank], &m_pColorMapResource, &m_pColorMapOne))
    {
        return false;
    }

    SetBlendingMode();

    CreateTextureSampler();

    m_resources.push_back((ID3D11Resource**)&m_pVertexShader);
    m_resources.push_back((ID3D11Resource**)&m_pPixelShader);
    m_resources.push_back((ID3D11Resource**)&m_pBasicInputLayout);
    m_resources.push_back((ID3D11Resource**)&m_pVertexBuffer);
    m_resources.push_back((ID3D11Resource**)&m_pIndexBuffer);
    m_resources.push_back((ID3D11Resource**)&m_pBlendState);
    m_resources.push_back((ID3D11Resource**)&m_pColorMapSampler);
    m_resources.push_back((ID3D11Resource**)&m_pColorMapOne);
    m_resources.push_back(&m_pColorMapResource);
    m_resources.push_back((ID3D11Resource**)&m_pPixelShaderWire);
    m_resources.push_back((ID3D11Resource**)&m_pViewCB);
    m_resources.push_back((ID3D11Resource**)&m_pProjCB);
    m_resources.push_back((ID3D11Resource**)&m_pModelCB);

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

void OMTRender::Render()
{
    CreateCameraMatrix();
    
    auto context = OMTGame::m_gameInstance->m_pD3DContext;
    // Check if D3D is ready
    if (context == nullptr)
        return;

    // Clear back buffer
    float color[4] = { 0.2f, 0.2f, 0.3f, 1.0f };
    context->ClearRenderTargetView(OMTGame::m_gameInstance->m_pD3DRenderTargetView, color);
    context->ClearDepthStencilView(OMTGame::m_gameInstance->m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // Set shaders
    context->VSSetShader(m_pVertexShader, nullptr, 0);
    context->PSSetShader(m_pPixelShader, nullptr, 0);
    context->PSSetSamplers(0, 1, &m_pColorMapSampler);

    // Set stuff
    context->IASetInputLayout(m_pBasicInputLayout);
    float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    context->OMSetBlendState(m_pBlendState, blendFactor, 0xFFFFFFFF);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Set the render target
    context->OMSetRenderTargets(1, &OMTGame::m_gameInstance->m_pD3DRenderTargetView, OMTGame::m_gameInstance->m_pDepthStencilView);
    UINT stride = sizeof(BasicVertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
    context->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    auto transposedModel = XMMatrixTranspose(OMTGame::m_gameInstance->m_modelMatrix);
    context->UpdateSubresource(m_pModelCB, 0, nullptr, &transposedModel, 0, 0);
    
    context->UpdateSubresource(m_pViewCB, 0, nullptr, &m_viewMatrix, 0, 0);
    context->UpdateSubresource(m_pProjCB, 0, nullptr, &m_projMatrix, 0, 0);
    
    context->VSSetConstantBuffers(0, 1, &m_pModelCB);
    context->VSSetConstantBuffers(1, 1, &m_pViewCB);
    context->VSSetConstantBuffers(2, 1, &m_pProjCB);
    
    context->PSSetShaderResources(0, 1, &m_pColorMapOne);
    context->DrawIndexed(m_model.indexCount, 0, 0);

    if(OMTGame::m_gameInstance->m_drawWire)
    {
        context->PSSetShader(m_pPixelShaderWire, nullptr, 0);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        context->DrawIndexed(m_model.indexCount, 0, 0);
    }

    // Present back buffer to display
    OMTGame::m_gameInstance->m_pSwapChain->Present(0, 0);
}