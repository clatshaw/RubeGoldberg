/// \file renderer.h 
/// \brief Definition of the renderer class CRenderer.

#pragma once

#include "spriteman.h"
#include "SpriteSheet.h"

#include <D3Dcompiler.h>

#include "WICTextureLoader.h"
#include "defines.h"

/// \brief The renderer.
///
/// Created by Ian Parberry to accompany his book
/// "Introduction to Game Physics with Box2D", published by CRC Press in 2013.
/// Copyright Ian Parberry, Laboratory for Recreational Computing,
/// Department of Computer Science & Engineering, University of North
/// Texas, Denton, TX, USA. URL: http://larc.unt.edu/ian.
///
/// This file is made available under the GNU All-Permissive License.
/// Copying and distribution of this file, with or without
/// modification, are permitted in any medium without royalty
/// provided the copyright notice and this notice are preserved.
/// This file is offered as-is, without any warranty.

class CRenderer{
  friend void InitGame();
  friend class CShader;
  friend class C3DSprite;
  
  protected:
    ID3D11Device2* m_pDev2; ///< D3D device.
    ID3D11DeviceContext2* m_pDC2; ///< Device context.
    IDXGISwapChain2* m_pSwapChain2; ///< Swap chain.

    ID3D11RenderTargetView* m_pRTV; ///< Render target view.
    ID3D11DepthStencilView* m_pDSV; ///< Depth stencil view.

    XMMATRIX m_matWorld; ///< World matrix.
    XMMATRIX m_matView; ///< View matrix.
    XMMATRIX m_matProj; ///< Projection matrix.
    
    ID3D11BlendState1* m_pBlendState; ///< Blend state.

  protected:
    HRESULT CreateD3DDeviceAndSwapChain(HWND hwnd); ///< Create D3d device.
    HRESULT CreateDepthBuffer(); ///< Create Z buffer.
    HRESULT CreateRasterizer(); ///< Create rasterizer.
    void CreateViewport(); ///< Create viewport.

    void SetWorldMatrix(const Vector3& v, const float angle=0.0f, float xsize=1.0f, float ysize=1.0f); ///< Set the world matrix.

    ID3D11RasterizerState1* m_pRasterizerState; ///< Rasterizer state.
    D3D11_RASTERIZER_DESC1 m_rasterizerDesc; ///< Rasterizer description.

  protected:
    ID3D11Buffer* m_pBackgroundVB;  ///< Vertex buffer for background.
    ID3D11ShaderResourceView* m_pBackgroundTexture; ///< Texture for background.
    ID3D11Buffer* m_pConstantBuffer; ///< Constant buffer for shader.
    CShader* m_pShader; ///< Pointer to an instance of the shader class.

    CSpriteManager m_cSpriteManager; ///< Sprite manager.
    CSpriteSheet* m_pScreenText; ///< Screen text sprite sheet.
    int m_nScreenWidth; ///< Screen width in pixels.
    int m_nScreenHeight; ///< Screen height in pixels.

    void DrawCharFromSpriteSheet(Vector3& s, int y, int xoffset); ///< Draw a character to screen.

  public:
    CRenderer(); ///< Constructor.
    BOOL Initialize(HWND hwnd); ///< Initialize Direct3D.
    void InitScreenText(); ///< Initialize screen text using a sprite sheet.
    void InitBackground(); ///< Initialize background.
    XMFLOAT4X4 CalculateWorldViewProjectionMatrix(); ///< Compute product of world, view, and projection matrices. 
    void DrawBackground(); ///< Draw the background to the screen.
    void LoadTexture(ID3D11ShaderResourceView* &v, char* fname,
      int* w=0, int* h=0); ///< Load texture from a file.
    void LoadBackground(); ///< Load background image from a file.
    void BeginFrame(); ///< Begin an animation frame.
    void EndFrame(); ///< End an animation frame.

    void Load(int sprite, char* name); ///< Load sprite.
    void draw(int sprite, const Vector3& s, float angle=0.0f, float xsize=1.0f, float ysize=1.0f); ///< Draw sprite.
    void drawtop(int sprite, Vector3 s, float angle=0.0f, float size=1.0f); ///< Draw sprite at top of screen.
    void drawtext(const char* text); ///< Draw text to screen.
    void GetWorldSize(int &x, int &y); ///< Get the Render World width and height.
    void Release(); ///< Release textures and vertex buffer.
}; //CRenderer