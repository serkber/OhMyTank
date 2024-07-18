#pragma once

#include <d3d11.h>
#include <vector>
#define _WIN32_WINNT 0x0602
#define _USE_MATH_DEFINES
#include "Audio.h"

class Dx11Base
{
    // Constructors
public:
    Dx11Base();
    virtual ~Dx11Base();

    static Dx11Base* m_instance;

    // Methods
    bool Initialize(HWND hWnd, HINSTANCE hInst);
    void Terminate();
    bool CompileShader(const wchar_t* shader_name, const char* shader_entry_point_name, LPCSTR shaderModel, ID3DBlob** buffer, LPCWSTR* errorMessage);
    bool CreateDepthStencilResources();
    void OnNewAudioDevice() noexcept;
    void ReleaseGraphicsResources();

    // Overrides
    virtual void ReInitializeGraphics(HWND hWnd, HINSTANCE hInst);
    virtual bool LoadContent() = 0;
    virtual void UnloadContent() = 0;

    virtual void Update() = 0;
    virtual void Render() = 0;

    // Attributes
    HWND m_hWnd;
    HINSTANCE m_hInst;
    ID3D11Device* m_pD3DDevice;
    ID3D11DeviceContext* m_pD3DContext;
    ID3D11RenderTargetView*	m_pD3DRenderTargetView;
    IDXGISwapChain* m_pSwapChain;

    ID3D11Texture2D* m_pDepthTexture;
    ID3D11DepthStencilState* m_pDepthStencilState;
    ID3D11DepthStencilView* m_pDepthStencilView;
    ID3D11ShaderResourceView* m_pDepthShaderResource;
    
    DirectX::AudioEngine* m_audEngine;
    bool m_retryAudio = false;

    POINT m_windSize;

private:
    void GetWindowSize(HWND hwnd);
    std::vector<ID3D11Resource**> m_resources;
};