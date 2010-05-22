#ifndef CF_Common_XmlHelpers_hpp
#define CF_Common_XmlHelpers_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/filesystem/path.hpp>

#include "Common/XML.hpp"
#include "Common/NonInstantiable.hpp"
#include "Common/StringOps.hpp"
#include "Common/BasicExceptions.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// Generic operations on the rapidxml structure
  /// @author Tiago Quintino
  struct Common_API XmlOps : public NonInstantiable<XmlOps>
  {
    /// prints the xml node to screen
    static void print_xml_node( const XmlNode& node );

    /// deep copies a node into another with all the memory allocated in the second
    static void deep_copy ( const XmlNode& in, XmlNode& out );

    /// parses a xml string
    /// @param str string with the xml contents
    static boost::shared_ptr<XmlDoc> parse ( const std::string& str );

    /// parses a xml file
    /// @param fpath path to file with xml contents
    static boost::shared_ptr<XmlDoc> parse ( const boost::filesystem::path& path );

  private:

    /// Helper function that copy the names and values to the memory pool of the out node
    /// @pre assume that the nodes have been cloned before
    static void deep_copy_names_values ( const XmlNode& in, XmlNode& out );

  }; // XmlOps

  /// Helper class that extracts parameters from a XmlNode
  /// @author Tiago Quintino
  /// @todo add_param function
  /// @todo implement params converted to std::vector

  struct Common_API XmlParams
  {
    /// Constructor
    /// @param node the node where the parameters will be extracted from
    /// @throw XmlError when the Params node is not found
    XmlParams ( XmlNode& node );

    /// access to the value of one parameter
    template < typename TYPE >
        TYPE get_value ( const std::string& pname ) const;

    /// storage of the XmlNode to retrieve params from
    XmlNode& xml;

    /// pointer to the params node
    XmlNode* params;

  }; // XmlParams

  template < typename TYPE >
      TYPE XmlParams::get_value ( const std::string& pname ) const
  {
    XmlNode* node = params->first_node( pname.c_str() );
    if ( !node )
      throw  Common::XmlError( FromHere(), "Did not find parameter [" + pname + "]" );

    return StringOps::from_str< TYPE >( node->value() );
  }

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_XmlHelpers_hpp
