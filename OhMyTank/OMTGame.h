#pragma once
#include <random>
#include "Dx11Base.h"
#include "OMTInput.h"
#include "OMTRender.h"

#define GRASS_FIELD_ASPECT 6.0f
#define GRASS_FIELD_SIZE 55

class OMTGame : public Dx11Base
{
    friend class OMTRender;
    friend class OMTInput;
    
// Constructors
public:
    OMTGame();

    static OMTGame* m_pGameInstance;

// Overrides
    void ReInitializeGraphics(HWND hWnd, HINSTANCE hInst) override;
    virtual bool LoadContent();
    virtual void UnloadContent();
    virtual void Update();
    virtual void Render();
    void CreateTankMatrix();
    void CreateGrassMatrices();
    void CreateCameraMatrix();

    void RotateCamera(float horizontal, float vertical);
    void PanCamera(float horizontal, float vertical);
    void MoveCamera(float forward, float right);
    void SetFocusState(bool state);

private:
    OMTRender m_render;
    OMTInput m_input;
    bool m_isWindowFocused = true;
    bool m_isPainting;

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

    matrix m_grass1Matrix;
    float m_grass1Pos;
    matrix m_grass2Matrix;
    float m_grass2Pos;

    matrix m_tankMatrix;
    DirectX::XMVECTOR m_tankPos;
    float m_tankRot = 0;
    
    matrix m_cameraMatrix;
    float m_camRotX, m_camRotY;
    DirectX::XMVECTOR m_camPos;
    float m_camOffsetX = 0;
    float m_camOffsetY = 10;
    float m_camOffsetZ = -10.5;

    bool m_isDebugEnabled = false;
    int m_isFXAAEnabled = true;
};