#include "OMTGame.h"
#include <DirectXMath.h>
#include <Audio.h>
#include "OMTRender.h"

OMTGame* OMTGame::m_pGameInstance = nullptr;

OMTGame::OMTGame()
{
    m_tankMatrix = DirectX::XMMatrixIdentity();    
    m_cameraMatrix = DirectX::XMMatrixIdentity();    
    m_grass1Matrix = DirectX::XMMatrixIdentity();
    m_grass2Matrix = DirectX::XMMatrixIdentity();
    m_tankPos = DirectX::XMVectorSet(0, 0, 0, 0); 
    m_camPos = DirectX::XMVectorSet(0, 6, -15, 0);
    m_camRotY = 0.35;
    m_camRotX = 0;
    m_grass1Pos = 0;
    m_grass2Pos = GRASS_FIELD_SIZE * GRASS_FIELD_ASPECT;

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
    m_emitter.SetPosition(float3(m_input.m_mousePosNorm.x, 0, 0));

    if(m_isWindowFocused)
    {
        m_input.HandleMouse();
        m_input.HandleKeyboard();
    }

    if(!m_input.m_isControllingCamera)
    {
        m_tankRot = m_input.m_isAPressed ? m_tankRot -= m_deltaTime * 3 : m_tankRot;
        m_tankRot = m_input.m_isDPressed ? m_tankRot += m_deltaTime * 3 : m_tankRot;
    }
    
    //m_tankRot = sin(m_elapsedTime) * 0.2;
    float speed = 0;
    if(m_input.m_isWPressed && !m_input.m_isControllingCamera)
    {
        speed += 30;
    }
    if(m_input.m_isSPressed && !m_input.m_isControllingCamera)
    {
        speed -= 30;
    }
    
    auto tankDelta = DirectX::XMVectorSet(sin(m_tankRot), 0, cos(m_tankRot), 0);
    tankDelta = DirectX::XMVectorScale(tankDelta, m_deltaTime * speed);
    
    m_tankPos = DirectX::XMVectorAdd(m_tankPos ,tankDelta);
    
    m_camPos = DirectX::XMVectorAdd(m_tankPos, DirectX::XMVectorSet(m_camOffsetX, m_camOffsetY, m_camOffsetZ, 0));

    // std::cout << m_camPos.m128_f32[1] << " " << m_camPos.m128_f32[2] << " " << m_camRotY << std::endl;

    CreateTankMatrix();
    CreateGrassMatrices();
    CreateCameraMatrix();
}

void OMTGame::Render()
{
    m_render.Render();
}

void OMTGame::CreateTankMatrix()
{
    auto scaleMatrix = DirectX::XMMatrixScaling(0.01, 0.01, 0.01);
    auto rotationMatrix = DirectX::XMMatrixRotationY(OMTGame::m_pGameInstance->m_tankRot);
    auto positionMatrix = DirectX::XMMatrixTranslationFromVector(OMTGame::m_pGameInstance->m_tankPos);
    m_tankMatrix = scaleMatrix * rotationMatrix * positionMatrix;
}

void OMTGame::CreateGrassMatrices()
{
    m_grass1Matrix = DirectX::XMMatrixTranslation(0, 0, m_grass1Pos);
    m_grass2Matrix = DirectX::XMMatrixTranslation(0, 0, m_grass2Pos);
    //std::cout << m_grass2Pos << std::endl;
}

void OMTGame::CreateCameraMatrix()
{
    m_cameraMatrix = DirectX::XMMatrixRotationX(m_camRotY);
    m_cameraMatrix *= DirectX::XMMatrixRotationY(m_camRotX);
    m_cameraMatrix *= DirectX::XMMatrixTranslationFromVector(m_camPos);
}

void OMTGame::RotateCamera(float horizontal, float vertical)
{
    m_camRotX += horizontal;
    m_camRotY += vertical;
}

void OMTGame::PanCamera(float horizontal, float vertical)
{
    // Free cam
    // auto up = DirectX::XMVectorScale(m_cameraMatrix.r[1], vertical * 10);
    // auto right = DirectX::XMVectorScale(m_cameraMatrix.r[0], -horizontal * 10);
    // auto delta = DirectX::XMVectorAdd(up, right);
    // m_camPos = DirectX::XMVectorAdd(m_camPos, delta);
    m_camOffsetX -= horizontal * 10;
    m_camOffsetY += vertical * 10;
}

void OMTGame::MoveCamera(float forward, float right)
{
    // Free cam
    // auto forwardVec = DirectX::XMVectorScale(m_cameraMatrix.r[2], forward * m_deltaTime * 10);
    // auto rightVec = DirectX::XMVectorScale(m_cameraMatrix.r[0], right * m_deltaTime * 10);
    // auto delta = DirectX::XMVectorAdd(forwardVec, rightVec);
    //
    // m_camPos = DirectX::XMVectorAdd(m_camPos, delta);

    m_camOffsetZ += forward * 10 * m_deltaTime;
}

void OMTGame::SetFocusState(bool state)
{
    if(state && !m_isWindowFocused)
    {
        m_input.InitializeInput();
    }
    
    m_isWindowFocused = state;
}