#include <string>
#include <iostream>

#include <boost/property_tree/detail/rapidxml.hpp>
#include <boost/algorithm/string.hpp>

/// prints the xml node to screen
void print_xml_node(rapidxml::xml_node<> *node)
{
  using namespace rapidxml;

  std::cout << "Node \'" << node->name() << "\'";
  std::cout << " w value [" << node->value() << "]\n";

  for (xml_attribute<> *attr = node->first_attribute(); attr ; attr = attr->next_attribute())
  {
    std::cout << "  + attribute \'" << attr->name() << "\'' ";
    std::cout << "with value [" << attr->value() << "]\n";
  }

  for (xml_node<> * itr = node->first_node(); itr ; itr = itr->next_sibling() )
  {
    print_xml_node(itr);
  }
}

/// copy the names and values to the memory pool of the out node
/// @pre assume that the nodes have been cloned before
void deep_copy_names_values ( rapidxml::xml_node<>& in, rapidxml::xml_node<>& out )
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
    deep_copy_names_values( *inode, *onode );
  }
}

/// deep copies a node into another with all the memory allocated in the second
void deep_copy ( rapidxml::xml_node<>& in, rapidxml::xml_node<>& out )
{
  rapidxml::memory_pool<>& mem = *out.document();
  mem.clone_node(&in,&out);
  deep_copy_names_values(in,out);
}

int main(int argc, char * argv[])
{
  using namespace rapidxml;

  std::string text = ( "<debug lolo=\"1\" koko=\"2\" >"
                       " blabla    lalala  "
                       "<filename>debug.log</filename>"
                       "<modules NBS=\"3\">"
                       "    <module>Finance</module>"
                       "    <module>Admin</module>"
                       "    <module>HR</module>"
                       "</modules>"
                       "<level>2</level>"
                       " propro " // will be ignored
                       "</debug>" );

  // creation of first xml doc
  xml_document<> doc;    // character type defaults to char

  char* ctext = doc.allocate_string(text.c_str());

  doc.parse< rapidxml::parse_no_data_nodes |
             rapidxml::parse_trim_whitespace |
             parse_normalize_whitespace > (ctext);

  // creation of second xml doc, and copy first to it
  xml_document<> adoc;

  deep_copy( doc, adoc );

  // print document
  print_xml_node(adoc.first_node());

  return 0;
}

