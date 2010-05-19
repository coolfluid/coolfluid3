#include <string>
#include <iostream>

#include "xmlParser.h"

void print_xmlnode( XMLNode node )
{
  std::cout << "Node [" << node.getName() << "]\n";

  for ( int i = 0; i < node.nText(); ++i)
  {
    std::cout << " w text [" << node.getText() << "]\n";
  }

  for ( int i = 0; i < node.nAttribute(); ++i)
  {
    XMLAttribute attr = node.getAttribute(i);
    std::cout << "  + attribute [" << attr.lpszName << "] ";
    std::cout << "with value [" << attr.lpszValue << "]\n";
  }

  for ( int i = 0; i < node.nChildNode(); ++i)
  {
    print_xmlnode(node.getChildNode(i));
  }
}

int main(int argc, char * argv[])
{
  std::string text = ( "<debug lolo=\"1\" koko=\"2\" >"
                       "<filename>debug.log</filename>"
                       "<modules NBS=\"3\">"
                       "    <module>Finance</module>"
                       "    <module>Admin</module>"
                       "    <module>HR</module>"
                       "</modules>"
                       "<level>2</level>"
                       "</debug>" );

  XMLNode root_node = XMLNode::parseString(text.c_str());

  print_xmlnode(root_node);
}


