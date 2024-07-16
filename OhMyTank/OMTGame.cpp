#include "OMTGame.h"
#include "Utils.h"
#include <DirectXMath.h>
#include <Audio.h>
#include "WaveBank.h"

OMTGame::OMTGame()
{
    m_render = nullptr;
    m_mousePos = new POINT;
    m_mousePosNorm = float2(0.0f, 0.0f);
    m_modelMatrix = DirectX::XMMatrixIdentity();
}

// Overrides
bool OMTGame::LoadContent()
{
    m_render = new OMTRender(this);
    
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
    delete m_render;
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
    m_render->Render();
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