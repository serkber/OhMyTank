#include "Dx11Base.h"
#include "Utils.h"
#include <D3Dcompiler.h>

Dx11Base* Dx11Base::m_instance = nullptr;

Dx11Base::Dx11Base()
{
    m_hWnd = nullptr;
    m_hInst = nullptr;
    m_pD3DDevice = nullptr;
    m_pD3DContext = nullptr;
    m_pD3DRenderTargetView = nullptr;
    m_pSwapChain = nullptr;
    m_pDepthTexture = nullptr;
    m_pDepthStencilView = nullptr;
    m_pDepthStencilState = nullptr;
    m_audEngine = nullptr;
    Dx11Base::m_instance = this;

    m_time = std::chrono::steady_clock::now();
}

Dx11Base::~Dx11Base()
{
}

void Dx11Base::GetWindowSize(HWND hWnd)
{
    RECT rc;
    ::GetClientRect(hWnd, &rc);
    m_windSize.x = rc.right - rc.left;
    m_windSize.y = rc.bottom - rc.top;
}

bool Dx11Base::Initialize(HWND hWnd, HINSTANCE hInst)
{
    // Set attributes
    m_hWnd = hWnd;
    m_hInst = hInst;

    // Get window size
    GetWindowSize(hWnd);
    
    // Swap chain structure
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ::ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = m_windSize.x;
    swapChainDesc.BufferDesc.Height = m_windSize.y;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //DXGI_FORMAT_B8G8R8A8_UNORM; for use in tandem with D2D
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = (HWND)hWnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = true;

    // Supported feature levels
    D3D_FEATURE_LEVEL featureLevel;
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    // Supported driver levels
    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE, D3D_DRIVER_TYPE_SOFTWARE
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    // Flags to use in tandem with D2D
    //UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    UINT flags = 0;

    // Create the D3D device and the swap chain
    HRESULT hr = ::D3D11CreateDeviceAndSwapChain(
        nullptr, 
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr, 
        flags,
        featureLevels,
        numFeatureLevels,
        D3D11_SDK_VERSION,
        &swapChainDesc,
        &m_pSwapChain, 
        &m_pD3DDevice, 
        &featureLevel,
        &m_pD3DContext
        );

    // Check device
    if (FAILED(hr))	{
        MessageBox(hWnd, Utils::GetMessageFromHr(hr), TEXT("ERROR"), MB_OK);
        return false;
    }

    // Get the back buffer from the swapchain
    ID3D11Texture2D *pBackBuffer;
    hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (FAILED(hr)) {
        MessageBox(hWnd, TEXT("Unable to get back buffer"), TEXT("ERROR"), MB_OK);
        return false;
    }

    // Create the render target view
    hr = m_pD3DDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pD3DRenderTargetView);

    // Check render target view
    if (FAILED(hr)) {
        MessageBox(hWnd, TEXT("Unable to create render target view"), TEXT("ERROR"), MB_OK);
        return false;
    }
    
    // Release the back buffer
    if (pBackBuffer != nullptr)
        pBackBuffer->Release();

    // Depth test stuff
    CreateDepthStencilResources();
    // Bind depth stencil state
    m_pD3DContext->OMSetDepthStencilState(m_pDepthStencilState, 1);
    
    m_resources.push_back((ID3D11Resource**)&m_pD3DRenderTargetView);
    m_resources.push_back((ID3D11Resource**)&m_pSwapChain);
    m_resources.push_back((ID3D11Resource**)&m_pD3DContext);
    m_resources.push_back((ID3D11Resource**)&m_pD3DDevice);
    m_resources.push_back((ID3D11Resource**)&m_pDepthTexture);
    m_resources.push_back((ID3D11Resource**)&m_pDepthStencilState);
    m_resources.push_back((ID3D11Resource**)&m_pDepthStencilView);
    m_resources.push_back((ID3D11Resource**)&m_pDepthShaderResource);

    DirectX::AUDIO_ENGINE_FLAGS eflags = DirectX::AudioEngine_Default;
    m_audEngine = new DirectX::AudioEngine(eflags);
    
    if ( !m_audEngine->IsAudioDevicePresent() )
    {
        //Silent mode
    }
}

void Dx11Base::ReleaseGraphicsResources()
{
    for(auto &resource : m_resources)
    {
        if(resource != nullptr)
        {
            Utils::UnloadD3D11Resource(resource);
            resource = nullptr;
        }
    }
}

void Dx11Base::ReInitializeGraphics(HWND hWnd, HINSTANCE hInst)
{
    ReleaseGraphicsResources();
    Initialize(hWnd, hInst);
}

void Dx11Base::PreUpdate()
{
    auto current = std::chrono::steady_clock::now();
    m_deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(current - m_time).count() / (double)1000000;
    m_elapsedTime += m_deltaTime;

    std::cout << 1 / m_deltaTime << '\t' << " FPS" << '\n';
    
    m_time = current;
}

void Dx11Base::Terminate()
{
    ReleaseGraphicsResources();

    if (m_audEngine)
    {
        m_audEngine->Suspend();
        m_audEngine = nullptr;
    }
    
    // Unload content
    UnloadContent();
}

bool Dx11Base::CompileShader(const wchar_t* shaderName, const char* shaderEntryPoint, LPCSTR shaderModel, ID3DBlob** buffer, LPCWSTR* errorMessage)
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

bool Dx11Base::CreateDepthStencilResources()
{
    // Create depth texture
    D3D11_TEXTURE2D_DESC depthTextureDesc;
    ::ZeroMemory( &depthTextureDesc, sizeof(depthTextureDesc));
    depthTextureDesc.Width = m_windSize.x;
    depthTextureDesc.Height = m_windSize.y;
    depthTextureDesc.MipLevels = 1;
    depthTextureDesc.ArraySize = 1;
    depthTextureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    depthTextureDesc.SampleDesc.Count = 1;
    depthTextureDesc.SampleDesc.Quality = 0;
    depthTextureDesc.Usage = D3D11_USAGE_DEFAULT;
    depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    depthTextureDesc.CPUAccessFlags = 0;
    depthTextureDesc.MiscFlags = 0;
    HRESULT hr = m_pD3DDevice->CreateTexture2D(&depthTextureDesc, nullptr, &m_pDepthTexture);
    if (FAILED(hr)) {
        ::MessageBox(m_hWnd, Utils::GetMessageFromHr(hr), L"Depth Texture Error", MB_OK);
        return false;
    }

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    // Depth test parameters
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    
    // Stencil test parameters
    depthStencilDesc.StencilEnable = false;
    depthStencilDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    depthStencilDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
    
    // Stencil operations if pixel is front-facing
    depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    
    // Stencil operations if pixel is back-facing
    depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    hr = m_pD3DDevice->CreateDepthStencilState(&depthStencilDesc, &m_pDepthStencilState);
    if (FAILED(hr)) {
        ::MessageBox(m_hWnd, Utils::GetMessageFromHr(hr), L"Depth Stencil State Error", MB_OK);
        return false;
    }
    
    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ::ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
    depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;
    hr = m_pD3DDevice->CreateDepthStencilView(m_pDepthTexture, &depthStencilViewDesc, &m_pDepthStencilView);
    if (FAILED(hr)) {
        ::MessageBox(m_hWnd, Utils::GetMessageFromHr(hr), L"Depth Stencil View Error", MB_OK);
        return false;
    }
    
    D3D11_SHADER_RESOURCE_VIEW_DESC depthShaderResourceDesc;
    depthShaderResourceDesc.Format = DXGI_FORMAT_R32_FLOAT;
    depthShaderResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    depthShaderResourceDesc.Texture2D.MostDetailedMip = 0;
    depthShaderResourceDesc.Texture2D.MipLevels = 1;

    m_pD3DDevice->CreateShaderResourceView(m_pDepthTexture, &depthShaderResourceDesc, &m_pDepthShaderResource);
    if (FAILED(hr)) {
        ::MessageBox(m_hWnd, Utils::GetMessageFromHr(hr), L"Depth Shader Resource Error", MB_OK);
        return false;
    }
    return true;
}

void Dx11Base::OnNewAudioDevice() noexcept
{
    m_retryAudio = true;
}