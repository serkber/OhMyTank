#pragma once
#include <d3d11.h>
#include "AssetsHelper.h"
#include "FBXImporter.h"
#include "Typedefs.h"

#define GRASS_FIELD_SIZE 90  //Must be divisible by 3
#define GRASS_SECTION_SIZE 30  //Third of the thing on top
#define RENDER_TEXTURE_SIZE 1024
#define PI 3.14159265359
#define HALF_PI 1.57079632679

class OMTRender
{
    struct DataCB
    {
        float d_elapsedTime;
        float2 d_mousePos;
        float d_grassfieldSize;
        float2 d_grassfieldPos;
        float2 d_grassScroll;
        float2 d_tankPos;
        float2 d_tankDir;
    };    

    struct VisibleGrassSections
    {
        int sections[3];
    };
    
friend class OMTGame;
    
public:
    OMTRender();
    ~OMTRender();
    bool SetupRenderTexture(ID3D11Texture2D** renderTexture, ID3D11ShaderResourceView** resourceView, ID3D11RenderTargetView** renderTarget);
    void InitGrass(ID3D11DeviceContext* context, ID3D11RenderTargetView** renderTarget);
    bool CreateTextureAndResource(UINT width, UINT height, ID3D11Texture2D** texture, ID3D11ShaderResourceView** resource);
    bool CreateTwoSidedRasterState();
    bool LoadGraphicContent();
    bool CreateConstantBuffers();
    void CreateBlendingResources();
    bool CreateTextureSampler();
    void SetRenderViewport(float width, float height);
    void UpdateCameraBuffers(ID3D11DeviceContext* context);
    void RenderTexture(ID3D11DeviceContext* context, matrix* grassMatrix, bool* isScrollFrame);
    DirectX::XMMATRIX GetGrassfieldMatrix();
    void CreateDebugTextureMatrix();
    bool CreateGrassInstancesBuffer();
    void RenderGrassSection(ID3D11DeviceContext* context, DirectX::XMVECTOR sectionPosition);
    void RenderGrassField(ID3D11DeviceContext* context);
    bool CheckGrassSectionVisibility(VisibleGrassSections visibleSections, int currentSection);
    void Render();
    void ReleaseResources();

private:
    //Post Processing
    ID3D11Texture2D* m_pPostpoTexture1;
    ID3D11ShaderResourceView* m_pPostpoTexture1Resource;
    ID3D11Texture2D* m_pPostpoTexture2;
    ID3D11ShaderResourceView* m_pPostpoTexture2Resource;
    ID3D11PixelShader* m_pSilhouettePS;
    ID3D11PixelShader* m_pBlurPS;
    ID3D11PixelShader* m_pFXAAPS;
    
    //Common resources
    ID3D11InputLayout* m_pBasicInputLayout;
    ID3D11InputLayout* m_pInstantiableInputLayout;
    ID3D11SamplerState* m_pTextureSampler;
    ID3D11BlendState* m_pAlphaBlendState;
    ID3D11BlendState* m_pCustomBlendState;
    ID3D11RasterizerState* m_pRasterStateTwoSided;
    
    //Tank Resources    
    FBXImporter::FBXModel m_tankModel;
    ID3D11ShaderResourceView* m_pTankMapResource;
    ID3D11Resource* m_pTankMap;
    ID3D11VertexShader* m_pBasicVS;
    ID3D11PixelShader* m_pTankPS;
    ID3D11Buffer* m_pVertexBufferTank;
    ID3D11Buffer* m_pIndexBufferTank;

    //Grassfield Resources
    FBXImporter::FBXModel m_grassPatchModel;
    ID3D11VertexShader* m_pGrassFieldVS;
    ID3D11PixelShader* m_pGrassFieldPS;
    ID3D11PixelShader* m_pClearChannelsPS;
    ID3D11PixelShader* m_pGroundPS;
    ID3D11Buffer* m_pVertexBufferGrassPatch;
    ID3D11Buffer* m_pIndexBufferGrassPatch;
    ID3D11Buffer* m_pInstancesBufferGrassPatch;
    ID3D11Buffer* m_pGrassBuffers[2];
    const float m_maxDeltaBeforeScroll = 0.1f;

    //Render texture stuff
    ID3D11PixelShader* m_pTextureScrollPS;
    ID3D11VertexShader* m_pTextureScrollVS;
    double renderTextureTimer = 0;
    ID3D11VertexShader* m_pRenderTextureVS;
    ID3D11PixelShader* m_pRenderTexturePS;
    ID3D11PixelShader* m_pRenderTextureDirPS;
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