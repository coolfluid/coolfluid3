#include "Common/Log.hpp"
#include "Common/XML.hpp"

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

  void XmlOps::print_xml_node( const rapidxml::xml_node<>& node )
  {
    using namespace rapidxml;

    CFinfo << "Node \'" << node.name() << "\'";
    CFinfo << " w value [" << node.value() << "]\n";

    for (xml_attribute<> *attr = node.first_attribute(); attr ; attr = attr->next_attribute())
    {
      CFinfo << "  + attribute \'" << attr->name() << "\'' ";
      CFinfo << "with value [" << attr->value() << "]\n";
    }

    for (xml_node<> * itr = node.first_node(); itr ; itr = itr->next_sibling() )
    {
      print_xml_node(*itr);
    }
  }

  void XmlOps::deep_copy ( const rapidxml::xml_node<>& in, rapidxml::xml_node<>& out )
  {
    rapidxml::memory_pool<>& mem = *out.document();
    mem.clone_node(&in,&out);
    XmlOps::deep_copy_names_values(in,out);
  }

  void XmlOps::deep_copy_names_values ( const rapidxml::xml_node<>& in, rapidxml::xml_node<>& out )
  {
    rapidxml::memory_pool<>& mem = *out.document();

    char* nname  = mem.allocate_string(in.name());
    char* nvalue = mem.allocate_string(in.value());

    // TESTING
    //  boost::to_upper(nname);
    //  boost::to_upper(nvalue);

    out.name(nname);
    out.value(nvalue);

    // copy names and values of the attributes
    rapidxml::xml_attribute<>* iattr = in.first_attribute();
    rapidxml::xml_attribute<>* oattr = out.first_attribute();

    for ( ; iattr ; iattr = iattr->next_attribute(),
                     oattr = oattr->next_attribute() )
    {
      char* aname  = mem.allocate_string(iattr->name());
      char* avalue = mem.allocate_string(iattr->value());

      // TESTING
      //    boost::to_upper(aname);
      //    boost::to_upper(avalue);

      oattr->name(aname);
      oattr->value(avalue);
    }

    // copy names and values of the child nodes
    rapidxml::xml_node<> * inode = in.first_node();
    rapidxml::xml_node<> * onode = out.first_node();

    for ( ; inode ; inode = inode->next_sibling(),
                     onode = onode->next_sibling() )
    {
      XmlOps::deep_copy_names_values( *inode, *onode );
    }
  }

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
