#ifndef CF_Common_OptionT_hpp
#define CF_Common_OptionT_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Option.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines options to be used in the ConfigObject class
  /// This class supports the following types:
  ///   - bool
  ///   - int
  ///   - CF:Uint
  ///   - CF::Real
  ///   - std::string
  /// @author Tiago Quintino
  template < typename TYPE >
      class OptionT : public Option  {

  public:

    typedef TYPE value_type;

    OptionT ( const std::string& name, const std::string& desc, value_type def ) :
        Option(name,DEMANGLED_TYPEID(value_type), desc, def)
    {
//      CFinfo
//          << " creating option ["
//          << m_name << "] of type ["
//          << m_type << "] w default ["
////          << StringOps::to_str( def<TYPE>() ) << "] desc ["
//          << m_description << "]\n" << CFflush;
    }

    /// updates the option value using the xml configuration
    /// @param node XML node with data for this option
    virtual void change_value ( XmlNode& node );

    /// @returns the xml tag for this option
    virtual const char * tag() const;

  private:

    void xmlvalue_convert ( TYPE& val, XmlNode& node );

    void copy_to_linked_params ( const TYPE& val );

  }; // OptionT

  template<>
  const char * OptionT<bool>::tag() const { return "bool"; }

  template<>
  const char * OptionT<int>::tag() const { return "integer"; };

  template<>
  const char * OptionT<CF::Uint>::tag() const { return "integer"; }

  template<>
  const char * OptionT<CF::Real>::tag() const { return "real"; }

  template<>
  const char * OptionT<std::string>::tag() const { return "string"; }

  template<>
  const char * OptionT< std::vector< bool > >::tag() const { return "vector"; }

  template<>
  const char * OptionT< std::vector< int > >::tag() const { return "vector"; }

  template<>
  const char * OptionT< std::vector< CF::Uint > >::tag() const { return "vector"; }

  template<>
  const char * OptionT< std::vector< CF::Real > >::tag() const { return "vector"; }

  template<>
  const char * OptionT< std::vector< std::string > >::tag() const { return "vector"; }

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_OptionT_hpp
