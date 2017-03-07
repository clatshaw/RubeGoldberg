/// \file Shader.h 
/// \brief Interface for the shader class CShader.

#pragma once

#include <D3Dcompiler.h>

#include "defines.h"

/// \brief The shader class.
///
/// The shader class takes care of the management and operation
/// of the vertex and pixel shaders.
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

class CShader{
  private:
    D3D11_INPUT_ELEMENT_DESC* m_pIEDesc; ///< Input element descriptor array.
    int m_nMaxDescs; ///< Size of input element descriptor array.
    int m_nNumDescs; ///< Next empty slot in input element descriptor array.

    ID3D11InputLayout* m_pInputLayout; ///< Input layout pointer.
  
    ID3D11VertexShader* m_pVertexShader; ///< Vertex shader pointer.
    ID3D11PixelShader* m_pPixelShader; ///< Pixel shader pointer.

  public:
    CShader(int n); ///< Constructor.
    ~CShader(); ///< Destructor.

    bool AddInputElementDesc(UINT offset, DXGI_FORMAT fmt, LPCSTR name); ///< Add an input element descriptor to the array.

    bool VSCreateAndCompile(ID3D11Device2* pDev2, LPCWSTR fileName, LPCSTR entryPoint); ///< Create and compile vertex shader.
    bool PSCreateAndCompile(ID3D11Device2* pDev2, LPCWSTR fileName, LPCSTR entryPoint);///< Create and compile pixel shader.

    void SetShaders(ID3D11DeviceContext2* pDC2); ///< Set shaders.
}; //CShader