#include "Common/BasicExceptions.hpp"
#include "Common/ConfigObject.hpp"

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

  void ConfigObject::configure ( rapidxml::xml_node<> *node )
  {
    using namespace rapidxml;
    OptionList::OptionStorage_t& options = m_option_list.m_options;

    // loop on the child nodes which should be option configurations
    for (xml_node<>* itr = node->first_node(); itr; itr = itr->next_sibling() )
    {
      OptionList::OptionStorage_t::iterator opt = options.find( itr->name() );
      if (opt != options.end())
        opt->second->configure_option(itr);
    }
  }

  Option::Ptr ConfigObject::option( const std::string& optname )
  {
    return m_option_list.getOption(optname);
  }

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
