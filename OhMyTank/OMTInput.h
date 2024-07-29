#pragma once

#include <array>
#include <dinput.h>
#include <map>

#include "Typedefs.h"

struct Key
{
    bool down = false;
    bool up = false;
    bool pressed = false;
};

class OMTInput
{
public:
    OMTInput();
    ~OMTInput();
    bool InitializeInput();
    void HandleMouse();
    void EvaluateKey(bool currentState, std::map<int, Key*>::value_type& key);
    void HandleKeyboard();

    POINT m_mousePos;
    POINT m_mousePosRelative;
    float2 m_mousePosNorm;
    POINT m_mouseDelta;
    bool m_isMouseOverWindow = false;

    std::map<int, Key*> mouseButtons{
        {0, nullptr},
        {1, nullptr},
        {2, nullptr},
    };

    std::map<int, Key*> keys {
        {DIK_W, nullptr},
        {DIK_S, nullptr},
        {DIK_A, nullptr},
        {DIK_D, nullptr},
        {DIK_R, nullptr},
        {DIK_T, nullptr}
    };

private:    
    LPDIRECTINPUT8 m_pDirectInput;
    LPDIRECTINPUTDEVICE8 m_pMouseDevice;
    LPDIRECTINPUTDEVICE8 m_pKeyboardDevice;
};