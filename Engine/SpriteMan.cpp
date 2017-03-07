/// \file SpriteMan.cpp
/// \brief Code for the sprite manager class CSpriteManager.
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

#include <stdio.h>

#include "SpriteMan.h"
#include "abort.h"

using namespace tinyxml2;

extern XMLElement* g_xmlSettings; //global XML settings

CSpriteManager::CSpriteManager(){ 
  m_pDev2 = nullptr; 
  m_pDC2 = nullptr; 
  for(int i=0; i<NUM_SPRITES; i++)
    m_pSprite[i] = nullptr; 
} //constructor

CSpriteManager::~CSpriteManager(){ 
  for(int i=0; i<NUM_SPRITES; i++)
    SAFE_DELETE(m_pSprite[i]); 
} //destructor

/// Save pointer to D3D device in a private member variable for later use.
/// We will need them later to create sprites. This isn't part of the constructor
/// because the sprite manager might be created before DirectX has been fired up.
/// \param pDev2 Pointer to a D3D11Device2.
/// \param pDC2 Pointer to a D3D11DeviceContext2.

void CSpriteManager::SetDevice(ID3D11Device2* pDev2, ID3D11DeviceContext2* pDC2){
  m_pDev2 = pDev2; 
  m_pDC2 = pDC2; 
} //SetDevice

/// Load sprite from file.
/// \param sprite Sprite type.
/// \param file Image file name.
/// \return TRUE if load succeeded

BOOL CSpriteManager::LoadFile(int sprite, const char* file){
  m_pSprite[sprite] = new C3DSprite(m_pDev2, m_pDC2); //get space in array for new sprite
  sprintf_s(m_pBuffer, MAX_PATH, "%s", file); //assemble file name
  return m_pSprite[sprite]->Load(m_pBuffer); //load from that file name
} //LoadFile

/// Load sprite from info in XML settings.
/// Loads information about the sprite from global variable g_xmlSettings, then
/// loads the sprite images as per that information. Aborts if something goes wrong.
/// \param sprite sprite type
/// \param name sprite name in XML file

void CSpriteManager::Load(int sprite, char* name){
  BOOL success = FALSE; //TRUE if loaded successfully

  if(g_xmlSettings){ //got "settings" tag
    //get "sprites" tag
    XMLElement* spriteSettings = g_xmlSettings->FirstChildElement("sprites"); //sprites tag
    if(spriteSettings){ //got "sprites" tag
      XMLElement* spr; //sprite element
      spr = spriteSettings->FirstChildElement("sprite");
      while(spr && strcmp(name, spr->Attribute("name")))
        spr = spr->NextSiblingElement("sprite");
      if(spr) //got "sprite" tag with right name
        success = LoadFile(sprite, spr->Attribute("file"));
    } //if
  } //if

  if(!success) //fail
    ABORT("Cannot load sprite \"%s\".\n",name);
} //Load

/// Draw a sprite to the back buffer.
/// \param sprite Sprite type.

void CSpriteManager::Draw(int sprite){ 
  if(m_pSprite[sprite])
    m_pSprite[sprite]->Draw();
} //Draw

/// Release textures and vertex buffers from all sprites.

void CSpriteManager::Release(){ 
  for(int i=0; i<NUM_SPRITES; i++)
    if(m_pSprite[i])m_pSprite[i]->Release();
} //Release