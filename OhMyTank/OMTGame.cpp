#include "OMTGame.h"
#include <DirectXMath.h>
#include <Audio.h>
#include "OMTRender.h"

OMTGame* OMTGame::m_pGameInstance = nullptr;

OMTGame::OMTGame()
{
    m_tankMatrix = DirectX::XMMatrixIdentity();    
    m_cameraMatrix = DirectX::XMMatrixIdentity();
    m_tankPos = DirectX::XMVectorSet(0, 0, 0, 0); 
    m_camPos = DirectX::XMVectorSet(0, 6, -15, 0);
    m_camRotJaw = 0.35;
    m_camRotPitch = 0;
    m_grassPos = DirectX::XMVectorSet(0, 0, 0, 0);

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

void OMTGame::ControlCamera()
{
    if(m_input.mouseButtons[1]->down || m_input.mouseButtons[2]->down)
    {
        m_isControllingCamera = true;
        ShowCursor(false);
        m_savedMousePos = m_input.m_mousePos;
    }
    if(m_input.mouseButtons[1]->up || m_input.mouseButtons[2]->up)
    {
        m_isControllingCamera = false;
        ShowCursor(true);
        ::SetCursorPos(m_savedMousePos.x, m_savedMousePos.y);
    }
    if(m_input.mouseButtons[1]->pressed)
    {
        RotateCamera(m_input.m_mouseDelta.x * m_rotSensitivity, m_input.m_mouseDelta.y * m_rotSensitivity);
        float forwardCam = 0;
        float rightCam = 0;
        if(m_input.keys[DIK_W]->pressed)
        {
            forwardCam += 1;
        }
        if(m_input.keys[DIK_S]->pressed)
        {
            forwardCam -= 1;
        }
        if(m_input.keys[DIK_A]->pressed)
        {
            rightCam -= 1;
        }
        if(m_input.keys[DIK_D]->pressed)
        {
            rightCam += 1;
        }
        MoveCamera(forwardCam, rightCam);
    }
    else if(m_input.mouseButtons[2]->pressed)
    {
        PanCamera(m_input.m_mouseDelta.x * m_panSensitivity, m_input.m_mouseDelta.y * m_panSensitivity);
    }
    
    m_camPos = DirectX::XMVectorAdd(m_tankPos, DirectX::XMVectorSet(m_camOffsetX, m_camOffsetY, m_camOffsetZ, 0));
}

void OMTGame::ControlTank()
{
    auto lol = DirectX::XMMatrixTranslation(1, 2, 3);
    
    float speed = 0;
    if(!m_isControllingCamera)
    {
        m_tankRot = m_input.keys[DIK_A]->pressed ? m_tankRot -= m_deltaTime * m_tankRotationSpeed : m_tankRot;
        m_tankRot = m_input.keys[DIK_D]->pressed ? m_tankRot += m_deltaTime * m_tankRotationSpeed : m_tankRot;

        speed = m_input.keys[DIK_W]->pressed ? speed += m_deltaTime * m_tankSpeed : speed;
        speed = m_input.keys[DIK_S]->pressed ? speed -= m_deltaTime * m_tankSpeed : speed;
    }
    
    m_tankDir = DirectX::XMVectorSet(sin(m_tankRot), 0, cos(m_tankRot), 0);
    auto tankVelocity = DirectX::XMVectorScale(m_tankDir, m_deltaTime * speed);
    
    m_tankPos = DirectX::XMVectorAdd(m_tankPos ,tankVelocity);
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

    if(m_input.keys[DIK_R]->down)
    {
        m_isDebugEnabled = !m_isDebugEnabled;
    }

    ControlTank();
    ControlCamera();

    CreateTankMatrix();
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

void OMTGame::CreateCameraMatrix()
{
    m_cameraMatrix = DirectX::XMMatrixRotationX(m_camRotJaw);
    m_cameraMatrix *= DirectX::XMMatrixRotationY(m_camRotPitch);
    m_cameraMatrix *= DirectX::XMMatrixTranslationFromVector(m_camPos);
}

void OMTGame::RotateCamera(float horizontal, float vertical)
{
    m_camRotPitch += horizontal;
    m_camRotJaw += vertical;
}

void OMTGame::PanCamera(float horizontal, float vertical)
{
    auto up = DirectX::XMVectorScale(m_cameraMatrix.r[1], vertical * m_camSpeed);
    auto right = DirectX::XMVectorScale(m_cameraMatrix.r[0], -horizontal * m_camSpeed);
    auto delta = DirectX::XMVectorAdd(up, right);
    
    //m_camPos = DirectX::XMVectorAdd(m_camPos, delta);

    m_camOffsetX += DirectX::XMVectorGetX(delta);
    m_camOffsetY += DirectX::XMVectorGetY(delta);
    m_camOffsetZ += DirectX::XMVectorGetZ(delta);
}

void OMTGame::MoveCamera(float forward, float right)
{
    auto forwardVec = DirectX::XMVectorScale(m_cameraMatrix.r[2], forward * m_deltaTime * m_camSpeed);
    auto rightVec = DirectX::XMVectorScale(m_cameraMatrix.r[0], right * m_deltaTime * m_camSpeed);
    auto delta = DirectX::XMVectorAdd(forwardVec, rightVec);
    
    //m_camPos = DirectX::XMVectorAdd(m_camPos, delta);

    m_camOffsetX += DirectX::XMVectorGetX(delta);
    m_camOffsetY += DirectX::XMVectorGetY(delta);
    m_camOffsetZ += DirectX::XMVectorGetZ(delta);
}

void OMTGame::SetFocusState(bool state)
{
    if(state && !m_isWindowFocused)
    {
        m_input.InitializeInput();
    }
    
    m_isWindowFocused = state;
}