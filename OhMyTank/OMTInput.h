#pragma once

#include <dinput.h>

#include "Typedefs.h"

class OMTInput
{
public:
    OMTInput();
    ~OMTInput();
    bool InitializeInput();
    void HandleMouse();
    void HandleKeyboard();

    POINT m_mousePos;
    POINT m_mousePosRelative;
    float2 m_mousePosNorm;
    
    bool m_isMouseOverWindow = false;
    bool m_isRightButtonDown = false;
    bool m_isMiddleButtonDown = false;
    bool m_isLeftButtonDown = false;

    bool m_isWPressed = false;
    bool m_isAPressed = false;
    bool m_isSPressed = false;
    bool m_isDPressed = false;
    
    bool m_shouldReinitialize;
    
    bool m_isControllingCamera = false;
    bool m_isCursorShown = true;

private:    
    LPDIRECTINPUT8 m_pDirectInput;
    LPDIRECTINPUTDEVICE8 m_pMouseDevice;
    LPDIRECTINPUTDEVICE8 m_pKeyboardDevice;
};