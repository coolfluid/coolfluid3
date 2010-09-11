#ifndef CF_Common_LogStream_hpp
#define CF_Common_LogStream_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/BoostIostreams.hpp"
//#include "Common/CF.hpp"
#include "Common/MPI/PEInterface.hpp"

namespace CF {
namespace Common {

class CodeLocation;
class LogToStream;
class LogLevelFilter;
class LogStampFilter;
class LogStringForwarder;
class PE;

////////////////////////////////////////////////////////////////////////////////

/// @brief Manages a log stream.

/// @author Quentin Gasper
class Common_API LogStream
{
  public:

  /// @brief Special message tag.

  /// For now, the only available tag is @c #ENDLINE.
  enum LogTag
  {
    /// @brief Indicates the end of a message.

    /// This tag must be used to flush stream buffers and must end each log
    /// message to guarantee that the stream buffers are flushed.
    ENDLINE
  };

  /// @brief Message destination
  enum LogDestination
  {
    /// @brief Standard output
    SCREEN = 1,

    /// @brief File
    FILE = 2,

    /// @brief A string buffer
    STRING = 4,

    /// @brief Standard output (with MPI synchronization)
    SYNC_SCREEN = 8
  };


  /// @brief Constructor

  /// @param fileStream The file stream.
  LogStream(const std::string & streamName, LogLevel level = NORMAL);

  /// @brief Destructor.

  /// Frees all allocated memory. The file stream is not destroyed. All
  /// unflushed streams are flushed
  ~LogStream();

  /// @brief Flushes the stream contents.
  void flush();

  /// @brief Overrides operator &lt;&lt; for @c #LogLevel type.

  /// Sets @c #level as current level for all destinations.
  /// @param level The level.
  /// @return Returns a reference to this object.
  LogStream & operator << (LogLevel level);

  /// @brief Overrides operator &lt;&lt; for @c #Logtag type.

  /// If @c tag is @c #ENDLINE, all stream buffers are flushed.
  /// @param tag The tag
  /// @return Returns a reference to this object.
  LogStream & operator << (LogTag tag);

  /// @brief Overrides operator &lt;&lt; for @c #LogLevel type.

  /// Sets @c #codeLoction as current code location for all destinations.
  /// @return Returns a reference to this object.
  LogStream & operator << (const CodeLocation & codeLoction);

  /// @brief Overrides operator &lt;&lt; for any type.

  /// Appends @c t to the stream
  /// @param t The value to append
  /// @return Returns a reference to this object.
  template <typename T> LogStream & operator << (const T & t)
  {
    std::map<LogDestination, boost::iostreams::filtering_ostream *>::iterator it;

    for(it = m_destinations.begin() ; it != m_destinations.end() ; it++)
    {
      if(it->first != SYNC_SCREEN && this->isDestinationUsed(it->first) &&
         (PEInterface::instance().rank() == 0 || !m_filterRankZero[it->first]))
      {
        *(it->second) << t;
        m_flushed = false;
      }
      else if(it->first != SYNC_SCREEN && PEInterface::instance().is_init()
        && this->isDestinationUsed(it->first))
        {
        for( Uint i = 0 ; i < (Uint)(PEInterface::instance().size()); ++i )
        {
          //PEInterface::instance().barrier();

          if(i == (Uint)PEInterface::instance().rank())
          {
            *(it->second) << t;
            m_flushed = false;
          }
          else if(it->first == SYNC_SCREEN && PEInterface::instance().is_init())
          {
            for( Uint i = 0 ; i < (Uint)(PEInterface::instance().size()); ++i )
            {
              //PEInterface::instance().barrier();

              if(i == (Uint)PEInterface::instance().rank())
              {
                *(it->second) << t;
                m_flushed = false;
              }
            }
          } // end of "else if (PEInterface::instance().isInit())"
        } // end of "if(this->isDestinationUsed(it->first()))"
      }
    }

    return *this;
  }

  /// @brief Sets new default level

  /// @c level is set as default log level to all destinations.
  /// @param level The new default level.
  /// @see LogLevelFilter
  void setLogLevel(LogLevel level);

  /// @brief Sets new default level to the specified destination.

  /// If @c destination is @c #FILE but @c #isFileOpen() returns @c false,
  /// nothing is done.
  /// @param destination The destination.
  /// @param level The new default level.
  /// @see LogLevelFilter
  void setLogLevel(LogDestination destination, LogLevel level);

  /// @brief Gives the default log level of a specified destination.

  /// @param destination The destination.
  /// @return Returns the default log level of the specified destination.
  /// @see LogLevelFilter
  LogLevel getLogLevel(LogDestination destination) const;

  /// @brief Modifies the use policy of a destination

  /// If @c destination is @c #FILE but @c #isFileOpen() returns @c false,
  /// nothing is done.
  /// @param destination The destination.
  /// @param use If @c true, the destination is set to "use"; otherwise
  /// it is set to "not use".
  void useDestination(LogDestination destination, bool use);

  /// @brief Gives the use policy of a destination.

  /// If @c destination is @c #FILE but @c #isFileOpen() returns @c false,
  /// this method returns @c false.
  /// @return The use policy of the specified destination.
  bool isDestinationUsed(LogDestination destination) const;

  /// @brief Sets a stamp format to a specified destination.

  /// If @c destination is @c #FILE but @c #isFileOpen() returns @c false,
  /// nothing is done.
  /// @param destination The destination
  /// @param stampFormat The stamp format
  /// @see LogStampFilter
  void setStamp(LogDestination destination, const std::string & stampFormat);

  /// @brief Gives the stamp format of a specified destination.

  /// @param destination The destination
  /// @return Returns the stamp format of the specified destination
  /// @see LogStampFilter
  std::string getStamp(LogDestination destination);

  /// @brief Sets a stamp format to all destinations.

  /// @param stampFormat The stamp format
  /// @see LogStampFilter
  void setStamp(const std::string & stampFormat);

  /// @brief Enables or disables the filter on MPI rank zero on a destination

  /// If enabled, only messages coming from process with MPI rank zero are
  /// forwarded to that destination.
  /// @param dest The destination
  /// @param filterRankZero If @c true, the filter is set to "enabled";
  /// otherwise, it is set to "disabled".
  void setFilterRankZero(LogDestination dest, bool filterRankZero);

  /// @brief Enables or disables the filter on MPI rank zero on all destinations

  /// If enabled, only messages coming from process with MPI rank zero are
  /// forwarded to that destination.
  /// @param filterRankZero If @c true, the filter is set to "enabled";
  /// otherwise, it is set to "disabled".
  void setFilterRankZero(bool filterRankZero);

  /// @brief Checks whether the filter for rank 0 is set on the specified
  /// destination.

  /// @param dest The destination to check
  /// @return Returns @c true if the filter is set; otherwise, returns @c false.
  bool getFilterRankZero(LogDestination dest) const;

  /// @brief Sets the file.

  /// Once this method has been called:
  /// @li it cannot be called another time and, if so, will exit immediately.
  /// @li @c #FILE destination becomes available and @c #isFileOpen() returns
  /// @c true.
  /// @param fileDescr The file descriptor.
  void setFile(const boost::iostreams::file_descriptor_sink & fileDescr);

  /// @brief Cheks whether the file is set.

  /// @return Returns @c true if the file has already been set.
  bool isFileOpen() const;

  /// @brief Appends a string forwarder to the forwarder list.

  /// The forwarder is linked to the string buffer and its
  /// @link LogStringForwarder::message @c message() @endlink method is called.
  /// If the pointer is @c NULL or already exists in the forwarder list,
  /// nothing is done.
  /// @param forwarder The forwarder to append.
  /// @see removeStringForwarder
  void addStringForwarder(LogStringForwarder * forwarder);

  /// @brief Removes a string from the forwarder list.

  /// If the pointer is @c NULL or does not exist in the forwarder list,
  /// nothing is done.
  /// @warning A string forwarder should be @b always removed from all streams
  /// it has been appended to before it is destroyed. Not doing so might result
  /// to a <i>segmentation fault</i>.
  /// @param forwarder The forwarder to remove.
  /// @see addStringForwarder
  void removeStringForwarder(LogStringForwarder * forwarder);

  /// @brief Gives the number of string forwarders the stream contains

  /// @return Returns the number of string forwarders.
  unsigned int getStringForwarderCount() const;

  private:

  /// @brief Destinations.

  /// The key is the destination. The value is the corresponding stream.
  std::map<LogDestination, boost::iostreams::filtering_ostream *> m_destinations;

  /// @brief The forwarder list
  std::list<LogStringForwarder *> m_stringForwarders;

  /// @brief Use policies

  /// The key is the destination. The value is the corresponding use policy.
  std::map<LogDestination, bool> m_usedDests;

  /// @brief Rank zero filter

  /// The key is the destination. The value is the corresponding filter policy.
  std::map<LogDestination, bool> m_filterRankZero;

  /// @brief Buffer for @c #STRING destination
  std::string m_buffer;

  /// @brief Stream name

  /// This attribute is used on @c #FILE stream creation.
  std::string m_streamName;

  /// @brief Default value.

  /// This attribute is used on @c #FILE stream creation. Its value is modified
  /// by @c #setLogLevel(LogLevel).
  LogLevel m_level;

  /// @brief Flush status

  /// If @c true, the streams are flushed. This attribute is used in object
  /// destrutor. If at this moment, it is @c false, the streams are flushed
  /// before their destruction.
  bool m_flushed;

  /// @brief Gives the level filter of a destination

  /// @param dest The destination
  /// @return Returns the level filter
  LogLevelFilter & getLevelFilter(LogDestination dest) const;

  /// @brief Gives the stamp filter of a destination

  /// @param dest The destination
  /// @return Returns the stamp filter
  LogStampFilter & getStampFilter(LogDestination dest) const;


}; // class LogStream

////////////////////////////////////////////////////////////////////////////////

} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_LogStream_hpp
