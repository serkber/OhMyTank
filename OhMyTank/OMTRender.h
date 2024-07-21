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
        float mouseX;
        float square;
        float grassfieldSize;
        float grassPos;
        float grassAspect;
        float camPos;
        float mouseY;
    };
    
friend class OMTGame;
    
public:
    OMTRender();
    ~OMTRender();
    bool SetupRenderTexture(ID3D11Texture2D** renderTexture, ID3D11ShaderResourceView** resourceView, ID3D11RenderTargetView** renderTarget);
    void InitGrass(ID3D11RenderTargetView** renderTarget);
    bool CreateTextureAndResource(UINT width, UINT height, ID3D11Texture2D** texture, ID3D11ShaderResourceView** resource);
    bool LoadGraphicContent();
    bool CreateConstantBuffers();
    void CreateBlendingResources();
    bool CreateTextureSampler();
    void SetRenderViewport(float width, float height);
    void RenderGrass(float grassPos, ID3D11ShaderResourceView** grassTexture);
    void RenderGrassTexture(ID3D11RenderTargetView** renderTarget, matrix grassMatrix, bool isCurrentGrass);
    void UpdateCameraBuffers(ID3D11DeviceContext* context);
    void CreateDebugTextureMatrix();
    bool CreateGrassInstancesBuffer();
    void Render();
    void ReleaseResources();

private:
    //Post Processing
    ID3D11Texture2D* m_pPostpoTexture;
    ID3D11ShaderResourceView* m_pPostpoTextureResource;
    ID3D11PixelShader* m_pVBlurPS;
    
    //Common resources
    ID3D11InputLayout* m_pBasicInputLayout;
    ID3D11InputLayout* m_pInstantiableInputLayout;
    ID3D11SamplerState* m_pTextureSampler;
    ID3D11BlendState* m_pAlphaBlendState;
    ID3D11BlendState* m_pMultiplyBlendState;
    
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
    ID3D11Buffer* m_pInstancesBufferGrassField;
    ID3D11Buffer* m_pGrassBuffers[2];

    //Render texture stuff
    double renderTextureTimer = 0;
    ID3D11VertexShader* m_pRenderTextureVS;
    ID3D11PixelShader* m_pRenderTexturePS;
    ID3D11PixelShader* m_pGrassInitPS;
    ID3D11VertexShader* m_pTextureDebugVS;
    ID3D11PixelShader* m_pTextureDebugPS;
    ID3D11Texture2D* m_pRenderTexture1;
    ID3D11ShaderResourceView* m_pRenderTexture1Resource;
    ID3D11RenderTargetView*	m_pRenderTexture1TargetView;
    ID3D11Texture2D* m_pRenderTexture2;
    ID3D11ShaderResourceView* m_pRenderTexture2Resource;
    ID3D11RenderTargetView*	m_pRenderTexture2TargetView;
    bool m_isPatchShiftFrame = false;
    ID3D11VertexShader* m_pFullScreenVS;
    int m_currentGrass = 0;
    
    matrix m_debugTextureMatrix;

    //Quad resources
    FBXImporter::FBXModel m_quadModel;
    ID3D11Buffer* m_pVertexBufferQuad;
    ID3D11Buffer* m_pIndexBufferQuad;

    matrix m_viewMatrix;
    matrix m_projMatrix;

    //CBuffers
    ID3D11Buffer* m_pViewCB;
    ID3D11Buffer* m_pProjCB;
    ID3D11Buffer* m_pModelCB;
    ID3D11Buffer* m_pDataCB;

    DataCB m_data;
    
    AssetsHelper m_assetsHelper;
    std::vector<ID3D11Resource**> m_resources;
};