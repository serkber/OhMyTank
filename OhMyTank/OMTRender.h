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
bool SetupRenderTexture();
bool LoadGraphicContent();
    bool CreateConstantBuffers();
    void SetBlendingMode();
    bool CreateTextureSampler();
void SetRenderViewport(float width, float height);
void Render();
    void ReleaseResources();

private:
    ID3D11VertexShader* m_pVertexShader;
    ID3D11InputLayout* m_pBasicInputLayout;
    ID3D11PixelShader* m_pPixelShader;
    ID3D11Buffer* m_pVertexBufferTank;
    ID3D11Buffer* m_pIndexBufferTank;
    ID3D11Buffer* m_pVertexBufferQuad;
    ID3D11Buffer* m_pIndexBufferQuad;

    ID3D11Buffer* m_pViewCB;
    ID3D11Buffer* m_pProjCB;
    ID3D11Buffer* m_pModelCB;
    
    ID3D11VertexShader* m_pRenderTextureVS;
    ID3D11PixelShader* m_pRenderTexturePS;
    ID3D11VertexShader* m_pTextureDebugVS;
    ID3D11PixelShader* m_pTextureDebugPS;
    ID3D11Texture2D* m_pRenderTexture;
    ID3D11ShaderResourceView* m_pRenderTextureResource;
    ID3D11RenderTargetView*	m_pRenderTextureTargetView;
    
    ID3D11ShaderResourceView* m_pColorMapResource;
    ID3D11Resource* m_pColorMap;
    
    ID3D11SamplerState* m_pColorMapSampler;
    ID3D11BlendState* m_pBlendState;

    matrix m_viewMatrix;
    matrix m_projMatrix;
    FBXImporter::FBXModel m_TankModel;
    FBXImporter::FBXModel m_QuadModel;
    
    AssetsHelper* m_assetsHelper;
    
    std::vector<ID3D11Resource**> m_resources;
};
