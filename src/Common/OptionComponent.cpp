#include "Common/OptionComponent.hpp"

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

OptionComponent::OptionComponent ( const std::string& name, const std::string& desc, const std::string& def_name ) :
    Option(name, Component::getClassName(), desc, def_name )
{
  CFinfo
      << " creating option ["
      << m_name << "] of type ["
      << m_type << "] w default ["
      << def_name << "] desc ["
      << m_description << "]\n" << CFendl;
}

void OptionComponent::change_value ( rapidxml::xml_node<> *node )
{
  std::string keyname = node->value();
  m_value = keyname;

  m_component.reset(); // delete previous pointee

  //    Common::SafePtr< Component::PROVIDER > prov =
  //        Factory< Component >::getInstance().getProvider( keyname );

  //    m_component = prov->create( keyname );
  // for the moment we repeat the keyname for the actual component name
  // later we will create subnodes in the xml,
  //  * one for the concrete type
  //  * one for the name

}

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
