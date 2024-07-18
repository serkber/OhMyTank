#pragma once
#include <d3d11.h>
#include "AssetsHelper.h"
#include "FBXImporter.h"
#include "Typedefs.h"

class OMTRender
{
friend class OMTGame;
    
public:
    OMTRender();
    ~OMTRender();
    void CreateCameraMatrix();
    bool LoadGraphicContent();
    bool CreateConstantBuffers();
    void SetBlendingMode();
    bool CreateTextureSampler();
    void Render();
    void ReleaseResources();

private:
    ID3D11VertexShader* m_pVertexShader;
    ID3D11InputLayout* m_pBasicInputLayout;
    ID3D11PixelShader* m_pPixelShader;
    ID3D11PixelShader* m_pPixelShaderWire;
    ID3D11Buffer* m_pVertexBuffer;
    ID3D11Buffer* m_pIndexBuffer;

    ID3D11Buffer* m_pViewCB;
    ID3D11Buffer* m_pProjCB;
    ID3D11Buffer* m_pModelCB;
    
    ID3D11ShaderResourceView* m_pColorMapOne;
    ID3D11SamplerState* m_pColorMapSampler;
    ID3D11Resource* m_pColorMapResource;
    ID3D11BlendState* m_pBlendState;

    matrix m_viewMatrix;
    matrix m_projMatrix;
    FBXImporter::FBXModel m_model;
    
    AssetsHelper* m_assetsHelper;
    
    std::vector<ID3D11Resource**> m_resources;
};
