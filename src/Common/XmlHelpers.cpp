#include <fstream>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <rapidxml/rapidxml_print.hpp>

#include "Common/CPath.hpp"
#include "Common/Log.hpp"

#include "Common/XmlHelpers.hpp"

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

  XmlParams::XmlParams( XmlNode& node ) :
      xmlnode(node),
      xmldoc(*node.document()),
      params( node.first_node( tag_node_valuemap() ) ) // might be NULL
  {
  }

  XmlNode& XmlParams::get_params_node() const
  {
    if ( params == 0 )
      throw  Common::XmlError( FromHere(), "XML node \'" + std::string(tag_node_valuemap()) + "\' not found" );
    return *params;
  }

  void XmlParams::set_clientid(const std::string & uuid)
  {
    XmlAttr * attr = xmlnode.first_attribute(tag_attr_clientid());

    if(attr == CFNULL)
      XmlOps::add_attribute_to(xmlnode, tag_attr_clientid(), uuid);
    else
      attr->value(uuid.c_str());
  }

  std::string XmlParams::get_clientid() const
  {
    std::string uuid;
    XmlAttr * attr = xmlnode.first_attribute(tag_attr_clientid());

    if(attr != CFNULL)
      uuid = attr->value();

    return uuid;
  }

  std::string XmlParams::get_frameid() const
  {
    std::string uuid;
    XmlAttr * attr = xmlnode.first_attribute(tag_attr_frameid());

    if(attr != CFNULL)
      uuid = attr->value();

    return uuid;
  }


////////////////////////////////////////////////////////////////////////////////

  const char * XmlParams::tag_node_doc()    { return "cfxml"; }

  const char * XmlParams::tag_node_signal() { return "signal"; }

  const char * XmlParams::tag_node_reply()  { return "reply"; }

  const char * XmlParams::tag_node_valuemap() { return "valuemap"; }

  const char * XmlParams::tag_node_frame()  { return "frame"; }

  const char * XmlParams::tag_attr_key()    { return "key"; }

  const char * XmlParams::tag_attr_type()    { return "type"; }

  const char * XmlParams::tag_attr_size()    { return "size"; }

  const char * XmlParams::tag_attr_descr()    { return "descr"; }

  const char * XmlParams::tag_attr_clientid()    { return "clientid"; }

  const char * XmlParams::tag_attr_frameid()    { return "frameid"; }

////////////////////////////////////////////////////////////////////////////////

  void XmlOps::write_xml_node ( const XmlNode& node, const boost::filesystem::path& fpath )
  {
    std::ofstream fout ( fpath.string().c_str() );

    std::string xml_as_string;

    XmlOps::xml_to_string ( node, xml_as_string );

    fout << xml_as_string << std::endl;
  }

////////////////////////////////////////////////////////////////////////////////

  void XmlOps::xml_to_string ( const XmlNode& node, std::string& str )
  {
    //    rapidxml::print(std::back_inserter(xml_as_string), node, rapidxml::print_no_indenting);
    rapidxml::print(std::back_inserter(str), node);
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

    size_t rs = fread(buffer,lenght, 1, filep);           // read into buffer
    if (!rs) { throw FileSystemError(FromHere(), "Error while reading file [" + filepath + "]" ); }

    fclose(filep);                             // close file

    // parser trims and merges whitespaces
    xmldoc->parse< parse_no_data_nodes |
                   parse_trim_whitespace |
                   parse_normalize_whitespace >(buffer);

    return xmldoc;
  }

  XmlNode* XmlOps::first_frame_node ( XmlNode& node )
  {
    using namespace rapidxml;

    for ( XmlNode* itr = node.first_node(); itr; itr = itr->next_sibling() )
    {
      const char * name = itr->name();
      if ( !strcmp (name, XmlParams::tag_node_signal()) || !strcmp (name,XmlParams::tag_node_reply()) )
        return itr;
    }

    throw ShouldNotBeHere(FromHere(),"Control reaches end of non-void function");
    return node.first_node();
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
    if ( strcmp(fnode->name() , XmlParams::tag_node_doc()) ) /* are not equal */
      fnode = fnode->next_sibling( XmlParams::tag_node_doc() );

    if ( !fnode )
      throw  Common::XmlError( FromHere(), "No xml doc found" );

    return fnode;
  }


  XmlNode* XmlOps::add_signal_frame( XmlNode& xmlnode,
                                     const std::string & target,
                                     const CPath & sender,
                                     const CPath & receiver,
                                     bool userTrans)
  {
    std::stringstream ss;

    XmlNode* signalnode = XmlOps::add_node_to( xmlnode, XmlParams::tag_node_frame() );

    ss << boost::uuids::random_generator()();

    XmlOps::add_attribute_to( *signalnode, "type", XmlParams::tag_node_signal() );
    XmlOps::add_attribute_to( *signalnode, "target", target );
    XmlOps::add_attribute_to( *signalnode, "sender", sender.string() );
    XmlOps::add_attribute_to( *signalnode, "receiver", receiver.string() );
    XmlOps::add_attribute_to( *signalnode, "transaction", userTrans ? "user" : "auto" );
    XmlOps::add_attribute_to( *signalnode, "frameid", ss.str() );

    return signalnode;
  }

  XmlNode* XmlOps::add_reply_frame( XmlNode& xmlnode )
  {
    XmlNode* replynode = XmlOps::add_node_to( *xmlnode.parent(), XmlParams::tag_node_frame() );

    XmlOps::add_attribute_to( *replynode, "type", XmlParams::tag_node_reply() );

    // reply with same target
    XmlAttr* target_att = xmlnode.first_attribute("target");
    std::string target = target_att ? target_att->value() : "";
    XmlOps::add_attribute_to( *replynode, "target", target );

    // reply with same target
    XmlAttr* sender_att = xmlnode.first_attribute("sender");
    std::string receiver = sender_att ? sender_att->value() : "";
    XmlOps::add_attribute_to( *replynode, "receiver", receiver );

    // same transaction type
    XmlAttr* trans_att = xmlnode.first_attribute("transaction");
    std::string trans = trans_att ? trans_att->value() : "auto";
    XmlOps::add_attribute_to( *replynode, "transaction", trans );

    // copy uuids, if any
    XmlAttr* uuid_att = xmlnode.first_attribute(XmlParams::tag_attr_clientid());

    if(uuid_att != CFNULL)
      XmlOps::add_attribute_to( *replynode, XmlParams::tag_attr_clientid(), uuid_att->value() );

    uuid_att = xmlnode.first_attribute(XmlParams::tag_attr_frameid());

    if(uuid_att != CFNULL)
      XmlOps::add_attribute_to( *replynode, XmlParams::tag_attr_frameid(), uuid_att->value() );

    return replynode;
  }

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
