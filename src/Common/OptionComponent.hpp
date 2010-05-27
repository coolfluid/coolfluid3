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

    /// @name VIRTUAL FUNCTIONS
    //@{

    /// updates the option value using the xml configuration
    /// @param node XML node with data for this option
    virtual void change_value ( XmlNode& node );

    /// @returns the xml tag for this option
    virtual const char * tag() const { return "component"; }

    /// @returns the value as a sd::string
    virtual std::string value_str () const { return value<std::string>(); }

    /// @returns the default value as a sd::string
    virtual std::string def_str () const  { return def<std::string>(); }

    //@} END VIRTUAL FUNCTIONS

  protected:

    /// storage of the component pointer
    Component::Ptr m_component;

  }; // OptionComponent

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_OptionComponent_hpp
