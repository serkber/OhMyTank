#include "OMTInput.h"
#include "OMTGame.h"
#include "Utils.h"

OMTInput::OMTInput()
{
    for (auto &key : keys)
    {
        key.second = new Key;
    }
    for (auto &key : mouseButtons)
    {
        key.second = new Key;
    }
}

OMTInput::~OMTInput()
{
    for (auto &key : keys)
    {
        delete key.second;
        key.second = nullptr;
    }
    for (auto &key : mouseButtons)
    {
        delete key.second;
        key.second = nullptr;
    }
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
    GetCursorPos(&m_mousePos);
    
    m_mousePosRelative = m_mousePos;
    ScreenToClient(OMTGame::m_pGameInstance->m_hWnd, &m_mousePosRelative);
    
    m_mousePosNorm.x = (float)m_mousePosRelative.x / OMTGame::m_pGameInstance->m_windSize.x;
    m_mousePosNorm.y = (1.0f - (float)m_mousePosRelative.y / OMTGame::m_pGameInstance->m_windSize.y);
    m_mousePosNorm.x -= 0.5f;
    m_mousePosNorm.x *= 2.0f;
    m_mousePosNorm.y -= 0.5f;
    m_mousePosNorm.y *= 2.0f;

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

    for (auto &key : mouseButtons)
    {
        key.second->down = false;
        key.second->up = false;
    }

    for(auto &key : mouseButtons)
    {
        EvaluateKey(mouseData.rgbButtons[key.first] & 0x80, key);
    }

    m_mouseDelta.x = mouseData.lX;
    m_mouseDelta.y = mouseData.lY;
}

void OMTInput::EvaluateKey(bool currentState, std::map<int, Key*>::value_type& key)
{
    if(currentState)
    {
        if(!key.second->pressed)
        {
            key.second->down = true;
        }
        key.second->pressed = true;
    }
    else
    {            
        if(key.second->pressed)
        {
            key.second->up = true;
        }
        key.second->pressed = false;
    }
}

void OMTInput::HandleKeyboard()
{
    // Get keyboard state data
    char keyboardData[256];
    m_pKeyboardDevice->GetDeviceState(sizeof(keyboardData), (void*)&keyboardData);

    for (auto &key : keys)
    {
        key.second->down = false;
        key.second->up = false;
    }

    for (auto &key : keys)
    {
        EvaluateKey(keyboardData[key.first] & 0x80, key);
    }
}
