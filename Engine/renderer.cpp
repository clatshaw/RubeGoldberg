/// \file renderer.cpp
/// \brief Direct3D rendering.
///
/// DirectX stuff that shouldn't change much is hidden away in this file
/// so you won't have to keep looking at it.
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

#include "renderer.h"
#include "defines.h"
#include "abort.h"
#include "SpriteMan.h"
#include "tinyxml2.h"
#include "Shader.h"
#include "debug.h"

using namespace tinyxml2;

extern int g_nScreenWidth;
extern int g_nScreenHeight;
extern HWND g_HwndApp;
extern XMLElement* g_xmlSettings;

CRenderer::CRenderer(): 
  m_pDev2(nullptr), m_pScreenText(nullptr)
{
  m_matWorld = XMMatrixIdentity();
  m_matView = XMMatrixIdentity();
  m_matProj = XMMatrixIdentity();
} //constructor

/// All textures used in the game are released - the release function is kind
/// of like a destructor for DirectX entities, which are COM objects.

void CRenderer::Release(){ 
  m_cSpriteManager.Release(); 
  SAFE_DELETE(m_pScreenText);

  SAFE_RELEASE(m_pRasterizerState);
  SAFE_RELEASE(m_pBackgroundTexture);
  SAFE_RELEASE(m_pBackgroundVB);
  SAFE_RELEASE(m_pBlendState); 
} //Release

/// Create a D3D device and swap chain, set the graphics mode, create a vertex buffer 
/// for the background.
/// \param hwnd Window handle.
/// \return TRUE if it succeeded.

BOOL CRenderer::Initialize(HWND hwnd){ 
  HRESULT hr; //result from D3D function calls

  //create d3d device and swap chain for mode
  hr = CreateD3DDeviceAndSwapChain(hwnd);
  if(FAILED(hr)){ //bail if failed
    m_pDev2 = nullptr; return FALSE;
  } //if
  m_cSpriteManager.SetDevice(m_pDev2, m_pDC2);

  //graphics settings
  if(FAILED(CreateDepthBuffer()))
    return FALSE;
  if(FAILED(CreateRasterizer()))
    return FALSE;
  CreateViewport();
  
  //transformation matrices
  float w = g_nScreenWidth/2.0f;
  float h = g_nScreenHeight/2.0f;

  m_matView = XMMatrixLookAtLH(Vector3(w, h, -350.0f), Vector3(w, h, 1000.0f), Vector3(0, 1, 0));
  m_matProj = XMMatrixOrthographicOffCenterLH(-w, w, -h, h, 1.0f, 10000.0f);

  //set the blend state, to be used by sprites that have an alpha-channel
  D3D11_BLEND_DESC1 blendDesc;
  blendDesc.AlphaToCoverageEnable = FALSE;
  blendDesc.IndependentBlendEnable = FALSE;
  blendDesc.RenderTarget[0].BlendEnable = TRUE;
  blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
  blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
  blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
  blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
  blendDesc.RenderTarget[0].LogicOp = D3D11_LOGIC_OP_CLEAR;
  blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
  blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
  blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
  blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;

  hr = m_pDev2->CreateBlendState1(&blendDesc, &m_pBlendState);
  
  m_nScreenWidth = g_nScreenWidth;
  m_nScreenHeight = g_nScreenHeight;

  return TRUE; //success exit
} //Initialize

/// Initialize screen text by creating a sprite sheet and reading it
/// in from a file specified in gamesettings.xml.

void CRenderer::InitScreenText(){
  m_pScreenText = new CSpriteSheet(m_pDev2, m_pDC2, 21, 37);

  //load text image from information in gamesettings.xml
  char buffer[256];
  XMLElement* imagesTag = g_xmlSettings->FirstChildElement("images"); 

  if(imagesTag){
    XMLElement* imageSubTag = imagesTag->FirstChildElement("image");    

    while(imageSubTag && strcmp("text", imageSubTag->Attribute("name")))
      imageSubTag = imageSubTag->NextSiblingElement("image");

    if(imageSubTag){ //got "image" tag with right name
      int len = strlen(imageSubTag->Attribute("src")); //length of name string
      strncpy_s(buffer, 256, imageSubTag->Attribute("src"), len); //copy file name string
      buffer[len] = '\0'; //NULL at end of string
      m_pScreenText->Load(buffer);
    } //if

    else LoadTexture(m_pBackgroundTexture, "No text file specified in gamesettings.xml");
  } // if 
} //InitScreenText

/// Set the world matrix m_matWorld to a scale, followed by a rotation, then a translation.
/// \param v Translation vector.
/// \param angle Orientation angle, rotation about the Z axis.
/// \param xsize Scale in the x direction.
/// \param ysize Scale in the y direction.

void CRenderer::SetWorldMatrix(const Vector3& v, const float angle, float xsize, float ysize){
  m_matWorld = XMMatrixScaling(xsize, ysize, 1.0f) * XMMatrixRotationZ(angle) * XMMatrixTranslation(v.x, v.y, v.z);
} //SetWorldMatrix

/// Compose the world, view, and projection transformations.
/// Of course, transformations are composed by multiplying their matrices.
/// \return Product of the world, view, and projection matrices.

XMFLOAT4X4 CRenderer::CalculateWorldViewProjectionMatrix(){
  XMFLOAT4X4 f;
  XMStoreFloat4x4(&f, XMMatrixTranspose(m_matWorld * m_matView * m_matProj));
  return f;
} //CalculateWorldViewProjectionMatrix

/// Initialize the vertex and constant buffers for the background.

void CRenderer::InitBackground(){
  HRESULT hr;

  //load vertex buffer
  float w = (float)g_nScreenWidth;
  float h = (float)g_nScreenHeight;
  
  //vertex information, first triangle in clockwise order
  BILLBOARDVERTEX pVertexBufferData[4]; 
  pVertexBufferData[0].p = Vector3(w, 0, 1500);
  pVertexBufferData[0].tu = 1.0f; pVertexBufferData[0].tv = 1.0f;

  pVertexBufferData[1].p = Vector3(0, 0, 1500);
  pVertexBufferData[1].tu = 0.0f; pVertexBufferData[1].tv = 1.0f;

  pVertexBufferData[2].p = Vector3(w, h, 1500);
  pVertexBufferData[2].tu = 1.0f; pVertexBufferData[2].tv = 0.0f;

  pVertexBufferData[3].p = Vector3(0, h, 1500);
  pVertexBufferData[3].tu = 0.0f; pVertexBufferData[3].tv = 0.0f;
  
  //create vertex buffer for background
  m_pShader = new CShader(2);
    
  m_pShader->AddInputElementDesc(0, DXGI_FORMAT_R32G32B32_FLOAT, "POSITION");
  m_pShader->AddInputElementDesc(12, DXGI_FORMAT_R32G32_FLOAT,  "TEXCOORD");
  m_pShader->VSCreateAndCompile(m_pDev2, L"VertexShader.hlsl", "main");
  m_pShader->PSCreateAndCompile(m_pDev2, L"PixelShader.hlsl", "main");
    
  // Create constant buffers.
  D3D11_BUFFER_DESC constantBufferDesc = { 0 };
  constantBufferDesc.ByteWidth = sizeof(ConstantBuffer);
  constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
  constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  constantBufferDesc.CPUAccessFlags = 0;
  constantBufferDesc.MiscFlags = 0;
  constantBufferDesc.StructureByteStride = 0;
    
  m_pDev2->CreateBuffer(&constantBufferDesc, nullptr, &m_pConstantBuffer);
    
  D3D11_BUFFER_DESC VertexBufferDesc;
  VertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
  VertexBufferDesc.ByteWidth = sizeof(BILLBOARDVERTEX)* 6;
  VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  VertexBufferDesc.CPUAccessFlags = 0;
  VertexBufferDesc.MiscFlags = 0;
  VertexBufferDesc.StructureByteStride = 0;
    
  D3D11_SUBRESOURCE_DATA subresourceData;
  subresourceData.pSysMem = pVertexBufferData;
  subresourceData.SysMemPitch = 0;
  subresourceData.SysMemSlicePitch = 0;
    
  hr = m_pDev2->CreateBuffer(&VertexBufferDesc, &subresourceData, &m_pBackgroundVB);
} //InitBackground

/// Load the background image after getting the file name from g_xmlSettings.

void CRenderer::LoadBackground(){
  char buffer[256];
  //image file names
  XMLElement* imagesTag = g_xmlSettings->FirstChildElement("images"); 
  if(imagesTag){
    XMLElement* imageSubTag = imagesTag->FirstChildElement("image");     
    while(imageSubTag && strcmp("background", imageSubTag->Attribute("name")))
      imageSubTag = imageSubTag->NextSiblingElement("image");
    if(imageSubTag){ //got "image" tag with right name
      int len = strlen(imageSubTag->Attribute("src")); //length of name string
      strncpy_s(buffer, 256, imageSubTag->Attribute("src"), len); //copy file name string
      buffer[len] = '\0'; //NULL at end of string
      LoadTexture(m_pBackgroundTexture, buffer);
    } //if image tag with correct name found
    else LoadTexture(m_pBackgroundTexture, "No background file specified in gamesettings.xml");
  } // if images tag found
} //LoadBackground

/// Draw the background, which is a simple image.

void CRenderer::DrawBackground(){
  SetWorldMatrix(Vector3(0.0f));

  UINT nVertexBufferOffset = 0;
  
  UINT nVertexBufferStride = sizeof(BILLBOARDVERTEX);
  m_pDC2->IASetVertexBuffers(0, 1, &m_pBackgroundVB, &nVertexBufferStride, &nVertexBufferOffset);
  m_pDC2->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
  m_pShader->SetShaders(m_pDC2);
  m_pDC2->PSSetShaderResources(0, 1, &m_pBackgroundTexture);  
  
  ConstantBuffer constantBufferData; 
  constantBufferData.wvp = CalculateWorldViewProjectionMatrix();
  constantBufferData.u0 = 0.0f;
  constantBufferData.u1 = 1.0f;
  constantBufferData.v0 = 0.0f;
  constantBufferData.v1 = 1.0f;
  m_pDC2->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &constantBufferData, 0, 0);
  m_pDC2->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
  m_pDC2->Draw(4, 0);
} //DrawBackground

/// Initialize the rendering pipeline for the start of a new animation frame.
/// Set the render target, and clear the depth buffer.

void CRenderer::BeginFrame(){
  m_pDC2->OMSetRenderTargets(1, &m_pRTV, m_pDSV);
  m_pDC2->ClearDepthStencilView(m_pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
} //BeginFrame

/// End of frame processing, meaning that we preent the rendered image in video
/// memory to the monitor screen. This is currently chained to vertical retrace to
/// limit the animation to 60fps max. If you unchain it,
/// bad things happen to the physics engine in the relaxation demo.
/// (Fixing that is left as an exercise for the reader.)

void CRenderer::EndFrame(){
  m_pSwapChain2->Present(2, 0); //present frame
} //EndFrame

/// Load a sprite using the Sprite Manager to do the heavy lifting.
/// \param sprite Index of sprite in Sprite Manager.
/// \param name Name of sprite in GameSettings.xml

void CRenderer::Load(int sprite, char* name){ 
  m_cSpriteManager.Load(sprite, name);
} //Load

/// Load an image from a file into a D3D texture. 
/// \param v Pointer to D3D texture to receive the image
/// \param fname Name of the file containing the texture
/// \param w Pointer to a variable that receives the texture width
/// \param h Pointer to a variable that receives the texture width 

void CRenderer::LoadTexture(ID3D11ShaderResourceView* &v, char* fname, int* w, int* h){
  wchar_t  ws[100];
  swprintf(ws, 100, L"%hs", fname);
  CreateWICTextureFromFile(m_pDev2, m_pDC2, ws, nullptr, &v, 0);

  //get texture width and height
  ID3D11Resource* r;
  D3D11_TEXTURE2D_DESC desc;
  v->GetResource(&r);
  ((ID3D11Texture2D*)r)->GetDesc(&desc);

  if(w)*w = desc.Width;
  if(h)*h = desc.Height;
} //LoadTexture

/// Draw a sprite, using the Sprite Manager to do the heavy lifting.
/// \param sprite Index of sprite in Sprite Manager.
/// \param s Position.
/// \param angle Orientation in radians.
/// \param xsize Width of sprite, 1.0 is actual size.
/// \param ysize Height of sprite, 1.0 is actual size.

void CRenderer::draw(int sprite, const Vector3& s, float angle, float xsize, float ysize){   
	SetWorldMatrix(Vector3(s.x, s.y, s.z), angle, xsize, ysize);
	XMFLOAT4X4 m = CalculateWorldViewProjectionMatrix();
    
  m_pShader->SetShaders(m_pDC2);

  m_pDC2->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
  m_pDC2->OMSetBlendState(m_pBlendState, nullptr, 0xffffffff);
  
  ConstantBuffer constantBufferData; 
  constantBufferData.wvp = CalculateWorldViewProjectionMatrix();
  constantBufferData.u0 = 0.0f;
  constantBufferData.u1 = 1.0f;
  constantBufferData.v0 = 0.0f;
  constantBufferData.v1 = 1.0f;
  m_pDC2->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &constantBufferData, 0, 0);
  m_pDC2->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);

  m_cSpriteManager.Draw(sprite);
} //draw

/// Draw a sprite, using the Sprite Manager to do the heavy lifting. This time the vertical
/// coordinate is measured down from top of screen.
/// \param sprite Index of sprite in Sprite Manager.
/// \param s Position.
/// \param angle Orientation in radians.
/// \param size Size of sprite, 1.0 is actual size.

void CRenderer::drawtop(int sprite, Vector3 s, float angle, float size){ 
  s.y = g_nScreenHeight - s.y;
  draw(sprite, s, angle, size);
} //draw

/// Draw a character specified by its position in the sprite sheet to a point
/// on the screen.
/// \param s Position in pixel coordinates. s.x gets incremented by the width of one character.
/// \param y Position of the top of the character in the sprite sheet.
/// \param xoffset Number of characters to skip in the current row of the sprite sheet.

void CRenderer::DrawCharFromSpriteSheet(Vector3& s, int y, int xoffset){   
  SetWorldMatrix(Vector3(s.x, s.y, s.z));
  XMFLOAT4X4 m = CalculateWorldViewProjectionMatrix();
    
  m_pShader->SetShaders(m_pDC2);

  m_pDC2->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
  m_pDC2->OMSetBlendState(m_pBlendState, nullptr, 0xffffffff);
  
  ConstantBuffer constantBufferData; 
  constantBufferData.wvp = CalculateWorldViewProjectionMatrix();

  const float w = (float)m_pScreenText->m_nWidth;
  const float h = (float)m_pScreenText->m_nHeight;
  const float fw = (float)m_pScreenText->m_nFrameWidth;
  const float fh = (float)m_pScreenText->m_nFrameHeight;

  int x = 1 + xoffset*(m_pScreenText->m_nFrameWidth + 1);

  //compute the uv offsets
  constantBufferData.u0 = (float)x/(w - 1);
  constantBufferData.u1 = (float)(x + fw)/(w - 1.0f);
  constantBufferData.v0 = (float)y/(h - 1);
  constantBufferData.v1 = (float)(y + fh)/(h - 1.0f);
    
  m_pDC2->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &constantBufferData, 0, 0);
  m_pDC2->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);

  m_pScreenText->Draw();
  s.x += m_pScreenText->m_nFrameWidth; //move rightwards for next character
} //DrawCharFromSpriteSheet

/// Draw a text string centered on the screen. The string must consist of digits,
/// upper and lower case letters, and spaces.
/// \param text The text string to be drawn to the screen.

void CRenderer::drawtext(const char* text){  
  //center the text on the screen
  Vector3 s = Vector3((m_nScreenWidth - strlen(text)*m_pScreenText->m_nFrameWidth)/2.0f,
    m_nScreenHeight/2.0f, -1000.0f);

  //draw character by character
  for(unsigned int i=0; i<strlen(text); i++){
    char c = text[i]; //current character

    //determine c's y coordinate and x offset (in chars) in the sprite sheet.
    int xoffset=0, y=0; //default values, should never be used

    if(c >='A' && c <= 'Z'){ //upper case alphabetic
      xoffset = c - 'A'; y = 48;
    } //if
    else if(c >='a' && c <= 'z'){ //lower  case alphabetic
      xoffset = c - 'a'; y = 95; 
    } //else if
    else if(c >='0' && c <= '9'){ //numeric
      xoffset = c - '0'; y = 0; 
    } //else if
    else{ //space by default
      xoffset = 10; y = 1;
    } //else

    //draw the character
    DrawCharFromSpriteSheet(s, y, xoffset);
  } //for
} //drawtext

/// Get the world width and height, which for these demos is the same
/// as the screen width and height, but it doesn't necessarily have to
/// be that way, of course.
/// \param w Width will be returned here.
/// \param h Height will be returned here.

void CRenderer::GetWorldSize(int &w, int &h){
  w = m_nScreenWidth; h = m_nScreenHeight;
} //GetWorldSize

/// Set and initialize the device, device context, and swap chain; and
/// assign them to the member pointers m_pDev2, m_pDC2, and m_pSwapChain2.
/// \param hwnd Window handle.
/// \return The error code returned by D3D.

HRESULT CRenderer::CreateD3DDeviceAndSwapChain(HWND hwnd){ 
  //set up device specs data structure
  DXGI_SWAP_CHAIN_DESC d3dscd; //Swap chain specifications.
  ZeroMemory(&d3dscd, sizeof(DXGI_SWAP_CHAIN_DESC)); //null it out
  d3dscd.BufferCount = 1;
  d3dscd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //color mode
  d3dscd.BufferDesc.Height = g_nScreenHeight; //height
  d3dscd.BufferDesc.RefreshRate.Denominator = 1;
  d3dscd.BufferDesc.RefreshRate.Numerator = 60;
  d3dscd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
  d3dscd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  d3dscd.BufferDesc.Width = g_nScreenWidth; //width
  d3dscd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  d3dscd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  d3dscd.OutputWindow = hwnd;
  d3dscd.SampleDesc.Count = 1;
  d3dscd.SampleDesc.Quality = 0;
  d3dscd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
  d3dscd.Windowed = true;

  //now we can create the device and swap chain
  
  IDXGISwapChain* pSwapChain = nullptr; //Swap chain.
  ID3D11Device* pDev = nullptr; //D3D device.
  ID3D11DeviceContext* pDC = nullptr; //Device context.
  D3D_FEATURE_LEVEL fl; //Feature level.

  HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE,
    nullptr, 0, nullptr, 0, D3D11_SDK_VERSION,
    &d3dscd, &pSwapChain, &pDev, &fl, &pDC);
  
  m_pDev2 = (ID3D11Device2*)pDev;
  m_pDC2 = (ID3D11DeviceContext2*)pDC;
  m_pSwapChain2 = (IDXGISwapChain2*)pSwapChain;

  ID3D11Texture2D* backBuffer = nullptr;
  
  hr = pSwapChain->ResizeBuffers(1, g_nScreenWidth, g_nScreenHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
  hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
  hr = m_pDev2->CreateRenderTargetView(backBuffer, 0, &m_pRTV);
  SAFE_RELEASE(backBuffer);

  return hr;
} //CreateD3DDeviceAndSwapChain

/// Create the Z or depth buffer using a depth stencil.
/// This function assumes that m_pDev2 has been initialized.
/// \return The error code returned by D3D, 1 if m_pDev2 is NULL

HRESULT CRenderer::CreateDepthBuffer(){ 
  if(!m_pDev2)return 1; //bail and fail

  D3D11_TEXTURE2D_DESC depthStencilDesc;
  depthStencilDesc.Width = g_nScreenWidth;
  depthStencilDesc.Height = g_nScreenHeight;
  depthStencilDesc.MipLevels = 1;
  depthStencilDesc.ArraySize = 1;
  depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depthStencilDesc.SampleDesc.Count = 1; // multisampling must match
  depthStencilDesc.SampleDesc.Quality = 0; // swap chain values.
  depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
  depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  depthStencilDesc.CPUAccessFlags = 0;
  depthStencilDesc.MiscFlags = 0;

  ID3D11Texture2D* depthStencilBuffer = nullptr;
  m_pDSV = nullptr;
  m_pDev2->CreateTexture2D(&depthStencilDesc, 0, &depthStencilBuffer);
  m_pDev2->CreateDepthStencilView(depthStencilBuffer, 0, &m_pDSV);

  m_pDC2->OMSetRenderTargets(1, &m_pRTV, m_pDSV);
  
  ID3D11DepthStencilState* pDepthStencilState = nullptr;

  D3D11_DEPTH_STENCIL_DESC dhStencilDesc;

  //Depth test parameters
  dhStencilDesc.DepthEnable = true;
  dhStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  dhStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

  //Stencil test parameters
  dhStencilDesc.StencilEnable = false;
  dhStencilDesc.StencilReadMask = 0xFF;
  dhStencilDesc.StencilWriteMask = 0xFF;

  //Stencil operations if pixel is front-facing
  dhStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  dhStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
  dhStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  dhStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

  //Stencil operations if pixel is back-facing.
  dhStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  dhStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
  dhStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  dhStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

  HRESULT hr = m_pDev2->CreateDepthStencilState(&dhStencilDesc, &pDepthStencilState);
  m_pDC2->OMSetDepthStencilState(pDepthStencilState, 1);

  return hr;
} //CreateDepthBuffer

/// Create the rasterizer and attach it to the device context.
/// This function assumes that m_pDev2 has been initialized.
/// \return The error code returned by D3D, 1 if m_pDev2 is NULL

HRESULT CRenderer::CreateRasterizer(){
  if(!m_pDev2)return 1; //bail and fail

  ZeroMemory(&m_rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC1));
  m_rasterizerDesc.CullMode = D3D11_CULL_BACK;
  m_rasterizerDesc.FillMode = D3D11_FILL_SOLID;
  m_rasterizerDesc.FrontCounterClockwise = false;

  m_pRasterizerState = nullptr;
  HRESULT hr = m_pDev2->CreateRasterizerState1(&m_rasterizerDesc, &m_pRasterizerState);
  m_pDC2->RSSetState(m_pRasterizerState);

  return hr;
} //CreateRasterizer

/// Create the viewport and attach it to the device context.
/// This function assumes that m_pDev2 has been initialized.

void CRenderer::CreateViewport(){
  if(!m_pDev2)return; //bail and fail
  
  D3D11_VIEWPORT vp;
  vp.Height = (float)g_nScreenHeight;
  vp.MaxDepth = 1.0f;
  vp.MinDepth = 0.0f;
  vp.TopLeftX = 0;
  vp.TopLeftY = 0;
  vp.Width = (float)g_nScreenWidth;

  m_pDC2->RSSetViewports(1, &vp);
} //CreateViewport
