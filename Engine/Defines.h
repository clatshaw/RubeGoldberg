/// \file defines.h
/// \brief Essential game engine defines. 
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

//windows
#include <windows.h>
#include <windowsx.h>

//DirectX
#include <d3d11_2.h>
#include <dxgi1_3.h>
#include <DirectXMath.h>

#include "SimpleMath.h"
#include "tinyxml2.h"

//Name spaces
using namespace DirectX;
using namespace SimpleMath;
using namespace tinyxml2;

//Game engine
#include "timer.h"

/// \brief Safe release of a pointer to a Windows COM object.
///
/// If the pointer is not null, then release what it is pointing
/// to and set it to null. Beware the extra parentheses in this define. 
/// They are there for a reason. 

#define SAFE_RELEASE(p) if(p){(p)->Release(); (p) = nullptr;}

/// \brief Safe delete of a pointer. 
///
/// Note that it is OK to delete a null pointer. Note the brackets and braces also.

#define SAFE_DELETE(p) {delete (p); (p) = nullptr;}

/// \brief Billboard vertex structure. 
///
/// Custom vertex format for representing a vanilla billboard object. Consists 
/// of position and texture coordinates. This is pretty much the smallest, simplest
/// vertex structure that you should ever use. Other things that can go in here when
/// you use a nontrivial lighting model include vertex normals.

struct BILLBOARDVERTEX{ 
  Vector3 p; ///< Position.
  float tu; ///< Texture U coordinate.
  float tv; ///< Texture V coordinate.
}; //BILLBOARDVERTEX

/// \brief Constant buffer for use by vertex shaders.
///
/// The world-view-projection matrix is required by DirectX.
/// The others are uv offsets into a sprite sheet, which will
/// be used for the text sprite.

struct ConstantBuffer{
  XMFLOAT4X4 wvp; ///< World view projection matrix.

  float u0; ///< Texture u offset for left side.
  float u1; ///< Texture u offset for right side.
  float v0; ///< Texture v offset for top.
  float v1; ///< Texture v offset for bottom.
}; //ConstantBuffer