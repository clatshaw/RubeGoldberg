/// \file SpriteSheet.cpp
/// \brief Code for the sprite sheet class CSpriteSheet.
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

#include "sprite.h"
#include "renderer.h"
#include "debug.h"

#include "SpriteSheet.h"

extern int g_nScreenWidth;

/// \param pDev2 Pointer to a D3D11Device2.
/// \param pDC2 Pointer to a D3D11DeviceContext2.
/// \param width Width of sprite frame.
/// \param height Height of sprite frame.

CSpriteSheet::CSpriteSheet(ID3D11Device2* pDev2, ID3D11DeviceContext2* pDC2, 
  int width, int height): 
  m_nFrameWidth(width), m_nFrameHeight(height),
C3DSprite(pDev2, pDC2){} 

/// Load the sprite image into a texture from a given file
/// name and create a vertex buffer for the billboard image containing 4 
/// corner vertices spaced apart the appropriate width and height.
/// \param filename The name of the image file.
/// \return TRUE for successful load.

BOOL CSpriteSheet::Load(char* filename){
  LoadTexture(filename, &m_nWidth, &m_nHeight);

  HRESULT hr = 0; //error code
  if(m_pVertexBuffer == nullptr){ //create only when first frame is loaded  
    //load vertex buffer
    float w = m_nFrameWidth/2.0f;
    float h = m_nFrameHeight/2.0f;
  
    //vertex information, first triangle in clockwise order
    m_pVertexBufferData[0].p = Vector3(w, h, 0.0f);
    m_pVertexBufferData[0].tu = 1.0f; m_pVertexBufferData[0].tv = 0.0f;
  
    m_pVertexBufferData[1].p = Vector3(w, -h, 0.0f);
    m_pVertexBufferData[1].tu = 1.0f; m_pVertexBufferData[1].tv = 1.0f;
  
    m_pVertexBufferData[2].p = Vector3(-w, h, 0.0f);
    m_pVertexBufferData[2].tu = 0.0f; m_pVertexBufferData[2].tv = 0.0f;
  
    m_pVertexBufferData[3].p = Vector3(-w, -h, 0.0f);
    m_pVertexBufferData[3].tu = 0.0f; m_pVertexBufferData[3].tv = 1.0f;
  
    //create vertex buffer
    D3D11_SUBRESOURCE_DATA subresourceData;
    subresourceData.pSysMem = m_pVertexBufferData;
    subresourceData.SysMemPitch = 0;
    subresourceData.SysMemSlicePitch = 0;

    D3D11_BUFFER_DESC d3d_vbDesc; //vertex buffer descriptor.
    d3d_vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    d3d_vbDesc.ByteWidth = sizeof(BILLBOARDVERTEX)* 4;
    d3d_vbDesc.CPUAccessFlags = 0;
    d3d_vbDesc.MiscFlags = 0;
    d3d_vbDesc.StructureByteStride = 0;
    d3d_vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
    
    hr = m_pDev2->CreateBuffer(&d3d_vbDesc, &subresourceData, &m_pVertexBuffer);
  } //if
  
  return SUCCEEDED(hr); //successful
} //Load