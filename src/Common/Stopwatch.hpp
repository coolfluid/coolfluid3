// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_Stopwatch_hpp
#define CF_Common_Stopwatch_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/HourMinSec.hpp"
#include "Common/TimePolicies.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// Performance Stopwatch  used for benchmarking codes.
/// Measure elapsed seconds
/// @author Tiago Quintino
template <typename TIMEPOLICY = Common::WallTime>
class Stopwatch {
public:

  /// Constructor
  Stopwatch();

  /// Destructor
  ~Stopwatch();

  /// Start timing from 0.00.
  /// @post m_running == true
  /// @post m_total == 0
  void start();

  /// Restart timing from 0.00. Same as calling start()
  /// @post m_running == true
  /// @post m_total == 0
  void restart();

  /// Reset the timer
  /// Clears the elapsed time.
  /// @post m_running == false
  /// @post m_total == 0
  void reset();

  /// Resumes counting. Doesn't clear the elapsed time.
  /// No effect if isRunning()
  /// @post m_running == true
  /// @post m_total >= 0
  void resume();

  /// Stop timing. Doesn't clear the elapsed time.
  /// No effect if isNotRunning()
  /// @post m_running == false
  /// @post m_total >= 0
  void stop();

  /// Read current time.
  /// @return current time in seconds
  CF::Real read() const;

  /// Converst the current time to CF::Real
  /// @return current time in seconds
  operator CF::Real () const { return read(); }

  /// Read current time.
  /// @return current time in Hour, Minutes and Seconds
  HourMinSec readTimeHMS();

  /// Checks if the Stopwatch is m_running
  /// @return TRUE if it is running
  bool isRunning() const;

  /// Checks if the Stopwatch is not running
  /// @return TRUE if it is not running
  bool isNotRunning() const;

private:

  /// Initializes the starting time
  void initStartTime();

  /// Takes the stoping time
  void takeStopTime();

  /// Adds the elapsed time to the accumulated time
  /// @post m_total is updated
  void accumulateTime();

private:

  /// flag to record if it is running
  bool   m_running;

  /// the accumulated time
  CF::Real m_total;

  /// the way to count the time
  TIMEPOLICY impl;

}; // Class Stopwatch

////////////////////////////////////////////////////////////////////////////////

template <typename TIMEPOLICY>
Stopwatch<TIMEPOLICY>::Stopwatch() : m_running(false)
{
}

////////////////////////////////////////////////////////////////////////////////

template <typename TIMEPOLICY>
Stopwatch<TIMEPOLICY>::~Stopwatch()
{
}

////////////////////////////////////////////////////////////////////////////////

template <typename TIMEPOLICY>
void Stopwatch<TIMEPOLICY>::restart()
{
  start();
}

////////////////////////////////////////////////////////////////////////////////

template <typename TIMEPOLICY>
void Stopwatch<TIMEPOLICY>::reset()
{
  stop();
  m_total = 0.;
}

////////////////////////////////////////////////////////////////////////////////

template <typename TIMEPOLICY>
void Stopwatch<TIMEPOLICY>::start()
{
  reset();
  resume();
}

////////////////////////////////////////////////////////////////////////////////

template <typename TIMEPOLICY>
void Stopwatch<TIMEPOLICY>::stop()
{
  if (isRunning()) {
    takeStopTime();
    accumulateTime();
    m_running = false;
  }
}

////////////////////////////////////////////////////////////////////////////////

template <typename TIMEPOLICY>
void Stopwatch<TIMEPOLICY>::resume()
{
  if (isNotRunning()) {
    initStartTime();
    m_running = true;
  }
}

////////////////////////////////////////////////////////////////////////////////

template <typename TIMEPOLICY>
CF::Real Stopwatch<TIMEPOLICY>::read() const
{
  if (isNotRunning())
    return m_total;

  return impl.getDelta() + m_total;
}

////////////////////////////////////////////////////////////////////////////////

template <typename TIMEPOLICY>
HourMinSec Stopwatch<TIMEPOLICY>::readTimeHMS()
{
  return HourMinSec(read());
}

////////////////////////////////////////////////////////////////////////////////

template <typename TIMEPOLICY>
bool Stopwatch<TIMEPOLICY>::isRunning() const
{
  return m_running;
}

////////////////////////////////////////////////////////////////////////////////

template <typename TIMEPOLICY>
bool Stopwatch<TIMEPOLICY>::isNotRunning() const
{
  return !isRunning();
}

////////////////////////////////////////////////////////////////////////////////

template <typename TIMEPOLICY>
void Stopwatch<TIMEPOLICY>::initStartTime()
{
  impl.initStartTime();
}

////////////////////////////////////////////////////////////////////////////////

template <typename TIMEPOLICY>
void Stopwatch<TIMEPOLICY>::accumulateTime()
{
  impl.accumulateTime(m_total);
}

////////////////////////////////////////////////////////////////////////////////

template <typename TIMEPOLICY>
void Stopwatch<TIMEPOLICY>::takeStopTime()
{
  impl.takeStopTime();
}

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#include "Stopwatch.ci"

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_Stopwatch_hpp

