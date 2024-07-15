#pragma once

#include <random>
#include "Dx11Base.h"
#include "FBXImporter.h"
#include "RenderingData.h"
#include "Typedefs.h"

class OMTGame : public Dx11Base
{
// Constructors
public:
    OMTGame();

// Overrides
    virtual bool LoadContent();
    virtual void UnloadContent();

    virtual void Update();
    virtual void Render();
    void ProcessClick();
    void ProcessRightClick();

private:
    bool CreateConstantBuffers();
    bool CreateIndexBuffer();
    
    void CreateCameraMatrix();
    void SetBlendingMode();
    bool CreateTextureSampler();

    ID3D11VertexShader* m_pVertexShader;
    ID3D11InputLayout* m_pBasicInputLayout;
    ID3D11PixelShader* m_pPixelShader;
    ID3D11PixelShader* m_pPixelShaderWire;
    ID3D11Buffer* m_pVertexBuffer;
    ID3D11Buffer* m_pIndexBuffer;

    ID3D11Buffer* m_pViewCB;
    ID3D11Buffer* m_pProjCB;
    ID3D11Buffer* m_pModelCB;
    
    ID3D11ShaderResourceView* m_pColorMapOne;
    ID3D11SamplerState* m_pColorMapSampler;
    ID3D11Resource* m_pColorMapResource;
    ID3D11BlendState* m_pBlendState;

    matrix m_viewMatrix;
    matrix m_projMatrix;
    matrix m_modelMatrix;

    POINT* m_mousePos;
    float2 m_mousePosNorm;

    FBXImporter::FBXModel m_model;
    
    int m_currentModel = 0;
    bool m_drawWire = false;
    
    std::mt19937 m_mt;
    std::uniform_int_distribution<int> m_rndBang;
    std::unique_ptr<DirectX::SoundEffectInstance> m_bangSound;
    std::unique_ptr<DirectX::SoundEffectInstance> m_ambientMusic;
    std::unique_ptr<DirectX::WaveBank> m_waveBank;
    DirectX::AudioListener m_listener;
    DirectX::AudioEmitter  m_emitter;
};