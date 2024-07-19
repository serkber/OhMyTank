#pragma once
#include <d3d11.h>
#include "AssetsHelper.h"
#include "FBXImporter.h"
#include "Typedefs.h"

class OMTRender
{
    struct DataCB
    {
        float elapsedTime;
        float padding1;
        float padding2;
        float padding3;
    };
    
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
void RenderGrass();
void Render();
    void ReleaseResources();

private:
    //Common resources
    ID3D11InputLayout* m_pBasicInputLayout;
    ID3D11SamplerState* m_pTextureSampler;
    ID3D11BlendState* m_pAlphaBlendState;
    
    //Tank Resources    
    FBXImporter::FBXModel m_tankModel;
    ID3D11ShaderResourceView* m_pTankMapResource;
    ID3D11Resource* m_pTankMap;
    ID3D11VertexShader* m_pTankVS;
    ID3D11PixelShader* m_pTankPS;
    ID3D11Buffer* m_pVertexBufferTank;
    ID3D11Buffer* m_pIndexBufferTank;

    //Grassfield Resources
    FBXImporter::FBXModel m_grassFieldModel;
    ID3D11VertexShader* m_pGrassFieldVS;
    ID3D11PixelShader* m_pGrassFieldPS;
    ID3D11Buffer* m_pVertexBufferGrassField;
    ID3D11Buffer* m_pIndexBufferGrassField;

    //Quad resources
    FBXImporter::FBXModel m_quadModel;
    ID3D11Buffer* m_pVertexBufferQuad;
    ID3D11Buffer* m_pIndexBufferQuad;

    //CBuffers
    ID3D11Buffer* m_pViewCB;
    ID3D11Buffer* m_pProjCB;
    ID3D11Buffer* m_pModelCB;
    ID3D11Buffer* m_pDataCB;

    //Render texture stuff
    ID3D11VertexShader* m_pRenderTextureVS;
    ID3D11PixelShader* m_pRenderTexturePS;
    ID3D11VertexShader* m_pTextureDebugVS;
    ID3D11PixelShader* m_pTextureDebugPS;
    ID3D11Texture2D* m_pRenderTexture;
    ID3D11ShaderResourceView* m_pRenderTextureResource;
    ID3D11RenderTargetView*	m_pRenderTextureTargetView;

    //Camera stuff
    matrix m_viewMatrix;
    matrix m_projMatrix;
    
    AssetsHelper m_assetsHelper;
    std::vector<ID3D11Resource**> m_resources;
};