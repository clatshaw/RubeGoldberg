/// \file main.cpp 
/// \brief Main file for Ian Parberry's game engine.
///
/// This file contains some custom Windows code for creating, maintaining,
/// and closing down the game window. It consists of some initialization code,
/// a default window procedure, and a default wwinmain. The real ones
/// will be in your game project, but they should mostly just call the
/// default ones here. The idea is to hide all of the nasty, messy,
/// horrible Windows details to a place where the normal user won't
/// see it and be scared by it.
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

#include <windows.h>
#include <windowsx.h>
#include <process.h>

#include "defines.h"
#include "abort.h"
#include "tinyxml2.h"
#include "debug.h"
#include "Sound.h"

#ifdef DEBUG_ON
  CDebugManager g_cDebugManager; ///< The debug manager.
#endif //DEBUG_ON

BOOL g_bActiveApp;  ///< TRUE if this is the active application.
HWND g_HwndApp; ///< Application window handle.
HINSTANCE g_hInstance; ///< Application instance handle.
extern char g_szGameName[]; ///< Name of this game.

const char* g_xmlFileName="gamesettings.xml"; ///< Settings file name.
tinyxml2::XMLDocument g_xmlDocument; ///< TinyXML document for settings.
XMLElement* g_xmlSettings = nullptr; ///< TinyXML element for settings tag.
  
CSoundManager* g_pSoundManager; ///< The sound manager.
CTimer g_cTimer; ///< The game timer.

int g_nScreenWidth; ///< Screen width in pixels.
int g_nScreenHeight; ///< Screen height in pixels.

//function prototypes from MyGame.cpp

void ProcessFrame(); ///< Process an animation frame.
void InitGame(); ///< Game initialization.
void EndGame(); ///< Game termination clean-up.
BOOL KeyboardHandler(WPARAM keystroke); ///< Keyboard handler
LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam); ///< Window procedure.

/// \brief Initialize XML settings.
/// Opens an XML file and prepares to read settings from it. Settings
/// tag is loaded to XML element g_xmlSettings for later processing. Aborts if it
/// cannot load the file or cannot find settings tag in loaded file.

void InitXMLSettings(){
  //open and load XML file
  const char* xmlFileName = "gamesettings.xml"; //Settings file name.
  if(g_xmlDocument.LoadFile(xmlFileName) != 0)
    ABORT("Cannot load settings file %s.", g_xmlFileName);

  //get settings tag
  g_xmlSettings = g_xmlDocument.FirstChildElement("settings"); //settings tag
  if(!g_xmlSettings) //abort if tag not found
    ABORT("Cannot find <settings> tag in %s.", g_xmlFileName);
} //InitXMLSettings


/// \brief Default window procedure.
/// This is the handler for messages from the operating system. 
/// \param hwnd window handle
/// \param message message code
/// \param wParam parameter for message 
/// \param lParam second parameter for message
/// \return 0 if message is handled

LRESULT CALLBACK DefaultWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
  switch(message){ //handle message
    case WM_ACTIVATEAPP: 
      g_bActiveApp = (BOOL)wParam; 
      break; 

    case WM_KEYDOWN: //keyboard hit
      if(KeyboardHandler(wParam))DestroyWindow(hwnd);
      break;

    case WM_DESTROY: //on exit
      EndGame();
      PostQuitMessage(0); //this is the last thing to do on exit
      break;

    default: //default window procedure
      return DefWindowProc(hwnd, message, wParam, lParam);
  } //switch(message)
  return 0;
} //WindowProc

/// \brief Create a default window.
/// Register and create a window. Care is taken to ensure that the
/// client area of the window is the right size, because the default
/// way of creating a window has you specify the width and height
/// including the frame.
///  \param name the name of this application
///  \param hInstance handle to the current instance of this application
///  \param nCmdShow specifies how the window is to be shown
///  \return handle to the application window

HWND CreateDefaultWindow(char* name, HINSTANCE hInstance, int nCmdShow){
  WNDCLASS wc; //window registration info
  //fill in registration information wc
  wc.style = CS_HREDRAW|CS_VREDRAW; //style
  wc.lpfnWndProc = WindowProc; //window message handler
  wc.cbClsExtra = wc.cbWndExtra = 0;
  wc.hInstance = hInstance; //instance handle
  wc.hIcon = LoadIcon(hInstance, 0); //icon
  wc.hCursor = nullptr; //no cursor
  wc.hbrBackground = nullptr; //we will draw background
  wc.lpszMenuName = nullptr; //no menu
  wc.lpszClassName = name; //app name provided as parameter

  //register window
  RegisterClass(&wc);

  //create and set up window 
  HWND hwnd;
  RECT r;  
  r.left = 0; r.right = g_nScreenWidth; 
  r.top = 0; r.bottom = g_nScreenHeight;

  DWORD dwStyle = WS_CAPTION | WS_MINIMIZEBOX | WS_THICKFRAME | WS_SYSMENU; 
  DWORD dwStyleEx = WS_EX_APPWINDOW | WS_EX_DLGMODALFRAME;

  AdjustWindowRectEx(&r, dwStyle, FALSE, dwStyleEx); 

  hwnd = CreateWindowEx(dwStyleEx, name, name, dwStyle, 0, 0,
    r.right-r.left, r.bottom-r.top, nullptr, nullptr, hInstance, nullptr);

  //center window on screen
	int x = (GetSystemMetrics(SM_CXSCREEN) - g_nScreenWidth)/2;
	int y = (GetSystemMetrics(SM_CYSCREEN) - g_nScreenHeight)/2;
	::SetWindowPos(hwnd, nullptr, x, y, r.right - r.left, r.bottom - r.top,
	  SWP_NOZORDER | SWP_SHOWWINDOW);

  //This is what we'd have to do to make it fullscreen instead of
  //windowed, which of course we are not. FYI.
  //hwnd = CreateWindowEx(WS_EX_TOPMOST, name, name, WS_POPUP, 0, 0,
  //  GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
  //  nullptr, nullptr, hInstance, nullptr);

  if(hwnd){ //if successfully created window
    ShowWindow(hwnd, nCmdShow); UpdateWindow(hwnd); //show and update
    SetFocus(hwnd); //get input from keyboard
  } //if

  return hwnd; //return window handle
} //CreateDefaultWindow

/// Loads essential game settings from an xml file.

void LoadGameSettings(){
  if(!g_xmlSettings)return; //bail and fail

  //get game name
  XMLElement* ist = g_xmlSettings->FirstChildElement("game"); 
  if(ist){
    int len = strlen(ist->Attribute("name")); //length of name string
    strncpy_s(g_szGameName, len+1, ist->Attribute("name"), len); 
  } //if

  //get renderer settings
  XMLElement* renderSettings = 
    g_xmlSettings->FirstChildElement("renderer"); //renderer tag
  if(renderSettings){ //read renderer tag attributes
    g_nScreenWidth = renderSettings->IntAttribute("width");
    g_nScreenHeight = renderSettings->IntAttribute("height");
  } //if

  //get debug settings
  #ifdef DEBUG_ON
    g_cDebugManager.GetDebugSettings(g_xmlSettings);
  #endif //DEBUG_ON
} //LoadGameSettings
              
/// \brief Default WinMain.  
/// The main entry point for this application should call this function first. 
///  \param hInstance handle to the current instance of this application
///  \param hPrevInstance unused
///  \param lpCmdLine unused 
///  \param nCmdShow specifies how the window is to be shown
///  \return TRUE if application terminates correctly

int DefaultWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow){
  #ifdef DEBUG_ON
    g_cDebugManager.open(); //open debug streams, settings came from XML file
  #endif //DEBUG_ON

  g_hInstance = hInstance;
  InitXMLSettings(); //initialize XML settings reader
  LoadGameSettings();

  g_HwndApp = CreateDefaultWindow(g_szGameName, hInstance, nCmdShow); //create fullscreen window
  if(!g_HwndApp)return FALSE; //bail if problem creating window
  
  g_cTimer.start(); //start game timer

  InitGame(); //initialize the game

  g_pSoundManager = new CSoundManager(); //create sound manager
  g_pSoundManager->Load(); //load the sounds for this game

  //Play background music
  g_pSoundManager->loop(8);

  //message loop
  MSG msg; //current message
  while(TRUE)
    if(PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE)){ //if message waiting
      if(!GetMessage(&msg, nullptr, 0, 0))return (int)msg.wParam; //get it
      TranslateMessage(&msg); DispatchMessage(&msg); //translate it
    }
    else 
      if(g_bActiveApp)ProcessFrame();
      else WaitMessage();
} //WinMain