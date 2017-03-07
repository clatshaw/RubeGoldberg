/// \file sprite.h
/// \brief Interface for the sprite class C3DSprite.

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

#pragma once

#include "Defines.h"
#include "Shader.h"
#include "WICTextureLoader.h"

/// \brief The sprite. 
///
/// The sprite class contains basic information for drawing a
/// billboard sprite in Render World.

class C3DSprite{ 
  friend class CSpriteManager;

  protected:
    ID3D11ShaderResourceView* m_pTexture; ///< Pointer to texture containing the sprite image.

    ID3D11Buffer* m_pVertexBuffer; ///< Vertex buffer.
    BILLBOARDVERTEX* m_pVertexBufferData; ///< Vertex buffer data.

    ID3D11Device2* m_pDev2; ///< D3D device.
    ID3D11DeviceContext2* m_pDC2; ///< Device context.
    
    void LoadTexture(char* fname, int* w, int* h);

  public:
    C3DSprite(ID3D11Device2* pDev2, ID3D11DeviceContext2* pDC2); ///< Constructor.
    ~C3DSprite(); ///< Destructor.

    BOOL Load(char* filename); ///< Load texture image from file.
    void Draw(); ///< Draw sprite.
    void Release(); ///< Release sprite.
}; //C3DSprite