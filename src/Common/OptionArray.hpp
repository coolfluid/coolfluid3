// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_OptionArray_hpp
#define CF_Common_OptionArray_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/foreach.hpp>

#include "Common/Option.hpp"
#include "Common/BasicExceptions.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// Class used to get element type of an OptionArray when down casting
  /// from Option.
  /// @author Quentin Gasper
  class Common_API OptionArray : public Option
  {
  public:
    OptionArray(const std::string& name, const std::string& desc, boost::any def);

    /// Returns a C-string representation of element type
    virtual const char * elem_type() const = 0;
  };

  /// Class defines an array of options of the same type to be used by the ConfigObject class
  /// This class supports the following types:
  ///   - bool
  ///   - int
  ///   - CF:Uint
  ///   - CF::Real
  ///   - std::string
  /// @author Tiago Quintino
  template < typename TYPE >
      class OptionArrayT : public OptionArray  {

  public:

    typedef std::vector<TYPE> value_type;

    typedef TYPE element_type;

    OptionArrayT( const std::string& name, const std::string& desc, const value_type& def);

    /// @name VIRTUAL FUNCTIONS
    //@{

    /// Returns a C-strng representation of element type
    virtual const char * elem_type() const { return type_to_str<element_type>(); }

    /// @returns the xml tag for this option
    virtual const char * tag() const { return "array"; }

    /// @returns the value as a sd::string
    virtual std::string value_str () const;

    /// @returns the default value as a sd::string
    virtual std::string def_str () const;

    std::vector<TYPE> value_vect() const;

    //@} END VIRTUAL FUNCTIONS

  protected:

    /// updates the option value using the xml configuration
    /// @param node XML node with data for this option
    virtual void configure ( XmlNode& node );

  private: // helper functions

    void copy_to_linked_params ( const value_type& val );

    std::string dump_to_str ( const boost::any& c ) const;

  }; // class OptionArray

////////////////////////////////////////////////////////////////////////////////

  template < typename TYPE>
  OptionArrayT<TYPE>::OptionArrayT ( const std::string& name, const std::string& desc, const value_type& def) :
  OptionArray(name, desc, def)
  {
//    CFinfo
//        << " creating OptionArray of " << elem_type() <<  "\'s [" << m_name << "]"
//        << " of type [" << m_type << "]"
//        << " w default [" << def_str() << "]"
//        << " w desc [" << m_description << "]\n"
//        << CFendl;
  }

  template < typename TYPE >
   void OptionArrayT<TYPE>::configure ( XmlNode& node )
  {
    XmlAttr *attr = node.first_attribute( "type" );

    if ( !attr )
      throw ParsingFailed (FromHere(), "OptionArray does not have \'type\' attribute" );

    if ( strcmp(attr->value(),elem_type()) )
      throw ParsingFailed (FromHere(), "OptionArray expected \'type\' attribute \'"
                                       +  std::string(attr->value())
                                       + "\' but got \'"
                                       +  std::string(elem_type()) + "\'"  );

    value_type val; // empty vector

    for (XmlNode * itr = node.first_node(); itr ; itr = itr->next_sibling() )
    {
      val.push_back(to_value<TYPE>(*itr));
    }

    XmlAttr *size_attr = node.first_attribute( "size" );
    if ( !size_attr )
      throw ParsingFailed (FromHere(), "OptionArray does not have \'size\' attribute" );

    Uint expected_size = 0;
    to_value(*size_attr,expected_size);
    if ( expected_size != val.size() )
      throw ParsingFailed (FromHere(), "OptionArray \'size\' did not match number of entries" );

    m_value = val;
    copy_to_linked_params(val);
  }

  template < typename TYPE >
   void OptionArrayT<TYPE>::copy_to_linked_params ( const value_type& val )
  {
    BOOST_FOREACH ( void* v, this->m_linked_params )
    {
      value_type* cv = static_cast<value_type*>(v);
      *cv = val;
    }
  }

  template < typename TYPE >
   std::string OptionArrayT<TYPE>::value_str () const
  {
    return dump_to_str(m_value);
  }

  template < typename TYPE >
      std::string OptionArrayT<TYPE>::def_str () const
  {
    return "";//dump_to_str(m_default);
  }

    template < typename TYPE >
     std::string OptionArrayT<TYPE>::dump_to_str ( const boost::any& c ) const
  {
    value_type values = boost::any_cast<value_type>(c);
    std::string result;

    BOOST_FOREACH ( TYPE v, values )
    {
      result += from_value ( v );
      result += ":";
    }


    if ( !result.empty() ) // remove last ":"
      result.erase(result.size()-1);

    return result;
  }

  template<typename TYPE>
    std::vector<TYPE> OptionArrayT<TYPE>::value_vect() const
  {
    return boost::any_cast<value_type>(m_value);
  }


////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_OptionArray_hpp
