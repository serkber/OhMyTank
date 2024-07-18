#pragma once
#include <d3d11.h>
#include <d3dcommon.h>
#include "Dx11Base.h"
#include "FBXImporter.h"
#include "RenderingData.h"

class AssetsHelper
{
public:    
    template <typename T>
    bool LoadShader(const wchar_t* file, ID3DBlob** pVSBuffer, T** pVertexShader);
    
    bool LoadTexture(const wchar_t* file, ID3D11Resource** texture, ID3D11ShaderResourceView** textureView);
    bool CreateInputLayout(D3D11_INPUT_ELEMENT_DESC* descriptor, UINT elementsCount, ID3DBlob* vertexShaderBlob, ID3D11InputLayout** inputLayout);
    
    template <typename T>
    bool CreateModelBuffers(FBXImporter::FBXModel* model, ID3D11Buffer** vertexBuffer, ID3D11Buffer** indexBuffer);

private:
    bool CompileShader(const wchar_t* shaderName, const char* shaderEntryPoint, LPCSTR shaderModel, ID3DBlob** buffer, LPCWSTR* errorMessage);
    void FullfillBasicVertexArray(RenderingData::BasicVertex* array, UINT elementsCount, FBXImporter::FBXModel* model);
};
