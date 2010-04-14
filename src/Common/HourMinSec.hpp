#ifndef CF_Common_HourMinSec_hh
#define CF_Common_HourMinSec_hh

#include "CF.hpp"
#include "CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// This class represents a place holder for some timing data.
/// @author Andrea Lani
/// @author Tiago Quintino
class Common_API HourMinSec {
public:

  /// Default constructor without arguments
  HourMinSec();

  /// Constructor with nb of seconds.
  /// @param sec number of seconds to initializes
  HourMinSec(const CF::Real& sec);

  /// Default destructor
  ~HourMinSec();

  /// Conversion to std::string
  std::string str() const;

  /// Set the values corresponding to the given time (in sec)
  void set(const CF::Real& total);

public:
  /// number of seconds per hour
  static const CF::Real secPerHour;
  /// number of seconds per minute
  static const CF::Real secPerMin;
  /// number of micro seconds per second
  static const CF::Real usecPerSec;

private:
  /// number of hours
  CF::Real m_h;
  /// number of minutes
  CF::Real m_m;
  /// number of seconds
  CF::Real m_s;

}; // end HourMinSec

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_HourMinSec_hh

