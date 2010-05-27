#ifndef CF_Common_OptionArray_hpp
#define CF_Common_OptionArray_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/foreach.hpp>

#include "Common/Option.hpp"
#include "Common/BasicExceptions.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines an array of options of the same type to be used by the ConfigObject class
  /// This class supports the following types:
  ///   - bool
  ///   - int
  ///   - CF:Uint
  ///   - CF::Real
  ///   - std::string
  /// @author Tiago Quintino
  template < typename TYPE >
      class OptionArray : public Option  {

  public:

    typedef std::vector<TYPE> value_type;
    typedef TYPE element_type;

    OptionArray ( const std::string& name, const std::string& desc, const value_type& def );

    /// @name VIRTUAL FUNCTIONS
    //@{

    /// updates the option value using the xml configuration
    /// @param node XML node with data for this option
    virtual void change_value ( XmlNode& node );

    /// @returns the xml tag for this option
    virtual const char * tag() const { return "array"; }

    /// @returns the value as a sd::string
    virtual std::string value_str () const;

    /// @returns the default value as a sd::string
    virtual std::string def_str () const;

    //@} END VIRTUAL FUNCTIONS

  private: // helper functions

    void copy_to_linked_params ( const value_type& val );

    const char * type_tag() const;

    std::string dump_to_str ( const boost::any& c ) const;

  }; // OptionArray

////////////////////////////////////////////////////////////////////////////////

  template<>
  const char * OptionArray<bool>::type_tag() const { return "bool"; }

  template<>
  const char * OptionArray<int>::type_tag() const { return "integer"; };

  template<>
  const char * OptionArray<CF::Uint>::type_tag() const { return "integer"; }

  template<>
  const char * OptionArray<CF::Real>::type_tag() const { return "real"; }

  template<>
  const char * OptionArray<std::string>::type_tag() const { return "string"; }

  template < typename TYPE>
  OptionArray<TYPE>::OptionArray ( const std::string& name, const std::string& desc, const value_type& def ) :
      Option(name,DEMANGLED_TYPEID(value_type), desc, def)
  {
    CFinfo
        << " creating OptionArray of " << type_tag() <<  "\'s [" << m_name << "]"
        << " of type [" << m_type << "]"
        << " w default [" << def_str() << "]"
        << " w desc [" << m_description << "]\n"
        << CFendl;
  }

  template < typename TYPE >
      void OptionArray<TYPE>::change_value ( XmlNode& node )
  {
    XmlAttr *attr = node.first_attribute( "type" );

    if ( !attr || attr->value() != type_tag() )
      throw ParsingFailed (FromHere(), "OptionArray has bad tag \'type\'" );

    std::vector<TYPE> val; // empty vector

    for (XmlNode * itr = node.first_node(); itr ; itr = itr->next_sibling() )
    {
      TYPE vi;
      xmlstr_to_value(*itr,vi);
      val.push_back(vi);
    }

    m_value = val;
    copy_to_linked_params(val);
  }

  template < typename TYPE >
      void OptionArray<TYPE>::copy_to_linked_params ( const value_type& val )
  {
    BOOST_FOREACH ( void* v, this->m_linked_params )
    {
      value_type* cv = static_cast<value_type*>(v);
      *cv = val;
    }
  }

  template < typename TYPE >
      std::string OptionArray<TYPE>::value_str () const
  {
    return dump_to_str(m_value);
  }

  template < typename TYPE >
      std::string OptionArray<TYPE>::def_str () const
  {
    return dump_to_str(m_default);
  }

    template < typename TYPE >
        std::string OptionArray<TYPE>::dump_to_str ( const boost::any& c ) const
  {
    value_type values = boost::any_cast<value_type>(c);
    std::string result;
    BOOST_FOREACH ( TYPE& v, values )
    {
      result += value_to_xmlstr ( v );
      result += ":";
    }
    if ( !result.empty() ) // remove last ":"
      result.erase(result.size()-1);

    return result;
  }


////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_OptionArray_hpp
