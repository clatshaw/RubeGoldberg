/// \file Shader.cpp
/// \brief Code for the Shader class CShader.
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

#include "Shader.h"
#include "defines.h"
#include "Abort.h"
#include "debug.h"

/// The constructor initializes member variables and creates the  
/// input element descriptor array.
/// \param n Size of the input element descriptor array.

CShader::CShader(int n){
  m_pIEDesc = new D3D11_INPUT_ELEMENT_DESC[n];
  m_nMaxDescs = n;
  m_nNumDescs = 0;
  m_pVertexShader = nullptr;
  m_pPixelShader = nullptr;
} //constructor

CShader::~CShader(){
  delete [] m_pIEDesc;

  SAFE_RELEASE(m_pInputLayout);
  SAFE_RELEASE(m_pVertexShader);
  SAFE_RELEASE(m_pPixelShader);
} //destructor

/// Add an input element descriptor to the input element descriptor array.
/// \param offset Aligned byte offset.
/// \param fmt Color format.
/// \param name Semantic name.
/// \return true if succeeded, false if array full.

bool CShader::AddInputElementDesc(UINT offset, DXGI_FORMAT fmt, LPCSTR name){
  if(m_nNumDescs < m_nMaxDescs){
    m_pIEDesc[m_nNumDescs].AlignedByteOffset = offset;
    m_pIEDesc[m_nNumDescs].Format = fmt;
    m_pIEDesc[m_nNumDescs].InputSlot = 0;
    m_pIEDesc[m_nNumDescs].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    m_pIEDesc[m_nNumDescs].InstanceDataStepRate = 0;
    m_pIEDesc[m_nNumDescs].SemanticIndex = 0;
    m_pIEDesc[m_nNumDescs].SemanticName = name;

    m_nNumDescs++;

    return true;
  } //if
  else return false;
} //AddInputElementDesc

/// Create and compile a vertex shader.
/// Calls the Abort macro if there is an error.
/// \param pDev2 Pointer to a D3D11Device2.
/// \param fileName Name of file containing vertex shader.
/// \param entryPoint Name of function to call in that file.

bool CShader::VSCreateAndCompile(ID3D11Device2* pDev2, LPCWSTR fileName, LPCSTR entryPoint){
  HRESULT hr; //error code
  ID3DBlob* code = nullptr; //shader code
  ID3DBlob* errorMsgs = nullptr; //error messages
  
  //compile
  hr = D3DCompileFromFile(fileName, 0, 0, entryPoint, "vs_4_0",  0, 0, &code, &errorMsgs);

  if(FAILED(hr)){ //failed
    if(errorMsgs){ //there were error messages associated with the failure
      ABORT("Vertex shader error: %s\n", (char*)errorMsgs->GetBufferPointer()); //report them
      errorMsgs->Release(); //clean up
    } //if
    return false; //bail out
  } //if

  //load
  hr = pDev2->CreateVertexShader(code->GetBufferPointer(),
    code->GetBufferSize(), nullptr, &m_pVertexShader);

  if(FAILED(hr)){ //failed
    if(errorMsgs){ //there were error messages associated with the failure
      ABORT("Vertex shader error: %s\n", (char*)errorMsgs->GetBufferPointer()); //report them
      errorMsgs->Release(); //clean up
    } //if
    return false; //bail out
  } //if
  
  hr = pDev2->CreateInputLayout(m_pIEDesc, m_nMaxDescs,
    code->GetBufferPointer(), code->GetBufferSize(), &m_pInputLayout);

  //clean up
  SAFE_RELEASE(code);
  SAFE_RELEASE(errorMsgs);

  return true; //success
} //VSCreateAndCompile

/// Create and compile a pixel shader.
/// Calls the Abort macro if there is an error.
/// \param pDev2 Pointer to a D3D11Device2.
/// \param fileName Name of file containing pixel shader.
/// \param entryPoint Name of function to call in that file.

bool CShader::PSCreateAndCompile(ID3D11Device2* pDev2, LPCWSTR fileName, LPCSTR entryPoint){
  HRESULT hr; //error code 
  ID3DBlob* code; //shader code
  ID3DBlob* errorMsgs; //error messages

  //compile
  hr = D3DCompileFromFile(fileName, 0, 0, entryPoint, "ps_4_0", 
    0, 0, &code, &errorMsgs);

  if(FAILED(hr)){ //failed
    if(errorMsgs){ //there were error messages associated with the failure
      ABORT("Pixel shader error: %s\n", (char*)errorMsgs->GetBufferPointer()); //report them
      errorMsgs->Release(); //clean up
    } //if
    return false; //bail out
  } //if

  //load
  hr = pDev2->CreatePixelShader(code->GetBufferPointer(), 
    code->GetBufferSize(), nullptr, &m_pPixelShader);

  if(FAILED(hr)){ //failed
    if(errorMsgs){ //there were error messages associated with the failure
      ABORT("Pixel shader error: %s\n", (char*)errorMsgs->GetBufferPointer()); //report them
      errorMsgs->Release(); //clean up
    } //if
    return false; //bail out
  } //if

  //clean up
  SAFE_RELEASE(code);
  SAFE_RELEASE(errorMsgs);

  return true; //success
} //PSCreateAndCompile

/// Set the renderer's vertex and pixel shaders.
/// This is intended to be called many times a frame, if necessary,
/// and assumes that everything has been correctly initialized. For
/// speed it does no error checking except to a null pointed check
/// on the device context pointer provided as a parameter.
/// \param pDC2 Pointer to a D3D11 device context 2.

void CShader::SetShaders(ID3D11DeviceContext2* pDC2){
  if(pDC2){
    pDC2->IASetInputLayout(m_pInputLayout);
    pDC2->VSSetShader(m_pVertexShader, nullptr, 0);
    pDC2->PSSetShader(m_pPixelShader, nullptr, 0);
  } //if
} //SetShaders