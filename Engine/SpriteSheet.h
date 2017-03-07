/// \file spritesheet.h 
/// \brief Definition of the sprite sheet class CSpriteSheet.
///
/// A sprite sheet consists of a single frame containing all of the
/// sprite images. They are drawn from selecting a bounding rectangle
/// around the actual frame by adjusting the texture coordinates in
/// the vertex shader.
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

#pragma once

#include "Sprite.h"

/// \brief The Sprite Sheet.
///
/// The sprite sheet enables you to read in a single image file
/// and draw a selected rectangle from that image.

class CSpriteSheet: public C3DSprite{
  friend class CRenderer;

  private:
    int m_nWidth; ///< Sprite width in pixels.
    int m_nHeight; ///< Sprite height in pixels.
    int m_nFrameWidth; ///< Width of the sprite frame.
    int m_nFrameHeight; ///< Height of the sprite frame.

  public:
    CSpriteSheet(ID3D11Device2* pDev2, ID3D11DeviceContext2* pDC2, int width, int height); ///< Constructor.
    BOOL Load(char* filename); ///< Load the sprite sheet.
}; //CSpriteSheet