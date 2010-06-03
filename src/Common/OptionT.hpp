#ifndef CF_Common_OptionT_hpp
#define CF_Common_OptionT_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/foreach.hpp>

#include "Common/Option.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines one option to be used by the ConfigObject class
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

    OptionT ( const std::string& name, const std::string& desc, value_type def );

    /// @name VIRTUAL FUNCTIONS
    //@{

    /// updates the option value using the xml configuration
    /// @param node XML node with data for this option
    virtual void change_value ( XmlNode& node );

    /// @returns the xml tag for this option
    virtual const char * tag() const;

    /// @returns the value as a sd::string
    virtual std::string value_str () const { return value_to_xmlstr ( value<TYPE>() ); }

    /// @returns the default value as a sd::string
    virtual std::string def_str () const  { return value_to_xmlstr ( def<TYPE>() ); }

    //@} END VIRTUAL FUNCTIONS

  private: // helper functions

    /// copy the configured update value to all linked parameters
    void copy_to_linked_params ( const TYPE& val );

  }; // OptionT

////////////////////////////////////////////////////////////////////////////////

  template < typename TYPE>
  OptionT<TYPE>::OptionT ( const std::string& name, const std::string& desc, value_type def ) :
      Option(name,DEMANGLED_TYPEID(value_type), desc, def)
  {
//    CFinfo
//        << " creating OptionT [" << m_name << "]"
//        << " of type [" << m_type << "]"
//        << " w default [" << def_str() << "]"
//        << " w desc [" << m_description << "]\n"
//        << CFendl;
  }

  template < typename TYPE>
  void OptionT<TYPE>::change_value ( XmlNode& node )
  {
    TYPE val;
    xmlstr_to_value(node,val);
    m_value = val;
    copy_to_linked_params(val);
  }

  template < typename TYPE >
  void OptionT<TYPE>::copy_to_linked_params ( const TYPE& val )
  {
    BOOST_FOREACH ( void* v, this->m_linked_params )
    {
      TYPE* cv = static_cast<TYPE*>(v);
      *cv = val;
    }
  }

  template < typename TYPE >
  const char* OptionT<TYPE>::tag () const
  {
    return XmlTag<TYPE>::type();
  }

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_OptionT_hpp
