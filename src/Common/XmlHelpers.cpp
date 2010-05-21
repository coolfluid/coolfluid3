#include "Common/XmlHelpers.hpp"
#include "Common/Log.hpp"

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

  void XmlOps::print_xml_node( const XmlNode& node )
  {
    CFinfo << "Node \'" << node.name() << "\'";
    CFinfo << " w value [" << node.value() << "]\n";

    for (XmlAttr *attr = node.first_attribute(); attr ; attr = attr->next_attribute())
    {
      CFinfo << "  + attribute \'" << attr->name() << "\'' ";
      CFinfo << "with value [" << attr->value() << "]\n";
    }

    for (XmlNode * itr = node.first_node(); itr ; itr = itr->next_sibling() )
    {
      print_xml_node(*itr);
    }
  }

  void XmlOps::deep_copy ( const XmlNode& in, XmlNode& out )
  {
    XmlMemPool& mem = *out.document();
    mem.clone_node(&in,&out);
    XmlOps::deep_copy_names_values(in,out);
  }

  void XmlOps::deep_copy_names_values ( const XmlNode& in, XmlNode& out )
  {
    XmlMemPool& mem = *out.document();

    char* nname  = mem.allocate_string(in.name());
    char* nvalue = mem.allocate_string(in.value());

    // TESTING
    //  boost::to_upper(nname);
    //  boost::to_upper(nvalue);

    out.name(nname);
    out.value(nvalue);

    // copy names and values of the attributes
    XmlAttr* iattr = in.first_attribute();
    XmlAttr* oattr = out.first_attribute();

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
    XmlNode * inode = in.first_node();
    XmlNode * onode = out.first_node();

    for ( ; inode ; inode = inode->next_sibling(),
                     onode = onode->next_sibling() )
    {
      XmlOps::deep_copy_names_values( *inode, *onode );
    }
  }

  boost::shared_ptr<XmlDoc> XmlOps::parse ( const std::string& str )
  {
    using namespace rapidxml;

    boost::shared_ptr<XmlDoc> xmldoc ( new XmlDoc() );

    char* ctext = xmldoc->allocate_string(str.c_str());

    // parser trims and merges whitespaces
    xmldoc->parse< parse_no_data_nodes |
                   parse_trim_whitespace |
                   parse_normalize_whitespace >(ctext);

    return xmldoc;
  }

  boost::shared_ptr<XmlDoc> XmlOps::parse ( const boost::filesystem::path& path )
  {
    using namespace rapidxml;

    boost::shared_ptr<XmlDoc> xmldoc ( new XmlDoc() );

    std::string filepath = path.string();
    FILE *filep = fopen( filepath.c_str(), "rb" );
    if (filep==NULL) { throw FileSystemError(FromHere(), "Unable to open file [" + filepath + "]" ); }

    fseek(filep,0,SEEK_END);                  // go to end
    Uint lenght = ftell(filep);               // get position at end (length)
    if (!lenght) { throw FileSystemError(FromHere(), "File [" + filepath + "] is empty" ); }

    fseek(filep,0,SEEK_SET);                  // go to beginning

    //    char *buffer = (char *) malloc(lenght);         // allocate buffer
    char* buffer = xmldoc->allocate_string( 0, lenght );  // allocate buffer directly inside the xmldoc

    fread(buffer,lenght, 1, filep);           // read into buffer

    fclose(filep);                             // close file

    // parser trims and merges whitespaces
    xmldoc->parse< parse_no_data_nodes |
                   parse_trim_whitespace |
                   parse_normalize_whitespace >(buffer);

    return xmldoc;
  }

/////////////////////////////////////////////////////////////////////////////////////

  XmlParams::XmlParams( XmlNode& node ) : xml(node), params(CFNULL)
  {
    // descend into this node
    XmlNode* in_node = node.first_node();

    // get the first "Params" node
    params = in_node->first_node("Params");

    if ( params == 0 )
      throw  Common::XmlError( FromHere(), "XML Params node not found" );
  }

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
