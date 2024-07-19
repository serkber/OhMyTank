#include "OMTInput.h"

#include <bitset>

#include "OMTGame.h"
#include "Utils.h"

OMTInput::OMTInput()
{
    m_mousePosNorm = float2(0.0f, 0.0f);
}

OMTInput::~OMTInput()
{
}

bool OMTInput::InitializeInput()
{
    HRESULT hr;
    
    // Create DirectInput8 device
    hr = ::DirectInput8Create(
        OMTGame::m_pGameInstance->m_hInst, DIRECTINPUT_VERSION, 
        IID_IDirectInput8, (void**)&m_pDirectInput, 0);
    if (FAILED(hr))
    {
        ::MessageBox(OMTGame::m_pGameInstance->m_hWnd, Utils::GetMessageFromHr(hr), L"Direct Input Device Error", MB_OK);
        return false;
    }
    
    // Create mouse device
    hr = m_pDirectInput->CreateDevice(GUID_SysMouse, &m_pMouseDevice, nullptr);
    if (FAILED(hr))
    {
        ::MessageBox(OMTGame::m_pGameInstance->m_hWnd, Utils::GetMessageFromHr(hr), L"Mouse Create Device Error", MB_OK);
        return false;
    }
    hr = m_pMouseDevice->SetDataFormat(&c_dfDIMouse);
    if (FAILED(hr))
    {
        ::MessageBox(OMTGame::m_pGameInstance->m_hWnd, Utils::GetMessageFromHr(hr), L"Mouse Data Format Error", MB_OK);
        return false;
    }
    hr = m_pMouseDevice->SetCooperativeLevel(OMTGame::m_pGameInstance->m_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    if (FAILED(hr))
    {
        ::MessageBox(OMTGame::m_pGameInstance->m_hWnd, Utils::GetMessageFromHr(hr), L"Mouse Cooperative Level Error", MB_OK);
        return false;
    }
    hr = m_pMouseDevice->Acquire();
    if (FAILED(hr))
    {
        ::MessageBox(OMTGame::m_pGameInstance->m_hWnd, Utils::GetMessageFromHr(hr), L"Mouse Acquire Error", MB_OK);
        return false;
    }
    
    // Create keyboard device
    hr = m_pDirectInput->CreateDevice(GUID_SysKeyboard, &m_pKeyboardDevice, 0);
    if (FAILED(hr))
    {
        ::MessageBox(OMTGame::m_pGameInstance->m_hWnd, Utils::GetMessageFromHr(hr), L"Keyboard Create Device Error", MB_OK);
        return false;
    }
    hr = m_pKeyboardDevice->SetDataFormat(&c_dfDIKeyboard);
    if (FAILED(hr))
    {
        ::MessageBox(OMTGame::m_pGameInstance->m_hWnd, Utils::GetMessageFromHr(hr), L"Keyboard Data Format Error", MB_OK);
        return false;
    }
    hr = m_pKeyboardDevice->SetCooperativeLevel(OMTGame::m_pGameInstance->m_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    if (FAILED(hr))
    {
        ::MessageBox(OMTGame::m_pGameInstance->m_hWnd, Utils::GetMessageFromHr(hr), L"Keyboard Cooperative Level Error", MB_OK);
        return false;
    }
    hr = m_pKeyboardDevice->Acquire();
    if (FAILED(hr))
    {
        ::MessageBox(OMTGame::m_pGameInstance->m_hWnd, Utils::GetMessageFromHr(hr), L"Keyboard Acquire Error", MB_OK);
        return false;
    }

    return true;
}

void OMTInput::HandleMouse()
{
    // Absolute mouse handling
    if(!m_isControllingCamera)
    {
        GetCursorPos(&m_mousePos);
        
        m_mousePosRelative = m_mousePos;
        ScreenToClient(OMTGame::m_pGameInstance->m_hWnd, &m_mousePosRelative);
        
        m_mousePosNorm.x = (float)m_mousePosRelative.x / OMTGame::m_pGameInstance->m_windSize.x;
        m_mousePosNorm.y = (1.0f - (float)m_mousePosRelative.y / OMTGame::m_pGameInstance->m_windSize.y);
        m_mousePosNorm.x -= 0.5f;
        m_mousePosNorm.x *= 2.0f;
        m_mousePosNorm.y -= 0.5f;
        m_mousePosNorm.y *= 2.0f;
    }

    if (m_mousePosNorm.x > -1.0f && m_mousePosNorm.x < 1.0f &&
        m_mousePosNorm.y > -1.0f && m_mousePosNorm.y < 1.0f)
    {
        m_isMouseOverWindow = true;
    }
    else
    {
        m_isMouseOverWindow = false;
    }

    // Mouse clicks
    // Get mouse state data
    DIMOUSESTATE mouseData;
    m_pMouseDevice->GetDeviceState(sizeof(mouseData), &mouseData);
    
    // Button: Left mouse button
    m_isLeftButtonDown = false;
    if (mouseData.rgbButtons[0] & 0x80) {
        m_isLeftButtonDown = true;
    }
    
    // Button: Right mouse button
    m_isRightButtonDown = false;
    if (mouseData.rgbButtons[1] & 0x80) {
        m_isRightButtonDown = true;
    }
    
    // Button: Middle mouse button
    m_isMiddleButtonDown = false;
    if (mouseData.rgbButtons[2] & 0x80) {
        m_isMiddleButtonDown = true;
    }

#ifdef _DEBUG
    OMTGame::m_pGameInstance->m_isPainting = false;
    if(m_isLeftButtonDown && m_isMouseOverWindow)
    {
        OMTGame::m_pGameInstance->m_isPainting = true;
    }
    
    if((m_isRightButtonDown || m_isMiddleButtonDown) && m_isMouseOverWindow)
    {
        if(m_isCursorShown)
        {
            m_isCursorShown = false;
            ShowCursor(false);
        }
        
        m_isControllingCamera = true;
        SetCursorPos(m_mousePos.x, m_mousePos.y);
        
        if(m_isRightButtonDown)
        {
            OMTGame::m_pGameInstance->RotateCamera(mouseData.lX / (float)OMTGame::m_pGameInstance->m_windSize.x,
                mouseData.lY / (float)OMTGame::m_pGameInstance->m_windSize.y);
        }
        if(m_isMiddleButtonDown)
        {
            OMTGame::m_pGameInstance->PanCamera(mouseData.lX / (float)OMTGame::m_pGameInstance->m_windSize.x,
                mouseData.lY / (float)OMTGame::m_pGameInstance->m_windSize.y);
        }
        
        float forwardCam = 0;
        float rightCam = 0;
        if(m_isWPressed)
        {
            forwardCam += m_camSpeed;
        }
        if(m_isSPressed)
        {
            forwardCam -= m_camSpeed;
        }
        if(m_isDPressed)
        {
            rightCam += m_camSpeed;
        }
        if(m_isAPressed)
        {
            rightCam -= m_camSpeed;
        }

        OMTGame::m_pGameInstance->MoveCamera(forwardCam, rightCam);
    }
    else
    {
        if(!m_isCursorShown)
        {
            m_isCursorShown = true;
            ShowCursor(true);
        }
        m_isControllingCamera = false;
    }
#endif
}

void OMTInput::HandleKeyboard()
{
    // Get keyboard state data
    char keyboardData[256];
    m_pKeyboardDevice->GetDeviceState(sizeof(keyboardData), (void*)&keyboardData);

    m_isWPressed = false;
    // Key: W
    if (keyboardData[DIK_W] & 0x80) {
        m_isWPressed = true;
    }

    m_isAPressed = false;
    // Key: A
    if (keyboardData[DIK_A] & 0x80) { 
        m_isAPressed = true;
    }

    m_isSPressed = false;
    // Key: S
    if (keyboardData[DIK_S] & 0x80) { 
        m_isSPressed = true;
    }

    m_isDPressed = false;
    // Key: D
    if (keyboardData[DIK_D] & 0x80) { 
        m_isDPressed = true;
    }

    if(keyboardData[DIK_R] & 0x80)
    {
        m_shouldReinitialize = true;
    }
}