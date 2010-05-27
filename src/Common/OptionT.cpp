#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include "Common/OptionT.hpp"
#include "Common/StringOps.hpp"
#include "Common/BasicExceptions.hpp"

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

//  /// specialization to handle vector conversions
//  template < typename TYPE >
//  struct ConvertValue< std::vector<TYPE> >
//  {
//    static std::vector<TYPE> convert ( char * str )
//    {
////      CFinfo << "converting string to vector\n" << CFendl;
//
//      std::vector < TYPE > vvalues; // start with clean vector
//
//      std::string ss (str);
//      boost::tokenizer<> tok (ss);
//      for(boost::tokenizer<>::iterator elem = tok.begin(); elem != tok.end(); ++elem)
//      {
//        vvalues.push_back( StringOps::from_str< TYPE >(*elem) );
//      }
//
//      return vvalues; // assign to m_value (replaces old values)
//    }
//  };

////////////////////////////////////////////////////////////////////////////////

template < typename TYPE >
void OptionT<TYPE>::copy_to_linked_params ( const TYPE& val )
{
  BOOST_FOREACH ( void* v, this->m_linked_params )
  {
    TYPE* cv = static_cast<TYPE*>(v);
    *cv = val;
  }
}

////////////////////////////////////////////////////////////////////////////////

template <>
    void OptionT<bool>::xmlvalue_convert ( bool& val, XmlNode& node )
{
  bool error = true;
  std::string vt ( node.value() );

  boost::algorithm::is_equal test_equal;

  if ( test_equal(vt,"true") ||
       test_equal(vt,"on")   ||
       test_equal(vt,"1")     )
  {
    val   = true;
    error = false;
  }

  if ( !error && (
       test_equal(vt,"false") ||
       test_equal(vt,"off")   ||
       test_equal(vt,"0")     ))
  {
      val   = false;
      error = false;
  }

  if (error)
    throw ParsingFailed (FromHere(), "Incorrect option conversion to bool of string [" + vt + "]" );
}

template <>
    void OptionT<int>::xmlvalue_convert ( int& val, XmlNode& node )
{
   val = boost::lexical_cast<int> ( node.value() );
}

template <>
    void OptionT<CF::Uint>::xmlvalue_convert ( CF::Uint& val, XmlNode& node )
{
  val = boost::lexical_cast<CF::Uint> ( node.value() );
}

template <>
    void OptionT<CF::Real>::xmlvalue_convert ( CF::Real& val, XmlNode& node )
{
  val = boost::lexical_cast<CF::Real> ( node.value() );
}

template <>
    void OptionT<std::string>::xmlvalue_convert ( std::string& val, XmlNode& node )
{
   val = node.value();
}


////////////////////////////////////////////////////////////////////////////////


  template <>
      void OptionT<bool>::change_value ( XmlNode& node )
  {
    bool val;
    xmlvalue_convert(val,node);
    m_value = val;
    copy_to_linked_params(val);
  }

  template <>
  void OptionT<int>::change_value ( XmlNode& node )
  {
    int val = 0;
    xmlvalue_convert(val,node);
    m_value = val;
    copy_to_linked_params(val);
  }

  template <>
      void OptionT<CF::Uint>::change_value ( XmlNode& node )
  {
    CF::Uint val = 0;
    xmlvalue_convert(val,node);
    m_value = val;
    copy_to_linked_params(val);
  }

  template <>
      void OptionT<CF::Real>::change_value ( XmlNode& node )
  {
    CF::Real val = 0;
    xmlvalue_convert(val,node);
    m_value = val;
    copy_to_linked_params(val);
  }

  template <>
      void OptionT<std::string>::change_value ( XmlNode& node )
  {
    std::string val;
    xmlvalue_convert(val,node);
    m_value = val;
    copy_to_linked_params(val);
  }

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
