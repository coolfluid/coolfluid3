#ifndef CF_Common_OptionT_hpp
#define CF_Common_OptionT_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/foreach.hpp>

#include "Common/Option.hpp"

#include "Common/BasicExceptions.hpp"

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

    /// @returns the xml tag for this option
    virtual const char * tag() const;

    /// @returns the value as a sd::string
    virtual std::string value_str () const { return from_value ( value<TYPE>() ); }

    /// @returns the default value as a sd::string
    virtual std::string def_str () const  { return from_value ( def<TYPE>() ); }

    //@} END VIRTUAL FUNCTIONS
        
  protected: // functions 
        
    /// updates the option value using the xml configuration
    /// @param node XML node with data for this option
    virtual void configure ( XmlNode& node );

  private: // helper functions

    /// copy the configured update value to all linked parameters
    void copy_to_linked_params ( const TYPE& val );

  }; // OptionT

////////////////////////////////////////////////////////////////////////////////

  template < typename TYPE>
  OptionT<TYPE>::OptionT ( const std::string& name, const std::string& desc, value_type def ) :
      Option(name, type_to_str<TYPE>(), desc, def)
  {
//    CFinfo
//        << " creating OptionT [" << m_name << "]"
//        << " of type [" << m_type << "]"
//        << " w default [" << def_str() << "]"
//        << " w desc [" << m_description << "]\n"
//        << CFendl;
  }

  template < typename TYPE>
  void OptionT<TYPE>::configure ( XmlNode& node )
  {
    TYPE val;
    const char * type_str = XmlTag<TYPE>::type();
    XmlNode * type_node = node.first_node(type_str);

    if(type_node != CFNULL)
      to_value(*type_node,val);
    else
    {
      std::string str;
      throw XmlError(FromHere(), std::string("Could not find a value of this type [")
                     + type_str + "].");
    }
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
