#include "Common/StringOps.hpp"
#include "Common/TimePolicies.hpp"
#include "Common/HourMinSec.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Common {

////////////////////////////////////////////////////////////////////////////////

#ifdef CF_HAVE_SYS_RESOURCE_H

TimePolicy_rusage::TimePolicy_rusage() : m_start(0.), m_stop(0.) {}

void TimePolicy_rusage::initStartTime()
{
  m_start = seconds();
}

void TimePolicy_rusage::takeStopTime()
{
  m_stop = seconds();
}

void TimePolicy_rusage::accumulateTime(CF::Real& accTime)
{
  accTime += (m_stop - m_start);
}

CF::Real TimePolicy_rusage::getDelta() const
{
  return (seconds() - m_start);
}

CF::Real TimePolicy_rusage::seconds() const
{
  rusage usg;
  getrusage(RUSAGE_SELF, &usg);
  timeval usertime = usg.ru_utime;
  return (usertime.tv_sec + (usertime.tv_usec * HourMinSec::usecPerSec));
}

#endif // CF_HAVE_SYS_RESOURCE_H

////////////////////////////////////////////////////////////////////////////////

#if defined (CF_HAVE_TIME_H) || defined (CF_HAVE_SYS_TIME_H)

TimePolicy_cclock::TimePolicy_cclock() : m_start(0.), m_stop(0.) {}

void TimePolicy_cclock::initStartTime()
{
  m_start = seconds();
}

void TimePolicy_cclock::takeStopTime()
{
  m_stop = seconds();
}

void TimePolicy_cclock::accumulateTime(CF::Real& accTime)
{
  accTime += (m_stop - m_start);
}

CF::Real TimePolicy_cclock::getDelta() const
{
  return (seconds() - m_start);
}

CF::Real TimePolicy_cclock::seconds() const
{
  // not so accurate but very portable
  const CF::Real secs_per_tick = 1.0 / CLOCKS_PER_SEC;
  return ( static_cast<CF::Real>(clock()) ) * secs_per_tick;
}

#endif // CF_HAVE_TIME_H || CF_HAVE_SYS_TIME_H

////////////////////////////////////////////////////////////////////////////////

#ifdef CF_HAVE_GETTIMEOFDAY

TimePolicy_gettimeofday::TimePolicy_gettimeofday()
{
  gettimeofday(&m_start,0);
  m_stop = m_start;
}

void TimePolicy_gettimeofday::initStartTime()
{
  gettimeofday(&m_start,0);
}

void TimePolicy_gettimeofday::takeStopTime()
{
  gettimeofday(&m_stop,0);
}

void TimePolicy_gettimeofday::accumulateTime(CF::Real& accTime)
{
  timeval diff;
  subTimeval(diff, m_stop, m_start);
  accTime += toDouble(diff);
}

CF::Real TimePolicy_gettimeofday::getDelta() const
{
  timeval diff, now;
  gettimeofday (&now, 0);
  bool nega = subTimeval(diff, now, m_start);
  return (nega ? toDouble(diff) * -1 : toDouble(diff));
}

CF::Real TimePolicy_gettimeofday::toDouble(const timeval & result) const
{
  return (result.tv_sec + static_cast<CF::Real>(result.tv_usec / 1000000.0L));
}

bool TimePolicy_gettimeofday::subTimeval (timeval & Res, const timeval & X1, const timeval & Y1) const
{
  timeval X = X1;
  timeval Y = Y1;
  if (X.tv_usec < Y.tv_usec)
  {
      int nsec = (Y.tv_usec - X.tv_usec) / 1000000;
      Y.tv_usec += 1000000 * nsec;
      Y.tv_sec -= nsec;
  }
  if (X.tv_usec - Y.tv_usec > 1000000)
  {
      int nsec = (Y.tv_usec - X.tv_usec) / 1000000;
      Y.tv_usec += 1000000 * nsec;
      Y.tv_sec -= nsec;
  }
  Res.tv_sec = X.tv_sec - Y.tv_sec;
  Res.tv_usec = X.tv_usec - Y.tv_usec;

  return X.tv_sec < Y.tv_sec;
}

#endif // CF_HAVE_GETTIMEOFDAY

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace CF

