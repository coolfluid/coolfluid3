#ifndef CF_Common_OptionComponent_hpp
#define CF_Common_OptionComponent_hpp

/////////////////////////////////////////////////////////////////////////////////////

#include "Common/Option.hpp"
#include "Common/Component.hpp"

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

  /// Class defines options to be used in the ConfigObject class
  /// @author Tiago Quintino
  class OptionComponent : public Option
  {
  public:

    typedef std::string value_type;

    /// constructor
    OptionComponent ( const std::string& name, const std::string& desc, const std::string& def_name );

    /// updates the option value using the xml configuration
    virtual void change_value ( rapidxml::xml_node<> *node );

  protected:

    /// storage of the component pointer
    Component::Ptr m_component;

  };

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_OptionComponent_hpp
