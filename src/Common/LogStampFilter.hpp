#ifndef CF_Common_LogStrampFilter_hpp
#define CF_Common_LogStrampFilter_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/BoostIostreams.hpp"

#include "Common/CodeLocation.hpp"
#include "Common/StringOps.hpp"
#include "Common/MPI/PE.hpp"

#include "Common/CommonAPI.hpp"

namespace CF {
namespace Common {

class StringOps;
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
      boost::algorithm::replace_all(stamp, "%rank%", StringOps::to_str( PE::getInstance().get_rank() ));
      
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

} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_LogStrampFilter_hpp
