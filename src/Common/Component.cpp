// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3.
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

#include "Common/Foreach.hpp"
#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/XmlHelpers.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/OptionArray.hpp"
#include "Common/OptionURI.hpp"
#include "Common/String/Conversion.hpp"

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

Component::Component ( const std::string& name ) :
    m_name (),
    m_path (),
    m_properties(),
    m_components(),
    m_dynamic_components(),
    m_raw_parent( nullptr ),
    m_is_link (false)
{
  // accept name

  if (!URI::is_valid_element( name ))
    throw InvalidURI(FromHere(), "Component name ["+name+"] is invalid");
  m_name = name;

  // signals

  regist_signal ( "create_component" , "creates a component", "Create component" )->connect ( boost::bind ( &Component::create_component_signal, this, _1 ) );

  regist_signal( "list_tree" , "lists the component tree inside this component", "List tree" )->connect ( boost::bind ( &Component::list_tree, this, _1 ) );

  regist_signal( "list_properties" , "lists the options of this component", "List properties" )->connect ( boost::bind ( &Component::list_properties, this, _1 ) );

  regist_signal( "list_signals" , "lists the options of this component", "List signals" )->connect ( boost::bind ( &Component::list_signals, this, _1 ) );

  regist_signal( "configure" , "configures this component", "Configure" )->connect ( boost::bind ( &Component::configure, this, _1 ) );

  regist_signal( "print_info" , "prints info on this component", "Info" )->connect ( boost::bind ( &Component::print_info, this, _1 ) );

  regist_signal( "rename_component" , "Renames this component", "Rename" )->connect ( boost::bind ( &Component::rename_component, this, _1 ) );

  regist_signal( "delete_component" , "Deletes a component", "Delete" )->connect ( boost::bind ( &Component::delete_component, this, _1 ) );

  regist_signal( "move_component" , "Moves a component to another component", "Move" )->connect ( boost::bind ( &Component::move_component, this, _1 ) );

  regist_signal( "save_tree", "Saves the tree", "Save tree")->connect( boost::bind(&Component::save_tree, this, _1) );

  regist_signal( "list_content", "Lists component content", "List content")->connect(boost::bind(&Component::list_content, this, _1));

  regist_signal( "signal_signature", "Gives signature of a signal", "")->connect( boost::bind(&Component::signal_signature, this, _1));

  // these signals should not be seen from the GUI
  signal("list_tree").is_hidden = true;
  signal("list_properties").is_hidden = true;
  signal("list_signals").is_hidden = true;
  signal("configure").is_hidden = true;
  signal("save_tree").is_hidden = true;
  signal("list_content").is_hidden = true;
  signal("signal_signature").is_hidden = true;

  // these signals are read-only
  signal("list_tree").is_read_only = true;

  // signatures
  signal("create_component").signature->connect( boost::bind(&Component::create_component_signature, this, _1) );
  signal("rename_component").signature->connect( boost::bind(&Component::rename_component_signature, this, _1) );
  signal("move_component").signature->connect( boost::bind(&Component::move_component_signature, this, _1) );

  // properties

  m_properties.add_property("brief", std::string("No brief description available"));
  m_properties.add_property("description", std::string("This component has not a long description"));
}

Component::~Component()
{
//  CFinfo << "deleting component \'" << name() << "\'" << CFendl;
}

/////////////////////////////////////////////////////////////////////////////////////

Component::Ptr Component::follow()
{
  return shared_from_this();
}

Component::ConstPtr Component::follow() const
{
  return shared_from_this();
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::rename ( const std::string& name )
{
  const std::string new_name = name;
  if ( new_name == m_name.path() ) // skip if name does not change
    return;

  const std::string old_name = m_name.path();

  // notification should be done before the real renaming since the path changes
  raise_path_changed();

  URI new_full_path = m_path / new_name;

  if( ! m_root.expired() ) // inform the root about the change in path
    m_root.lock()->change_component_path( new_full_path , shared_from_this() );

  if ( is_not_null(m_raw_parent) ) // rename via parent to insure unique names
  {
    Component::Ptr parent = m_raw_parent->self();
    parent->remove_component( old_name );

    m_name = new_name;

    parent->add_component( shared_from_this() );
  }
  else  // direct rename in case of no parent
  {
    m_name = new_name;
  }

  /// @todo solve this differently,
  /// maybe putting finally uuid's in the comps and using the root to get the path

  // loop on children and inform them of change in name
  BOOST_FOREACH( CompStorage_t::value_type c, m_components )
  {
    c.second->change_parent( this );
  }
}

/////////////////////////////////////////////////////////////////////////////////////

Component::Ptr Component::get_parent()
{
  cf_assert( is_not_null(m_raw_parent) );
  return m_raw_parent->self();
}

Component::ConstPtr Component::get_parent() const
{
  cf_assert( is_not_null(m_raw_parent) );
  return m_raw_parent->self();
}

/////////////////////////////////////////////////////////////////////////////////////

Component::Ptr Component::add_component ( Component::Ptr subcomp )
{
  cf_assert( subcomp != nullptr );

  const std::string name = subcomp->name();

  const std::string unique_name = ensure_unique_name(subcomp);
  if ( name != unique_name )
  {
    subcomp->m_name = unique_name; // change name to unique
    CFinfo << "Component renamed from \'" << name  << "\' to unique name \'" << unique_name << "\'"  << CFendl;
  }

  m_components[unique_name] = subcomp;           // add to all component list
  m_dynamic_components[unique_name] = subcomp;   // add to dynamic component list

  subcomp->change_parent( this );

  raise_path_changed();

  return subcomp;
}

/////////////////////////////////////////////////////////////////////////////////////

Component::Ptr Component::add_static_component ( Component::Ptr subcomp )
{
  std::string unique_name = ensure_unique_name(subcomp);
  cf_always_assert_desc("static components must always have a unique name", unique_name == subcomp->name());
  m_components[unique_name] = subcomp;

  raise_path_changed();

  subcomp->change_parent( this );

  return subcomp;
}

/////////////////////////////////////////////////////////////////////////////////////

std::string Component::ensure_unique_name ( Component::Ptr subcomp )
{
  const std::string name = subcomp->name();
  std::string new_name = name;
  boost::regex e(name+"(_[0-9]+)?");
  BOOST_FOREACH(CompStorage_t::value_type subcomp_pair, m_components)
  {
    if (boost::regex_match(subcomp_pair.first,e))
    {
      Uint count = 1;

      new_name = name + "_" + String::to_str(count);

      // make sure constructed name does not exist
      while ( m_components.find(new_name) != m_components.end() )
      {
        ++count;
        new_name = name  + "_" + String::to_str(count);
      }

//      CFwarn << "Component named \'" << subcomp->full_path().string() << "\' already exists. Component renamed to \'" << new_name << "\'" << CFendl;

      break;
    }
  }
  return new_name;
}

/////////////////////////////////////////////////////////////////////////////////////


Component::Ptr Component::remove_component ( const std::string& name )
{
  // find the component exists
  Component::CompStorage_t::iterator itr = m_dynamic_components.find(name);

  if ( itr != m_dynamic_components.end() )         // if exists
  {
    Component::Ptr comp = itr->second;             // get the component

    // remove the component from the root
    if( !m_root.expired() )
      m_root.lock()->remove_component_path(comp->full_path());

    m_dynamic_components.erase(itr);               // remove it from the storage

    // remove from the list of all components
    Component::CompStorage_t::iterator citr = m_components.find(name);
    m_components.erase(citr);

    comp->change_parent( NULL );                   // set parent to invalid

    raise_path_changed();

    return comp;                                   // return it to client
  }
  else                                             // if does not exist
  {
    throw ValueNotFound(FromHere(), "Dynamic component with name '"
                        + name + "' does not exist in component '"
                        + this->name() + "' with path ["
                        + m_path.path() + "]");
  }
}

//////////////////////////////////////////////////////////////////////////////

void Component::complete_path ( URI& path ) const
{
  using namespace boost::algorithm;

//  CFinfo << "PATH [" << path.string() << "]\n" << CFflush;

  cf_assert( path.scheme() == URI::Scheme::CPATH );

  if(path.empty())
    path = "./";

  if ( is_null(m_raw_parent) )
    throw  InvalidURI(FromHere(), "Component \'" + name() + "\' has no parent");

  if (m_root.expired())
    throw  InvalidURI(FromHere(), "Component \'" + name() + "\' has no root");

  boost::shared_ptr<Component> parent = m_raw_parent->self();
  boost::shared_ptr<Component> root   = m_root.lock();

  std::string sp = path.path();

  if ( path.is_relative() ) // transform it to absolute
  {
    if ( starts_with(sp,"/") ) // remove leading "/" if any
      boost::algorithm::replace_first(sp, "/", "" );

    // substitute leading "../" for full_path() of parent
    if (starts_with(sp,".."))
    {
      std::string pfp = parent->full_path().path();
      boost::algorithm::replace_first(sp, "..", pfp);
    }
    else // substitute leading "./" for full_path() of this component
      if (starts_with(sp,"."))
      {
        boost::algorithm::replace_first(sp, ".", full_path().path());
      }
  }

  cf_assert ( URI(sp).is_absolute() );

  // break path in tokens and loop on them, while concatenaitng to a new path
  boost::char_separator<char> sep("/");
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tok (sp,sep);

  path = "/" ;
  std::string last;
  for(tokenizer::iterator el=tok.begin(); el!=tok.end(); ++el)
  {
    if ( equals (*el, ".") ) continue;     // substitute any "/./" for nothing

    if ( equals (*el, "..") )              // substitute any "../" for base path
      path = path.base_path();
    else
      path /= *el;
  }

//  CFinfo << "FINAL PATH: [" << path.string() << "]\n" << CFflush;

  cf_assert ( path.is_complete() );
}

/////////////////////////////////////////////////////////////////////////////////////

Component::Ptr Component::get_child(const std::string& name)
{
  const CompStorage_t::iterator found = m_components.find(name);
  if(found != m_components.end())
    return found->second;
  return Ptr();
}

Component::ConstPtr Component::get_child(const std::string& name) const
{
  const CompStorage_t::const_iterator found = m_components.find(name);
  if(found != m_components.end())
    return found->second;
  return ConstPtr();
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::change_parent ( Component* new_parent )
{
  if( !m_root.expired() )   // get the root and remove the current path
  {
   CRoot::Ptr root = m_root.lock();
    root->remove_component_path(full_path());
  }

  if( new_parent ) // valid ?
  {
    m_path = new_parent->full_path(); // modify the path
    m_root = new_parent->m_root;      // modify the root

    if( !m_root.expired() )   // get the root and set the new path
    {
      m_root.lock()->change_component_path( full_path() , shared_from_this() );
    }
  }
  else // new parent is invalid
  {
    m_path = "";
    m_root.reset();
  }

  // modifiy the parent, may be NULL
  m_raw_parent = new_parent;

  // modify the children
  BOOST_FOREACH( CompStorage_t::value_type c, m_components )
  {
    c.second->change_parent( this );
  }

  // raise_path_changed() event is raised for each child
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::move_to ( Component::Ptr new_parent )
{
  Component::Ptr this_ptr = get_parent()->remove_component( this->name() );
  new_parent->add_component( this_ptr );
  raise_path_changed();
}

/////////////////////////////////////////////////////////////////////////////////////

Component::ConstPtr Component::look_component ( const URI& path ) const
{
  if (!m_root.expired())  // root is available. This is a faster method.
  {
    URI lpath = path;

    complete_path(lpath); // ensure the path is complete

    // get the root
   CRoot::Ptr root = m_root.lock();

    return root->access_component(lpath);
  }
  else // we are in the case with no root. Hence the path must be relative
  {
    using namespace boost::algorithm;

    std::string sp = path.path();

    // break path in tokens and loop on them, while concatenaitng to a new path
    boost::char_separator<char> sep("/");
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    tokenizer tok (sp,sep);

    Component::ConstPtr look_comp = follow();
    std::string last;
    for(tokenizer::const_iterator el=tok.begin(); el!=tok.end(); ++el)
    {
      if ( equals (*el, ".") ) continue;     // substitute any "/./" for nothing

      if ( equals (*el, "..") )              // substitute any "../" for base path
        look_comp = look_comp->get_parent();
      else
      {
        Component::ConstPtr parent = look_comp;
        look_comp = look_comp->get_child(*el);
        if( is_null(look_comp) )
          throw ValueNotFound (FromHere(), "Component with name " + *el + " was not found in " + parent->full_path().path());
      }
    }
    return look_comp;
  }
}

/////////////////////////////////////////////////////////////////////////////////////

Component::Ptr Component::look_component ( const URI& path )
{
  if (!m_root.expired())  // root is available. This is a faster method.
  {
    URI lpath = path;

    complete_path(lpath); // ensure the path is complete

    // get the root
   CRoot::Ptr root = m_root.lock();

    return root->access_component(lpath);
  }
  else // we are in the case with no root. Hence the path must be relative
  {
    using namespace boost::algorithm;

    std::string sp = path.path();

    // break path in tokens and loop on them, while concatenaitng to a new path
    boost::char_separator<char> sep("/");
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    tokenizer tok (sp,sep);

    Component::Ptr look_comp = follow() ;
    std::string last;
    for(tokenizer::iterator el=tok.begin(); el!=tok.end(); ++el)
    {
      if ( equals (*el, ".") ) continue;     // substitute any "/./" for nothing

      if ( equals (*el, "..") )              // substitute any "../" for base path
        look_comp = look_comp->get_parent();
      else
      {
        Component::Ptr parent = look_comp;
        look_comp = look_comp->get_child(*el);
        if( is_null(look_comp) )
          throw ValueNotFound (FromHere(), "Component with name " + *el + " was not found in " + parent->full_path().path());
      }
    }
    return look_comp;
  }
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::create_component_signal ( XmlNode& node  )
{
  XmlParams p ( node );

  std::string name  = p.get_option<std::string>("Component name");
  std::string atype = p.get_option<std::string>("Generic type");
  std::string ctype = p.get_option<std::string>("Concrete type");

  bool basic = p.get_option<bool>("Basic mode");

  CFactories::Ptr factories = Core::instance().root()->get_child< CFactories >("Factories");
  CFactory::Ptr factory = factories->get_child< CFactory >( atype );
  if (!factory)
    throw ValueNotFound(FromHere(), "Factory of generic type " + atype + " not found");
  CBuilder::Ptr builder = factory->get_child< CBuilder >( ctype );
  if (!builder)
    throw ValueNotFound(FromHere(), "Builder of concrete type " + ctype + " not found in factory of generic type " + atype);

  Ptr comp = add_component( builder->build( name ) );

  if(basic)
    comp->mark_basic();
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::delete_component ( XmlNode& node  )
{
//  XmlParams p ( node );

//  URI path = p.get_option<URI>("Component");
//  if( ! path.is_protocol("cpath") )
//    throw ProtocolError( FromHere(), "Wrong protocol to access the Domain component, expecting a \'cpath\' but got \'" + path.string() +"\'");

//  Component::Ptr comp = look_component( path.path() )->get_parent();
//  Component::Ptr parent = comp->get_parent();
//  parent->remove_component( comp->name() );

  // when goes out of scope it gets deleted
  // unless someone else shares it
  Component::Ptr meself = self();
  Component::Ptr parent = meself->get_parent();

  parent->remove_component( meself->name() );
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::move_component ( XmlNode& node  )
{
  XmlParams p ( node );

  URI path = p.get_option<URI>("Path");
  if( path.scheme() != URI::Scheme::CPATH )
    throw ProtocolError( FromHere(), "Wrong protocol to access the Domain component, expecting a \'cpath\' but got \'" + path.string() +"\'");

  Component::Ptr new_parent = look_component( path.path() );

  this->move_to( new_parent );
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::print_info ( XmlNode& node  )
{
  CFinfo << "Info on component \'" << full_path().path() << "\'" << CFendl;

  CFinfo << "  sub components:" << CFendl;
  BOOST_FOREACH( CompStorage_t::value_type c, m_components )
  {
    if ( m_dynamic_components.find(c.first) == m_dynamic_components.end() )
      CFinfo << "  + [static]  ";
    else
      CFinfo << "  + [dynamic] ";

    CFinfo << c.second->name() << " / " << c.second->derived_type_name() << CFendl;
  }
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::write_xml_tree( XmlNode& node, bool put_all_content )
{
  std::string type_name = derived_type_name();

//  CFinfo << "xml tree for " << name() << CFendl;

  if(type_name.empty())
    CFerror << "Unknown derived name for " << DEMANGLED_TYPEID(*this)
            << ". Was this class added to the component builder?" << CFendl;
  else
  {
    XmlNode& this_node = *XmlOps::add_node_to( node, "node");
    XmlOps::add_attribute_to( this_node, "name", name() );
    XmlOps::add_attribute_to( this_node, "atype", type_name);
    XmlOps::add_attribute_to( this_node, "mode", has_tag("basic") ? "basic" : "adv");

    CLink::Ptr lnk = this->as_type<CLink>();
    if( is_not_null(lnk) ) // if it is a link, we put the target path as value
    {
      if ( lnk->is_linked() )
       this_node.value( this_node.document()->allocate_string( self()->full_path().path().c_str() ));
//      else
//        this_node.value( this_node.document()->allocate_string( "//Root" ));
    }
    else
    {
      if( put_all_content && !m_properties.store.empty() )
      {
        XmlNode& options = *XmlOps::add_node_to( this_node, XmlParams::tag_node_map());

        // add properties
        list_properties(options);
      }

      boost_foreach( CompStorage_t::value_type c, m_components )
      {
        c.second->write_xml_tree( this_node, put_all_content );
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::list_tree( XmlNode& node )
{
  XmlNode& reply = *XmlOps::add_reply_frame( node );

  XmlOps::add_attribute_to( reply, "sender", full_path().path() );

  write_xml_tree(reply, false);
}

/////////////////////////////////////////////////////////////////////////////////////

std::string Component::tree(Uint level) const
{
  std::string tree;
  for (Uint i=0; i<level; i++)
    tree += "  ";
  tree += name() + "\n";

  boost_foreach( CompStorage_t::value_type c, m_components )
  {
    tree += c.second->tree(level+1);
  }
  return tree;
}

/////////////////////////////////////////////////////////////////////////////////////

size_t Component::get_child_count() const
{
  return m_components.size();
}

/////////////////////////////////////////////////////////////////////////////////////

/// Adds an array to XML tree
/// @param params XmlParams object that manages the tree
/// @param array Array to add. It must be an OptionArrayT<TYPE>
template<typename TYPE>
void add_array_to_xml(XmlParams & params, const std::string & name,
                      boost::shared_ptr<OptionArray> array)
{
  boost::shared_ptr<OptionArrayT<TYPE> > optArray;
  bool basic = array->has_tag("basic");
  std::string desc = array->description();

  optArray = boost::dynamic_pointer_cast< OptionArrayT<TYPE> >(array);

  params.add_array(name, boost::any_cast< std::vector<TYPE> >(optArray->value()), desc, basic);
}

/////////////////////////////////////////////////////////////////////////////////////

/// Adds an array to XML tree
/// @param params XmlParams object that manages the tree
/// @param prop Property to add
template<typename TYPE>
void add_prop_to_xml(XmlParams & params, const std::string & name,
                     boost::shared_ptr<Property> prop)
{
  TYPE value = prop->value<TYPE>();

  if(!prop->is_option())
    params.add_property(name, value);
  else
  {
    Option & opt = prop->as_option();
    bool basic = opt.has_tag("basic");
    std::string desc = opt.description();
    XmlNode * node;

    if(opt.has_restricted_list())
      node = params.add_option(name, value, desc, basic, opt.restricted_list());
    else
      node = params.add_option(name, value, desc, basic);

    if(std::strcmp(opt.tag(), "uri") == 0)
    {
      std::vector<URI::Scheme::Type> prots = static_cast<OptionURI*>(&opt)->supported_protocols();
      std::vector<URI::Scheme::Type>::iterator it = prots.begin();

      for( ; it != prots.end() ; it++)
        XmlOps::add_attribute_to(*node, XmlParams::tag_attr_protocol(), URI::Scheme::Convert::instance().to_str(*it));
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::list_properties( XmlNode& node )
{
  PropertyList::PropertyStorage_t::iterator it = m_properties.store.begin();

  XmlParams p(*node.parent());

  for( ; it != m_properties.store.end() ; it++)
  {
    std::string name = it->first;
    Property::Ptr prop = it->second;

    if(std::strcmp(prop->tag(), "array") != 0)
    {
      std::string type = prop->type();

      if(type == "string")
        add_prop_to_xml<std::string>(p, name, prop);
      else if(type == "bool")
        add_prop_to_xml<bool>(p, name, prop);
      else if(type == "integer")
        add_prop_to_xml<int>(p, name, prop);
      else if(type == "unsigned")
        add_prop_to_xml<CF::Uint>(p, name, prop);
      else if(type == "real" || type == "double")
        add_prop_to_xml<CF::Real>(p, name, prop);
      else if(type == "uri")
        add_prop_to_xml<URI>(p, name, prop);
      else
        throw ShouldNotBeHere(FromHere(),
             std::string("Don't know how the manage \"") + type + "\" type.");
    }
    else
    {
      boost::shared_ptr<OptionArray> optArray;

      optArray = boost::dynamic_pointer_cast<OptionArray>(prop);

      const char * elem_type = optArray->elem_type();

      if(strcmp(elem_type, "string") == 0)
        add_array_to_xml< std::string >(p, name, optArray);
      else if(strcmp(elem_type, "bool") == 0)
        add_array_to_xml< bool >(p, name, optArray);
      else if(strcmp(elem_type, "integer") == 0)
        add_array_to_xml< int >(p, name, optArray);
      else if(strcmp(elem_type, "unsigned") == 0)
        add_array_to_xml< CF::Uint >(p, name, optArray);
      else if(strcmp(elem_type, "real") == 0 || strcmp(elem_type, "double") == 0)
        add_array_to_xml< CF::Real >(p, name, optArray);
      else if(strcmp(elem_type, "uri") == 0)
        add_array_to_xml< URI >(p, name, optArray);
      else
        throw ShouldNotBeHere(FromHere(),
             std::string("Don't know how the manage OptionArrayT<") +
                  elem_type + ">.");
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::list_signals( XmlNode& node )
{
  sigmap_t::iterator it = m_signals.begin();

  XmlNode & value_node = *XmlOps::add_node_to(node, "value");

  XmlOps::add_attribute_to(value_node, XmlParams::tag_attr_key(), XmlParams::tag_key_signals());

  for( ; it != m_signals.end() ; it++)
  {
    XmlNode & map = *XmlParams::add_map_to(value_node, it->first, it->second.description);
    XmlOps::add_attribute_to(map, "name", it->second.readable_name);
    XmlOps::add_attribute_to(map, "hidden", it->second.is_hidden ? "true" : "false");
  }
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::configure ( XmlNode& node )
{
  XmlParams pn ( node );

  if ( pn.option_map == 0 )
    throw  Common::XmlError( FromHere(), "ConfigObject received  XML without a \'" + std::string(XmlParams::tag_node_map()) + "\' node" );

  // get the list of options
  PropertyList::PropertyStorage_t& options = m_properties.store;

  // loop on the param nodes
  for (XmlNode* itr =  pn.option_map->first_node(); itr; itr = itr->next_sibling() )
  {
    // search for the attribute with key
    XmlAttr* att = itr->first_attribute( XmlParams::tag_attr_key() );
    if ( att )
    {
      PropertyList::PropertyStorage_t::iterator opt = options.find( att->value() );
      if (opt != options.end() && opt->second->is_option())
        opt->second->as_option().configure_option(*itr);
    }
  }

  // add a reply frame
  /// @todo adapt when the new XML layer is in place
  XmlNode & reply_node = *XmlOps::add_reply_frame(node);
  XmlParams p_reply(reply_node);
  XmlNode & map_node = *p_reply.add_map(XmlParams::tag_key_options()); //XmlOps::add_node_to(reply_node, XmlParams::tag_node_map());

  XmlOps::deep_copy(*pn.option_map, map_node);
}

/////////////////////////////////////////////////////////////////////////////////////

const Property& Component::property( const std::string& optname ) const
{
  return m_properties.property(optname);
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::rename_component ( XmlNode& node )
{
  XmlParams p(node);

  std::string new_name = p.get_option<std::string>("New name");

  rename(new_name);
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::save_tree ( XmlNode& node )
{
  XmlParams p(node);
  URI filename = p.get_option<URI>("filename");

  if(filename.scheme() != URI::Scheme::FILE)
    throw InvalidURI(FromHere(), "A file was expected but got \'" + filename.string() + "\'");

  save_tree_to( filename.path() );
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::list_content( XmlNode& node )
{
  XmlNode& reply = *XmlOps::add_reply_frame( node );
  XmlNode& map_node = *XmlOps::add_node_to(reply, XmlParams::tag_node_map());

  XmlOps::add_attribute_to( reply, "sender", full_path().path() );

  list_properties(map_node);
  list_signals(map_node);
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::signal_signature( XmlNode & node )
{
  XmlParams p(node);
  XmlNode & reply = *XmlOps::add_reply_frame(node);

  XmlOps::add_attribute_to( reply, "sender", full_path().path() );

  ( *signal( p.get_option<std::string>("name") ).signature )(reply);
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::save_tree_to( const boost::filesystem::path & path )
{
  boost::shared_ptr<XmlDoc> xmldoc = XmlOps::create_doc();
  XmlNode & doc_node = *XmlOps::goto_doc_node(*xmldoc.get());

  write_xml_tree(doc_node, true);

  XmlOps::write_xml_node(*xmldoc.get(), path);

  CFinfo << "Tree saved to '" << path.string() << "'" << CFendl;
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::raise_path_changed ()
{
  raise_event("tree_updated");
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::raise_event ( const std::string & name )
{
  if( !m_root.expired() )
  {
   CRoot::Ptr root = m_root.lock();

    root->raise_new_event(name, full_path());
  }
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::mark_basic()
{
  add_tag("basic");
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::create_component_signature( XmlNode& node )
{
  XmlParams p(node);

  p.add_option("Component name", std::string(), "Name for created component.");
  p.add_option("Generic name", std::string(), "Generic type of the component.");
  p.add_option("Concrete type", std::string(), "Concrete type of the component.");
  p.add_option("Basic mode", false, "Component will be visible in basic mode.");
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::rename_component_signature( XmlNode& node )
{
  XmlParams p(node);

  p.add_option("New name", std::string(), "Component new name.");
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::move_component_signature( XmlNode& node )
{
  XmlParams p(node);

  p.add_option("Path", std::string(), "Path to the new component to which this one will move to.");
}

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
