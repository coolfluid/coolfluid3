// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_LogStrampFilter_hpp
#define cf3_common_LogStrampFilter_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string/replace.hpp>

#include "common/BoostIostreams.hpp"

#include "common/CodeLocation.hpp"
#include "common/StringConversion.hpp"
#include "common/PE/Comm.hpp"

#include "common/CommonAPI.hpp"

namespace cf3 {
namespace common {

class CodeLocation;

////////////////////////////////////////////////////////////////////////////////

/// @brief Prepends a stamp to the log messages.

/// This class is written to act as a Boost.Iostreams filter. It defines
/// @c char_type and @c category types and a @c #write method for that purpose.@n
/// The stamp can be personalized with @c #setStamp(). Four tags are regonized:
/// @li @c \%time%: Timestamp (seconds elapsed since the application has been
/// launched).
/// @li @c \%place%: The code location, as given by @c #CodeLocation::strShort()
/// @li @c \%type%: The stream name
/// @li @c \%rank%: The MPI process rank
///
/// @author Quentin Gasper

class Common_API LogStampFilter
{
public:
  
  typedef char char_type;
  typedef boost::iostreams::multichar_output_filter_tag category;
  
  /// @brief Constructor
  
  /// @param streamName The stream name.
  /// @param stamp The stamp. Can be empty.
  LogStampFilter(const std::string & streamName,
                 const std::string & stamp = std::string());
  
  /// @brief Sets stamp.
  
  /// @param stamp The stamp. Can be empty.
  void setStamp(const std::string & stamp);
  
  /// @brief Gives the stamps
  
  /// @return Returns the stamp.
  std::string getStamp() const;
  
  /// @brief Sets the code location
  
  /// @param place The code location
  void setPlace(const CodeLocation & place);
  
  /// @brief Ends message
  
  /// After calling this method, the stamp will be prepended again on the next
  /// call to @c #write.
  void endMessage();
  
  /// @brief Writes data to a sink.
  
  /// If it is a new message, the stamp is prepended.
  
  /// @return Returns @c size or (@c size + the size of the stamp) if the
  /// stamp has been prepended.
  template<typename Sink>
  std::streamsize write(Sink& sink, const char_type * data, std::streamsize size)
  {
    bool ok = true;
    std::string stamp;
    std::streamsize writtenBytes = 0;
    
    if(m_newMessage)
    {
      stamp = m_stamp;
      
      boost::algorithm::replace_all(stamp, "%time%", "TIME");
      boost::algorithm::replace_all(stamp, "%type%", m_streamName);
      boost::algorithm::replace_all(stamp, "%place%", m_place.short_str());
      boost::algorithm::replace_all(stamp, "%rank%", to_str( PE::Comm::instance().rank() ));
      
      m_newMessage = false;
      
      writtenBytes += this->write(sink, stamp.c_str(), stamp.length());
    }
    
    for(int counter = 0 ; counter < size && ok ; counter++)
      ok = boost::iostreams::put(sink, *data++);
    
    return size + writtenBytes;
  }
  
private:
  
  /// @brief The current code location
  CodeLocation m_place;
  
  /// @brief The current stamp
  std::string m_stamp;
  
  /// @brief The stream name
  std::string m_streamName;
  
  /// @brief Indicates whether it is a new message or not.
  
  /// If @c true, the stamp will be prepended on the next call of @c #write.
  /// It can be set back to @c true by calling @c #endMessage()
  bool m_newMessage;
  
}; // class LogStampFilter

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_LogStrampFilter_hpp
