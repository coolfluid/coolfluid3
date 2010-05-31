#ifndef CF_GUI_Network_LogMessage_hpp
#define CF_GUI_Network_LogMessage_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/EnumT.hpp"
#include "GUI/Network/NetworkAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Network {

////////////////////////////////////////////////////////////////////////////////

class Network_API LogMessage
{
  public:

  /// Enumeration of the worker statuses recognized in CF
  enum Type  { INVALID    =-1,
               INFO       = 0,
               ERROR      = 1,
               EXCEPTION  = 2,
               WARNING    = 3};

  typedef Common::EnumT< LogMessage > ConverterBase;

  struct Network_API Convert : public ConverterBase
  {
    /// storage of the enum forward map
    static ConverterBase::FwdMap_t all_fwd;
    /// storage of the enum reverse map
    static ConverterBase::BwdMap_t all_rev;
  };

}; // class LogMessage

////////////////////////////////////////////////////////////////////////////////

Network_API std::ostream& operator<< ( std::ostream& os, const LogMessage::Type& in );
Network_API std::istream& operator>> ( std::istream& is, LogMessage::Type& in );

////////////////////////////////////////////////////////////////////////////////

} // namespace Network
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Network_LogMessage_hpp
