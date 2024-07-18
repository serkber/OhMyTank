#pragma once
#include <random>
#include "Dx11Base.h"
#include "OMTInput.h"
#include "OMTRender.h"
#include "Typedefs.h"

class OMTGame : public Dx11Base
{
    friend class OMTRender;
    friend class OMTInput;
    
// Constructors
public:
    OMTGame();

    static OMTGame* m_gameInstance;

// Overrides
    void ReInitializeGraphics(HWND hWnd, HINSTANCE hInst) override;
    virtual bool LoadContent();
    virtual void UnloadContent();
    virtual void Update();
    virtual void Render();

    void ProcessClick();
    void ProcessRightClick();
    void RotateCamera(float horizontal, float vertical);
    void PanCamera(float horizontal, float vertical);
    void MoveCamera(float forward, float right);
    void SetFocusState(bool state);

private:
    OMTRender m_render;
    OMTInput m_input;
    bool m_isWindowFocused = true;

    bool CreateConstantBuffers();
    
    void SetBlendingMode();
    bool CreateTextureSampler();
    
    int m_currentModel = 0;
    
    std::mt19937 m_mt;
    std::uniform_int_distribution<int> m_rndBang;
    std::unique_ptr<DirectX::SoundEffectInstance> m_bangSound;
    std::unique_ptr<DirectX::SoundEffectInstance> m_ambientMusic;
    std::unique_ptr<DirectX::WaveBank> m_waveBank;
    DirectX::AudioListener m_listener;
    DirectX::AudioEmitter  m_emitter;

    float modelRot = 0;
    matrix m_modelMatrix;
    bool m_drawWire = false;

    float m_camRotX = 0, m_camRotY = 0;
    DirectX::XMVECTOR m_camPos;
};