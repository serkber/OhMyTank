#pragma once

#include <array>
#include <d3d11.h>
#include "Typedefs.h"

namespace RenderingData
{
    static std::array<D3D11_INPUT_ELEMENT_DESC, 4> basicInputLayoutDescriptor
    {
        "POSITION", 0,  DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0,
        "NORMAL", 0,  DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0,
        "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0,
        "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0,
    };
    
    struct BasicVertex
    {
        float3 pos;
        float3 nor;
        float2 uv;
        float4 col;
    };
}