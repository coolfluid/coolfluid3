// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_LogLevelFilter_hpp
#define CF_Common_LogLevelFilter_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/BoostIostreams.hpp"

#include <iostream>

#include "Common/CommonAPI.hpp"
#include "Common/LogLevel.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// @brief Filters log messages corresponding to their level.

/// This class is written to act as a Boost.Iostreams filter. It defines
/// @c char_type and @c category types and a @c #write method for that purpose.@n
/// The class manages two levels: current level and default level. The current
/// level is the one of the current message. It is compared against the default
/// level to determine whether the current message has to be forwarded:
///
/// @li if the default level is @c #SILENT, nothing is forwarded
/// @li if the default level is @c #NORMAL, only @c #NORMAL messages are forwarded
/// @li if the default level is @c #SILENT, @c #NORMAL and @c #VERBOSE messages
/// are forwarded.@n
///
/// @author Quentin Gasper

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
  void setLogLevel(LogLevel level);

  /// @brief Gives the default log level.

  /// @return Returns the default log level.
  LogLevel getLogLevel() const;

  /// @brief Sets the current log level.

  /// @param level The current log level.
  void setCurrentLogLevel(LogLevel level);

  /// @brief Gives the current log level.

  /// @return Returns the current log level.
  LogLevel getCurrentLogLevel() const;

  /// @brief Resets the current log level to the default level.

  /// Calling this method is equivalent to:
  /// @code filter.setCurrentLogLevel(filter.getLogLevel()); @endcode
  void resetToDefaultLevel();

  /// @brief Forwards a message.

  /// @li If the default level is @c #SILENT, nothing is forwarded
  /// @li If the default level is @c #NORMAL, only @c #NORMAL messages are forwarded
  /// @li If the default level is @c #SILENT, @c #NORMAL and @c #VERBOSE messages
  /// @param sink The sink to which the message has to be written.
  /// @param data Message data.
  /// @param size Message data size.

  /// @return Always returns @c size.
  template<typename Sink>
    std::streamsize write(Sink& sink, const char_type * data, std::streamsize size)
  {
    bool ok = m_currentLogLevel != SILENT && m_currentLogLevel >= m_logLevel;

    for(int counter = 0 ; counter < size && ok ; counter++)
    ok = boost::iostreams::put(sink, *data++);

    return size;
  }

  private:

  /// @brief The default log level.
  LogLevel m_logLevel;

  /// @brief The current log level.
  LogLevel m_currentLogLevel;

  }; // struct LogLevelFilter

////////////////////////////////////////////////////////////////////////////////

} // Common

} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_LogLevelFilter_hpp
