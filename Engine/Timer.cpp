/// \file timer.cpp
/// \brief Code for timer class CTimer.
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

#include "timer.h"

/// The constructor sets member variables to sensible initial values.

CTimer::CTimer(): 
  m_nStartTime(0),  
  m_nLastFrameStartTime(0),
  m_nFrameTime(0),
  m_nLevelStartTime(0), 
  m_nLevelFinishTime(0),
  m_bLevelTimerOn(FALSE),
  m_bStepMode(FALSE)
{};

/// Start the timer.

void CTimer::start(){ 
  m_nStartTime = timeGetTime();
} //start

/// Get current time.
/// \return Current time since start in milliseconds.

int CTimer::time(){ 
  return m_nCurrentTime;
} //time

/// Get last frame time for last frame in milliseconds.

int CTimer::frametime(){ 
  return m_nFrameTime;
} //frametime

/// The elapsed function is a useful function for measuring repeating time intervals.
/// Given the start and duration times, this function returns TRUE if the interval
/// is over, and has the side-effect of resetting the start time when that happens,
/// thus setting things up for the next interval.
/// \param start start of time interval
/// \param interval duration of time interval
/// \return TRUE if time interval is over

int CTimer::elapsed(int &start, int interval){
  if(m_nCurrentTime >= start + interval){ //if interval is over
    start = m_nCurrentTime; return TRUE; //reset the start and succeed
  } //if
  else return FALSE; //otherwise, fail
} //elapsed

/// This is the signal that a new animation frame has begun,
/// so that the timer can return the same time value for the
/// duration of this frame.

void CTimer::beginframe(){ 
  const int t = timeGetTime(); 
  m_nCurrentTime = t - m_nStartTime;
  if(!m_bStepMode)
    m_nFrameTime = t - m_nLastFrameStartTime;
  m_nLastFrameStartTime = t;
} //beginframe

/// This is the signal that an animation frame has ended.

void CTimer::endframe(){ 
  m_nFrameTime = 0;
} //endframe

/// Increment time by 1/60 of a second when in step mode. This is useful
/// when you want to step through frame by frame to 
/// verify that collisions are working correctly. It does nothing if
/// the timer is not in step mode.

void CTimer::IncrementFrame(){
  if(m_bStepMode)
    m_nFrameTime = 34; //1/60 of a second is 16.6666 milliseconds
} //incrementframe

/// Toggle in and out of step mode. In step mode you must step time manually
/// using IncrementFrame().

void CTimer::ToggleStepMode(){
  m_bStepMode = !m_bStepMode;
} //ToggleDebugMode

/// Get the time that the current level started.
/// \return Level start time.

int CTimer::GetLevelStartTime(){
  return m_nLevelStartTime;
} //GetLevelStartTime

/// Get amount of time spent in the current level.
/// \return Level elapsed time.

int CTimer::GetLevelElapsedTime(){
  return m_bLevelTimerOn?
    timeGetTime() - m_nLevelStartTime: 
    m_nLevelFinishTime - m_nLevelStartTime;
} //GetLevelElapsedTime

/// Start the level timer. Should be called at the start of a level.

void CTimer::StartLevelTimer(){
  m_bLevelTimerOn = TRUE;
  m_nLevelStartTime = timeGetTime();
} //StartLevelTimer

/// Stop the level timer. Should be called at the end of a level.

void CTimer::StopLevelTimer(){
  m_bLevelTimerOn = FALSE;
  m_nLevelFinishTime = timeGetTime();
} //StopLevelTimer