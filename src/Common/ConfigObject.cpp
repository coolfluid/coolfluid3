#include "Common/BasicExceptions.hpp"
#include "Common/ConfigObject.hpp"
#include "Common/XmlHelpers.hpp"

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

  Option::Ptr OptionList::getOption( const std::string& optname)
  {
    OptionStorage_t::iterator itr = m_options.find(optname);
    if ( itr != m_options.end() )
      return itr->second;
    else
      throw ValueNotFound(FromHere(), "Option with name [" + optname + "] not found" );
  }

  void ConfigObject::configure ( XmlNode& node )
  {
    XmlParams pn ( node );


    if ( pn.params == 0 )
      throw  Common::XmlError( FromHere(), "ConfigObject received  XML without a \'" + std::string(XmlParams::tag_node_valuemap()) + "\' node" );

    // get the list of options
    OptionList::OptionStorage_t& options = m_option_list.m_options;

    // loop on the param nodes
    for (XmlNode* itr =  pn.params->first_node(); itr; itr = itr->next_sibling() )
    {
      // search for the attribute with key
      XmlAttr* att = itr->first_attribute( XmlParams::tag_attr_key() );
      if ( att )
      {
        OptionList::OptionStorage_t::iterator opt = options.find( att->value() );
        if (opt != options.end())
          opt->second->configure_option(*itr);
      }
    }
  }

  Option::Ptr ConfigObject::option( const std::string& optname )
  {
    return m_option_list.getOption(optname);
  }

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
