#include <fstream>

#include <rapidxml/rapidxml_print.hpp>

#include "Common/XmlHelpers.hpp"
#include "Common/Log.hpp"

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

  XmlParams::XmlParams( XmlNode& node ) :
      xmlnode(node),
      xmldoc(*node.document()),
      params( node.first_node( tag_params() ) ) // might be NULL
  {
  }

////////////////////////////////////////////////////////////////////////////////

  const char * XmlParams::tag_doc()    { return "cfxml"; }

  const char * XmlParams::tag_params() { return "params"; }

  const char * XmlParams::tag_key()        { return "key"; }

////////////////////////////////////////////////////////////////////////////////

  void XmlOps::write_xml_node ( const XmlNode& node, const boost::filesystem::path& fpath )
  {
    std::ofstream fout ( fpath.string().c_str() );

    std::string xml_as_string;
    //    rapidxml::print(std::back_inserter(xml_as_string), node, rapidxml::print_no_indenting);
    rapidxml::print(std::back_inserter(xml_as_string), node);

    fout << xml_as_string << std::endl;
  }

////////////////////////////////////////////////////////////////////////////////

  void XmlOps::print_xml_node( const XmlNode& node, Uint nesting )
  {
    std::string nest_str (nesting, '+');
    CFinfo << nest_str
        << " Node \'" << node.name() << "\' [" << node.value() << "]\n";

    for (XmlAttr *attr = node.first_attribute(); attr ; attr = attr->next_attribute())
    {
      CFinfo << nest_str
          << " - attribute \'" << attr->name() << "\' [" << attr->value() << "]\n";
    }

    for (XmlNode * itr = node.first_node(); itr ; itr = itr->next_sibling() )
    {
      print_xml_node( *itr, nesting+1 );
    }
  }

////////////////////////////////////////////////////////////////////////////////

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

  XmlNode* XmlOps::add_node_to ( XmlNode& node, const std::string& nname,  const std::string& nvalue )
  {
    XmlDoc& doc = *node.document();
    XmlNode* nnode = doc.allocate_node( rapidxml::node_element, doc.allocate_string( nname.c_str()), doc.allocate_string(nvalue.c_str()) );
    node.append_node(nnode);
    return nnode;
  }

  XmlAttr* XmlOps::add_attribute_to ( XmlNode& node, const std::string& atname,  const std::string& atvalue )
  {
    XmlDoc& doc = *node.document();
    XmlAttr* nattr = doc.allocate_attribute( doc.allocate_string(atname.c_str()), doc.allocate_string(atvalue.c_str()) );
    node.append_attribute(nattr);
    return nattr;
  }

////////////////////////////////////////////////////////////////////////////////

  boost::shared_ptr<XmlDoc> XmlOps::create_doc ()
  {
    using namespace rapidxml;

    boost::shared_ptr<XmlDoc> xmldoc ( new XmlDoc() );
    XmlDoc& doc = *xmldoc.get();

    // add declaration node
    XmlNode& dnode = *doc.allocate_node ( node_declaration );
    doc.append_node(&dnode);
    XmlOps::add_attribute_to(dnode, "version",    "1.0");
    XmlOps::add_attribute_to(dnode, "encoding",   "UTF-8");
//  XmlOps::add_attribute(*dnode, "standalone",   "yes");

    // add root node
    XmlNode& root = *XmlOps::add_node_to ( doc, "cfxml" );
    XmlOps::add_attribute_to ( root, "version", "1.0");
    XmlOps::add_attribute_to ( root, "type", "message");

    return xmldoc;
  }

////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////

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


  XmlNode* XmlOps::goto_doc_node ( XmlNode& node )
  {
    using namespace rapidxml;

    XmlNode* fnode = &node;

    if ( fnode->type() == node_document )
    {
      fnode = fnode->first_node();
      if ( !fnode )
        throw  Common::XmlError( FromHere(), "XML document is empty" );
    }

    if ( fnode->type() == node_declaration )
    {
      fnode = fnode->next_sibling();
      if ( !fnode )
        throw  Common::XmlError( FromHere(), "No xml nodes after declaration" );
    }

    // find the first doc node
    if ( strcmp(fnode->name() , XmlParams::tag_doc()) ) /* are not equal */
      fnode = fnode->next_sibling( XmlParams::tag_doc() );

    if ( !fnode )
      throw  Common::XmlError( FromHere(), "No xml doc found" );

    return fnode;
  }

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
