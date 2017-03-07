/// \file timer.h
/// \brief Interface for the timer class CTimer.

#pragma once

#include "Defines.h"

/// \brief The timer.
///
/// The timer allows you to manage game events by duration, rather than
/// on a frame-by-frame basis. This simple version of the timer is based on
/// the Windows API function timeGetTime, which is notoriously inaccurate
/// but perfectly adequate for a simple game demo. The timer can
/// be set to step mode, in which case time is paused, and
/// advanced by a fixed amount when asked.
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

class CTimer{ 
  private:
    int m_nStartTime; ///< Time that timer was started.
    int m_nCurrentTime; ///< Current time.

    int m_nLastFrameStartTime; ///< Start time for previous frame.
    int m_nFrameTime; ///< Elapsed time for previous frame.

    int m_nLevelStartTime; ///< Time the current level started.
    int m_nLevelFinishTime; ///< Time the current level finished.
    BOOL m_bLevelTimerOn; ///< Whether the current level is being timed.
    BOOL m_bStepMode; ///< Whether in step mode.

  public:
    CTimer(); ///< Constructor.

    void start(); ///< Start the timer.
    int time(); ///< Return the time in ms.
    BOOL elapsed(int &start ,int interval); ///< Has interval ms elapsed since start?

    int frametime(); ///< Return the time for last frame in ms.

    //begin and end of frame functions
    void beginframe(); ///< Beginning of animation frame.
    void endframe(); ///< End of animation frame.

    //step mode functions
    void ToggleStepMode(); /// Toggle step mode.
    void IncrementFrame(); /// Increment time artificially by 1/60 of a second.

    //level timer functions
    int GetLevelStartTime(); ///< Get time that the level started.
    int GetLevelElapsedTime(); ///< Get elapsed time since the level started.
    void StartLevelTimer(); ///< Start the level timer.
    void StopLevelTimer(); ///< Stop the level timer.
}; //CTimer