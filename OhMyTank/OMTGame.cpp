#include "OMTGame.h"
#include "Utils.h"
#include "DDSTextureLoader.h"
#include <DirectXMath.h>
#include <iostream>
#include "FBXImporter.h"
#include <Audio.h>
#include "Resources.h"
#include "AssetsHelper.h"
#include "WaveBank.h"

using RenderingData::BasicVertex, RenderingData::basicInputLayoutDescriptor;

OMTGame::OMTGame()
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
    m_mousePos = new POINT;
    m_mousePosNorm = float2(0.0f, 0.0f);
    m_viewMatrix = DirectX::XMMatrixIdentity();
    m_projMatrix = DirectX::XMMatrixIdentity();
    m_modelMatrix = DirectX::XMMatrixIdentity();
}

void OMTGame::CreateCameraMatrix()
{
    m_viewMatrix = DirectX::XMMatrixIdentity();
    m_viewMatrix = XMMatrixInverse(nullptr, m_viewMatrix);
    
    m_projMatrix = DirectX::XMMatrixPerspectiveFovLH(1.0f, m_windSize.x / m_windSize.y, 0.05f, 100.0f);
}

void OMTGame::SetBlendingMode()
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
    m_pD3DDevice->CreateBlendState(&blendDesc, &m_pBlendState);
}

bool OMTGame::CreateTextureSampler()
{
    D3D11_SAMPLER_DESC textureDesc;
    ::ZeroMemory(&textureDesc, sizeof(textureDesc));
    textureDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    textureDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    textureDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    textureDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    textureDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    textureDesc.MaxLOD = D3D11_FLOAT32_MAX;
    HRESULT hr = m_pD3DDevice->CreateSamplerState(&textureDesc, &m_pColorMapSampler);
    if (FAILED(hr)) {
        ::MessageBox(m_hWnd, Utils::GetMessageFromHr(hr), L"Texture Sampler Error", MB_OK);
        return false;
    }

    return true;
}

bool OMTGame::CreateConstantBuffers()
{
    // Create world matrix buffer
    D3D11_BUFFER_DESC constDesc;
    ::ZeroMemory(&constDesc, sizeof(constDesc));
    constDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constDesc.ByteWidth = sizeof(matrix);
    constDesc.Usage = D3D11_USAGE_DEFAULT;
    HRESULT hr = m_pD3DDevice->CreateBuffer(&constDesc, nullptr, &m_pViewCB);
    if (FAILED(hr)) {
        ::MessageBox(m_hWnd, Utils::GetMessageFromHr(hr), L"View Constant Buffer Error", MB_OK);
        return false;
    }
    hr = m_pD3DDevice->CreateBuffer(&constDesc, nullptr, &m_pProjCB);
    if (FAILED(hr)) {
        ::MessageBox(m_hWnd, Utils::GetMessageFromHr(hr), L"Projection Constant Buffer Error", MB_OK);
        return false;
    }
    hr = m_pD3DDevice->CreateBuffer(&constDesc, nullptr, &m_pModelCB);
    if (FAILED(hr)) {
        ::MessageBox(m_hWnd, Utils::GetMessageFromHr(hr), L"Model Constant Buffer Error", MB_OK);
        return false;
    }
    
    m_viewMatrix = DirectX::XMMatrixTranspose(m_viewMatrix);
    m_projMatrix = DirectX::XMMatrixTranspose(m_projMatrix);
    
    m_pD3DContext->UpdateSubresource(m_pViewCB, 0, nullptr, &m_viewMatrix, 0, 0);
    m_pD3DContext->UpdateSubresource(m_pProjCB, 0, nullptr, &m_projMatrix, 0, 0);
    return true;
}

bool OMTGame::CreateIndexBuffer()
{
    // Index buffer data
    WORD* indices = new WORD[m_model.indexCount];

    for (int i = 0; i < m_model.indexCount; ++i)
    {
        indices[i] = m_model.indices[i];
    }
    
    D3D11_SUBRESOURCE_DATA resourceData;
    ZeroMemory(&resourceData, sizeof(resourceData));

    // Create index buffer
    D3D11_BUFFER_DESC indexBufferDesc;
    ::ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.ByteWidth = sizeof( WORD ) * m_model.indexCount;
    indexBufferDesc.CPUAccessFlags = 0;
    resourceData.pSysMem = indices;
    HRESULT hr = m_pD3DDevice->CreateBuffer(&indexBufferDesc, &resourceData, &m_pIndexBuffer);

    delete indices;
    
    if (FAILED(hr)) {
        ::MessageBox(m_hWnd, Utils::GetMessageFromHr(hr), L"Index Buffer Error", MB_OK);
        return false;
    }
    return true;
}

// Overrides
bool OMTGame::LoadContent()
{
    m_rndBang = std::uniform_int_distribution<int>(0, 3);

    if(!m_waveBank)
    {
        m_waveBank = std::make_unique<DirectX::WaveBank>(m_audEngine, L"XACT/Win/WaveBank.xwb");
        m_ambientMusic = m_waveBank->CreateInstance( XACT_WAVEBANK_WAVEBANK_SPACESHIP_THEME );
        if ( m_ambientMusic )
        {
            m_ambientMusic->Play(true);
        }
    }
        
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

    if (!m_assetsHelper->CreateVertexBuffer<BasicVertex>(&m_model, &m_pVertexBuffer))
    {
        return false;
    }    

    if (!CreateIndexBuffer())
    {
        return false;
    }

    CreateCameraMatrix();

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

void OMTGame::UnloadContent()
{
    if(m_ambientMusic)
    {
        m_ambientMusic->Stop();
        m_ambientMusic.release();
    }
    if(m_bangSound)
    {
        m_bangSound->Stop();
        m_bangSound.release();
    }
    m_waveBank.release();
    delete m_mousePos;
}

void OMTGame::Update()
{
    if (m_retryAudio)
    {
        m_retryAudio = false;
        if (m_audEngine->Reset())
        {
            // Restart any looped sounds here
            if(m_ambientMusic)
                m_ambientMusic->Play(true); 
        }
    }
    else if(!m_audEngine->Update())
    {
        if (m_audEngine->IsCriticalError())
        {
            m_retryAudio = true;
        }
    }
    
    GetCursorPos(m_mousePos);
    ScreenToClient(m_hWnd, m_mousePos);
    m_mousePosNorm.x = (float)m_mousePos->x / m_windSize.x;
    m_mousePosNorm.y = (1.0f - (float)m_mousePos->y / m_windSize.y);
    m_mousePosNorm.x -= 0.5f;
    m_mousePosNorm.x *= 2.0f;
    m_mousePosNorm.y -= 0.5f;
    m_mousePosNorm.y *= 2.0f;

    if (m_mousePosNorm.x > -1.0f && m_mousePosNorm.x < 1.0f &&
        m_mousePosNorm.y > -1.0f && m_mousePosNorm.y < 1.0f)
    {
        //ShowCursor(false);
    }

    auto scaleMatrix = DirectX::XMMatrixScaling(0.005, 0.005, 0.005);
    auto rotationMatrix = DirectX::XMMatrixRotationX(m_mousePosNorm.y * 2) * DirectX::XMMatrixRotationY(-m_mousePosNorm.x * 2);
    auto positionMatrix = DirectX::XMMatrixTranslation(0, 0, 3);

    m_modelMatrix = scaleMatrix * rotationMatrix * positionMatrix;

    m_emitter.SetPosition(float3(m_mousePosNorm.x, 0, 0));
}

void OMTGame::Render()
{
    // Check if D3D is ready
    if (m_pD3DContext == nullptr)
        return;

    // Clear back buffer
    float color[4] = { 0.2f, 0.2f, 0.3f, 1.0f };
    m_pD3DContext->ClearRenderTargetView(m_pD3DRenderTargetView, color);
    m_pD3DContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // Set shaders
    m_pD3DContext->VSSetShader(m_pVertexShader, nullptr, 0);
    m_pD3DContext->PSSetShader(m_pPixelShader, nullptr, 0);
    m_pD3DContext->PSSetSamplers(0, 1, &m_pColorMapSampler);

    // Set stuff
    m_pD3DContext->IASetInputLayout(m_pBasicInputLayout);
    float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_pD3DContext->OMSetBlendState(m_pBlendState, blendFactor, 0xFFFFFFFF);
    m_pD3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Set the render target
    m_pD3DContext->OMSetRenderTargets(1, &m_pD3DRenderTargetView, m_pDepthStencilView);
    UINT stride = sizeof(BasicVertex);
    UINT offset = 0;
    m_pD3DContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
    m_pD3DContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    m_modelMatrix = XMMatrixTranspose(m_modelMatrix);
    m_pD3DContext->UpdateSubresource(m_pModelCB, 0, nullptr, &m_modelMatrix, 0, 0);
    
    m_pD3DContext->VSSetConstantBuffers(0, 1, &m_pModelCB);
    m_pD3DContext->VSSetConstantBuffers(1, 1, &m_pViewCB);
    m_pD3DContext->VSSetConstantBuffers(2, 1, &m_pProjCB);
    
    m_pD3DContext->PSSetShaderResources(0, 1, &m_pColorMapOne);
    m_pD3DContext->DrawIndexed(m_model.indexCount, 0, 0);

    if(m_drawWire)
    {
        m_pD3DContext->PSSetShader(m_pPixelShaderWire, nullptr, 0);
        m_pD3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        m_pD3DContext->DrawIndexed(m_model.indexCount, 0, 0);
    }

    // Present back buffer to display
    m_pSwapChain->Present(0, 0);
}

void OMTGame::ProcessClick()
{
}

void OMTGame::ProcessRightClick()
{
    m_drawWire = !m_drawWire;

    if(m_bangSound)
    {
        m_bangSound->Stop();
        m_bangSound.release();
    }

    m_bangSound = m_waveBank->CreateInstance(m_rndBang(m_mt), DirectX::SoundEffectInstance_Use3D);
    m_bangSound->Play();
    m_bangSound->Apply3D(m_listener, m_emitter);
}