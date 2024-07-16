#pragma once
#include <random>
#include "Dx11Base.h"
#include "OMTRender.h"
#include "Typedefs.h"

class OMTGame : public Dx11Base
{
    friend class OMTRender;
    
// Constructors
public:
    OMTGame();

// Overrides
    virtual bool LoadContent();
    virtual void UnloadContent();

    virtual void Update();
    virtual void Render();

// Self logic
    void ProcessClick();
    void ProcessRightClick();
    

private:
    OMTRender* m_render;
    
    bool CreateConstantBuffers();
    
    void SetBlendingMode();
    bool CreateTextureSampler();

    POINT* m_mousePos;
    float2 m_mousePosNorm;
    
    int m_currentModel = 0;
    
    std::mt19937 m_mt;
    std::uniform_int_distribution<int> m_rndBang;
    std::unique_ptr<DirectX::SoundEffectInstance> m_bangSound;
    std::unique_ptr<DirectX::SoundEffectInstance> m_ambientMusic;
    std::unique_ptr<DirectX::WaveBank> m_waveBank;
    DirectX::AudioListener m_listener;
    DirectX::AudioEmitter  m_emitter;
    
    matrix m_modelMatrix;
    bool m_drawWire = false;
};