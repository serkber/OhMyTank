#include "OMTGame.h"
#include <DirectXMath.h>
#include <Audio.h>
#include "WaveBank.h"
#include "OMTRender.h"
#include "SimpleMath.h"

OMTGame* OMTGame::m_pGameInstance = nullptr;

OMTGame::OMTGame()
{
    m_tankMatrix = DirectX::XMMatrixScaling(0.005, 0.005, 0.005);
    m_tankMatrix *= DirectX::XMMatrixTranslation(0, 0, 3);
    m_camPos = DirectX::XMVectorSet(0, 4, -20, 0);
    m_camRotY = 0.349066;
    m_camRotX = 0;
    m_pGameInstance = this;
}

void OMTGame::ReInitializeGraphics(HWND hWnd, HINSTANCE hInst)
{
    Dx11Base::ReInitializeGraphics(hWnd, hInst);
    m_render.LoadGraphicContent();
}

// Overrides
bool OMTGame::LoadContent()
{
    if(!m_input.InitializeInput())
    {
        return false;
    }
    if(!m_render.LoadGraphicContent())
    {
        return false;
    }
    
    m_rndBang = std::uniform_int_distribution<int>(0, 3);

    // if(!m_waveBank)
    // {
    //     m_waveBank = std::make_unique<DirectX::WaveBank>(m_audEngine, L"XACT/Win/WaveBank.xwb");
    //     m_ambientMusic = m_waveBank->CreateInstance( XACT_WAVEBANK_WAVEBANK_SPACESHIP_THEME );
    //     if ( m_ambientMusic )
    //     {
    //         m_ambientMusic->Play(true);
    //     }
    // }
        
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

    if(m_isWindowFocused)
    {
        m_input.HandleMouse();
        m_input.HandleKeyboard();
    }
    
    //std::cout << m_input.m_mousePos.x << ' ' << m_input.m_mousePos.y << std::endl;
    //std::cout << m_input.m_isLeftButtonDown << std::endl;

    auto scaleMatrix = DirectX::XMMatrixScaling(0.01, 0.01, 0.01);
    auto rotationMatrix = DirectX::XMMatrixRotationY(m_tankRot);
    m_tankRot += 1 * m_deltaTime;
    auto positionMatrix = DirectX::XMMatrixTranslation(0, 0, 3);
    
    m_tankMatrix = scaleMatrix * rotationMatrix * positionMatrix;
    
    m_emitter.SetPosition(float3(m_input.m_mousePosNorm.x, 0, 0));
}

void OMTGame::Render()
{
    m_render.Render();
}

void OMTGame::RotateCamera(float horizontal, float vertical)
{
    m_camRotX += horizontal;
    m_camRotY += vertical;
}

void OMTGame::PanCamera(float horizontal, float vertical)
{
    auto up = DirectX::XMVectorScale(m_render.m_viewMatrix.r[1], vertical * 2);
    auto right = DirectX::XMVectorScale(m_render.m_viewMatrix.r[0], -horizontal * 2);
    auto delta = DirectX::XMVectorAdd(up, right);
    m_camPos = DirectX::XMVectorAdd(m_camPos, delta);
}

void OMTGame::MoveCamera(float forward, float right)
{
    auto forwardVec = DirectX::XMVectorScale(m_render.m_viewMatrix.r[2], forward * m_deltaTime);
    auto rightVec = DirectX::XMVectorScale(m_render.m_viewMatrix.r[0], right * m_deltaTime);
    auto delta = DirectX::XMVectorAdd(forwardVec, rightVec);
    
    m_camPos = DirectX::XMVectorAdd(m_camPos, delta);
}

void OMTGame::SetFocusState(bool state)
{
    if(state && !m_isWindowFocused)
    {
        m_input.InitializeInput();
    }
    
    m_isWindowFocused = state;
}
