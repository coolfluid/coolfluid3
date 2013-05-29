// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_LogLevelFilter_hpp
#define cf3_common_LogLevelFilter_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/BoostIostreams.hpp"

#include "common/CommonAPI.hpp"
#include "common/LogLevel.hpp"

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

/// @brief Filters log messages corresponding to their level.

/// This class is written to act as a Boost.Iostreams filter. It defines
/// @c char_type and @c category types and a @c #write method for that purpose.@n
/// The class is constructed with a filter value.
/// An additional level can be set that gets compared to the filter. If the level
/// is higher or equal to the filter, the message gets forwarded. @n
/// A temporary override of the level is possible, using set_tmp_log_level(), which
/// gets restored by calling resetToDefaultLevel()
///
/// Example:@n
/// If the filter is set to only pass messages of @c #WARNING, and the level is set to:
/// - @c #DEBUG,   then it is blocked
/// - @c #INFO,    then it is blocked
/// - @c #WARNING, then it is forwarded
/// - @c #ERROR,   then it is forwarded
///
/// @author Quentin Gasper
/// @author Willem Deconinck

class Common_API LogLevelFilter
{
  public:

  typedef char char_type;
  typedef boost::iostreams::multichar_output_filter_tag category;

  /// @brief Constructor

  /// @param level Default log level.
  LogLevelFilter(LogLevel level);

  /// @brief Sets the default log level.

  /// @param level The new default level.
  void set_filter(LogLevel level);

  /// @brief Gives the default log level.

  /// @return Returns the default log level.
  LogLevel get_filter() const;


  void set_tmp_log_level(const Uint level);

  /// @brief Sets the current log level.

  /// @param level The current log level.
  void set_log_level(const Uint level);

  /// @brief Gives the current log level.

  /// @return Returns the current log level.
  Uint get_log_level() const;

  /// @brief Resets the current log level to the default level.

  /// Calling this method is equivalent to:
  /// @code filter.setCurrentLogLevel(filter.getLogLevel()); @endcode
  void resetToDefaultLevel();

  /// @brief Forwards a message.

  /// Example:@n
  /// If the filter is set to only pass messages of @c #WARNING, and the level is set to:
  /// - @c #DEBUG,   then it is blocked
  /// - @c #INFO,    then it is blocked
  /// - @c #WARNING, then it is forwarded
  /// - @c #ERROR,   then it is forwarded
  /// @param sink The sink to which the message has to be written.
  /// @param data Message data.
  /// @param size Message data size.

  /// @return Always returns @c size.
  template<typename Sink>
    std::streamsize write(Sink& sink, const char_type * data, std::streamsize size)
  {
    bool ok = m_tmp_log_level >= static_cast<Uint>(m_filter);

    for(int counter = 0 ; counter < size && ok ; counter++)
    ok = boost::iostreams::put(sink, *data++);

    return size;
  }

  private:

  /// @brief The filtering value for the log level
  LogLevel m_filter;

  /// @brief The default log level
  Uint m_log_level;

  /// @brief A temporary override of the log level
  Uint m_tmp_log_level;

  }; // struct LogLevelFilter

////////////////////////////////////////////////////////////////////////////////

} // common

} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_LogLevelFilter_hpp
