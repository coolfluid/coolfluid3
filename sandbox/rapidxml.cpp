#include <string>
#include <iostream>

#include <boost/property_tree/detail/rapidxml.hpp>

void print_xml_node(rapidxml::xml_node<> *node)
{
  using namespace rapidxml;

  std::cout << "Node [" << node->name() << "]";
  std::cout << " w value [" << node->value() << "]\n";

  for (xml_attribute<> *attr = node->first_attribute(); attr; attr = attr->next_attribute())
  {
    std::cout << "++ attribute " << attr->name() << " ";
    std::cout << "with value " << attr->value() << "\n";
  }

  for (xml_node<> * itr = node->first_node(); itr; itr = itr->next_sibling() )
  {
    print_xml_node(itr);
  }
}

int main(int argc, char * argv[])
{
  using namespace rapidxml;

  std::string text = ( "<debug lolo=\"1\" koko=\"2\" >"
                       "<filename>debug.log</filename>"
                       "<modules NBS=\"3\">"
                       "    <module>Finance</module>"
                       "    <module>Admin</module>"
                       "    <module>HR</module>"
                       "</modules>"
                       "<level>2</level>"
                       "</debug>" );

  xml_document<> doc;    // character type defaults to char

  char* ctext = doc.allocate_string(text.c_str());

  doc.parse< rapidxml::parse_no_data_nodes >(ctext);    // 0 means default parse flags

  print_xml_node(doc.first_node());

  return 0;
}

