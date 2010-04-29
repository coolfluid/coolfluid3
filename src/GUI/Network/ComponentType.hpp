#ifndef CF_GUI_Network_ComponentType_hpp
#define CF_GUI_Network_ComponentType_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/EnumT.hpp"
#include "GUI/Network/NetworkAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Network {

////////////////////////////////////////////////////////////////////////////////
    
  class Network_API ComponentType
  {
  public:
    
    /// Enumeration of the worker statuses recognized in CF
    enum Type  { INVALID     =-1,
      ROOT  = 0,
      GROUP = 1,
      LINK  = 2};
    
    typedef Common::EnumT< ComponentType > ConverterBase;
    
    struct Network_API Convert : public ConverterBase
    {
      /// storage of the enum forward map
      static ConverterBase::FwdMap_t all_fwd;
      /// storage of the enum reverse map
      static ConverterBase::BwdMap_t all_rev;
    };
    
  }; // class StatusWorker
  
////////////////////////////////////////////////////////////////////////////////
  
  Network_API std::ostream& operator<< ( std::ostream& os, const ComponentType::Type& in );
  Network_API std::istream& operator>> ( std::istream& is, ComponentType::Type& in );
  
////////////////////////////////////////////////////////////////////////////////
  
} // namespace Network
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Network_ComponentType_hpp
