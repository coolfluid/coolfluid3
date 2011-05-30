// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3.
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

#include "rapidxml/rapidxml.hpp"

#include "Common/Log.hpp"
#include "Common/Signal.hpp"
#include "Common/Foreach.hpp"
#include "Common/CBuilder.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/OptionArray.hpp"
#include "Common/OptionURI.hpp"
#include "Common/StringConversion.hpp"
#include "Common/FindComponents.hpp"
#include "Common/Core.hpp"
#include "Common/OSystem.hpp"
#include "Common/LibLoader.hpp"

#include "Common/XML/Protocol.hpp"
#include "Common/XML/FileOperations.hpp"
#include "Common/XML/SignalOptions.hpp"

using namespace CF::Common::XML;

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////////////////

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

  regist_signal( "create_component" , "creates a component", "Create component" )->signal->connect ( boost::bind ( &Component::signal_create_component, this, _1 ) );

  regist_signal( "list_tree" , "lists the component tree inside this component", "List tree" )->signal->connect ( boost::bind ( &Component::signal_list_tree, this, _1 ) );

  regist_signal( "list_properties" , "lists the options of this component", "List properties" )->signal->connect ( boost::bind ( &Component::signal_list_properties, this, _1 ) );

  regist_signal( "list_signals" , "lists the options of this component", "List signals" )->signal->connect ( boost::bind ( &Component::signal_list_signals, this, _1 ) );

  regist_signal( "configure" , "configures this component", "Configure" )->signal->connect ( boost::bind ( &Component::signal_configure, this, _1 ) );

  regist_signal( "print_info" , "prints info on this component", "Info" )->signal->connect ( boost::bind ( &Component::signal_print_info, this, _1 ) );

  regist_signal( "rename_component" , "Renames this component", "Rename" )->signal->connect ( boost::bind ( &Component::signal_rename_component, this, _1 ) );

  regist_signal( "delete_component" , "Deletes a component", "Delete" )->signal->connect ( boost::bind ( &Component::signal_delete_component, this, _1 ) );

  regist_signal( "move_component" , "Moves a component to another component", "Move" )->signal->connect ( boost::bind ( &Component::signal_move_component, this, _1 ) );

  regist_signal( "save_tree", "Saves the tree", "Save tree")->signal->connect( boost::bind(&Component::signal_save_tree, this, _1) );

  regist_signal( "list_content", "Lists component content", "List content")->signal->connect(boost::bind(&Component::signal_list_content, this, _1));

  regist_signal( "signal_signature", "Gives signature of a signal", "")->signal->connect( boost::bind(&Component::signal_signature, this, _1));

  // these signals should not be seen from the GUI
  signal("list_tree")->is_hidden = true;
  signal("list_properties")->is_hidden = true;
  signal("list_signals")->is_hidden = true;
  signal("configure")->is_hidden = true;
  signal("save_tree")->is_hidden = true;
  signal("list_content")->is_hidden = true;
  signal("signal_signature")->is_hidden = true;

  // these signals are read-only
  signal("list_tree")->is_read_only = true;
  signal("list_content")->is_read_only = true;
  signal("signal_signature")->is_read_only = true;

  // signatures
  signal("create_component")->signature->connect( boost::bind(&Component::signature_create_component, this, _1) );
  signal("rename_component")->signature->connect( boost::bind(&Component::signature_rename_component, this, _1) );
  signal("move_component")->signature->connect( boost::bind(&Component::signature_move_component, this, _1) );

  // properties

  m_properties.add_property("brief", std::string("No brief description available"));
  m_properties.add_property("description", std::string("This component has not a long description"));
}

Component::~Component()
{
}

////////////////////////////////////////////////////////////////////////////////////////////

Component::Ptr Component::follow()
{
  return shared_from_this();
}

Component::ConstPtr Component::follow() const
{
  return shared_from_this();
}

std::string Component::derived_type_name() const
{
  return CF::Common::TypeInfo::instance().portable_types[ typeid(*this).name() ];
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::rename ( const std::string& name )
{
  const std::string new_name = name;
  if ( new_name == m_name.path() ) // skip if name does not change
    return;

  const std::string old_name = m_name.path();

  // notification should be done before the real renaming since the path changes
  raise_path_changed();

  URI new_uri = m_path / new_name;

  if( ! m_root.expired() ) // inform the root about the change in path
    m_root.lock()->change_component_path( new_uri , shared_from_this() );

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

////////////////////////////////////////////////////////////////////////////////////////////

Component& Component::parent()
{
  cf_assert( is_not_null(m_raw_parent) );
  return *m_raw_parent;
}

Component const& Component::parent() const
{
  cf_assert( is_not_null(m_raw_parent) );
  return *m_raw_parent;
}

////////////////////////////////////////////////////////////////////////////////////////////

Component& Component::add_component ( Component::Ptr subcomp )
{
  const std::string name = subcomp->name();

  const std::string unique_name = ensure_unique_name(*subcomp);
  if ( name != unique_name )
  {
    subcomp->m_name = unique_name; // change name to unique
    CFinfo << "Component renamed from \'" << name  << "\' to unique name \'" << unique_name << "\'"  << CFendl;
  }

  m_components[unique_name] = subcomp;           // add to all component list
  m_dynamic_components[unique_name] = subcomp;   // add to dynamic component list

  subcomp->change_parent( this );

  raise_path_changed();

  return *subcomp;
}

////////////////////////////////////////////////////////////////////////////////////////////

Component& Component::add_component ( Component& subcomp )
{
  return add_component(subcomp.self());
}

////////////////////////////////////////////////////////////////////////////////////////////

Component& Component::add_static_component ( Component::Ptr subcomp )
{
  std::string unique_name = ensure_unique_name(*subcomp);
  cf_always_assert_desc("static components must always have a unique name", unique_name == subcomp->name());
  m_components[unique_name] = subcomp;

  raise_path_changed();

  subcomp->change_parent( this );
  subcomp->signal("rename_component")->is_hidden = true;
  subcomp->signal("delete_component")->is_hidden = true;
  subcomp->signal("move_component")->is_hidden   = true;

  return *subcomp;
}

////////////////////////////////////////////////////////////////////////////////////////////

Component& Component::add_static_component ( Component& subcomp )
{
  return add_static_component(subcomp.self());
}

////////////////////////////////////////////////////////////////////////////////

bool Component::is_child_static(const std::string& name) const
{
  if (m_dynamic_components.find(name)==m_dynamic_components.end())
    return true;
  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////

std::string Component::ensure_unique_name ( Component& subcomp )
{
  const std::string name = subcomp.name();
  std::string new_name = name;
  boost::regex e(name+"(_[0-9]+)?");
  BOOST_FOREACH(CompStorage_t::value_type subcomp_pair, m_components)
  {
    if (boost::regex_match(subcomp_pair.first,e))
    {
      Uint count = 1;

      new_name = name + "_" + to_str(count);

      // make sure constructed name does not exist
      while ( m_components.find(new_name) != m_components.end() )
      {
        ++count;
        new_name = name  + "_" + to_str(count);
      }

//      CFwarn << "Component named \'" << subcomp->uri().string() << "\' already exists. Component renamed to \'" << new_name << "\'" << CFendl;

      break;
    }
  }
  return new_name;
}

////////////////////////////////////////////////////////////////////////////////////////////


Component::Ptr Component::remove_component ( const std::string& name )
{
  // find the component exists
  Component::CompStorage_t::iterator itr = m_dynamic_components.find(name);

  if ( itr != m_dynamic_components.end() )         // if exists
  {
    Component::Ptr comp = itr->second;             // get the component

    // remove the component from the root
    if( !m_root.expired() )
      m_root.lock()->remove_component_path(comp->uri());

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

Component::Ptr Component::remove_component ( Component& subcomp )
{
  return remove_component(subcomp.name());
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

    // substitute leading "../" for uri() of parent
    if (starts_with(sp,".."))
    {
      std::string pfp = parent->uri().path();
      boost::algorithm::replace_first(sp, "..", pfp);
    }
    // substitute leading "./" for uri() of this component
    else if (starts_with(sp,"."))
    {
      boost::algorithm::replace_first(sp, ".", uri().path());
    }
    else
    {
      sp = uri().path()+"/"+sp;
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

////////////////////////////////////////////////////////////////////////////////////////////

Component& Component::get_child(const std::string& name)
{
  return * get_child_ptr_checked(name);
}

Component::Ptr Component::get_child_ptr(const std::string& name)
{
  const CompStorage_t::iterator found = m_components.find(name);
  if(found != m_components.end())
    return found->second;
  return Ptr();
}

Component::ConstPtr Component::get_child_ptr(const std::string& name) const
{
  const CompStorage_t::const_iterator found = m_components.find(name);
  if(found != m_components.end())
    return found->second;
  return ConstPtr();
}

Component::Ptr Component::get_child_ptr_checked(const std::string& name)
{
  const CompStorage_t::iterator found = m_components.find(name);
  if(found != m_components.end())
    return found->second;
  else
    throw ValueNotFound( FromHere(), "Component with name " + name + " was not found inside component " + uri().string() );
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::change_parent ( Component* new_parent )
{
  if( !m_root.expired() )   // get the root and remove the current path
  {
   CRoot::Ptr root = m_root.lock();
    root->remove_component_path(uri());
  }

  if( new_parent ) // valid ?
  {
    m_path = new_parent->uri(); // modify the path
    m_root = new_parent->m_root;      // modify the root

    if( !m_root.expired() )   // get the root and set the new path
    {
      m_root.lock()->change_component_path( uri() , shared_from_this() );
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

////////////////////////////////////////////////////////////////////////////////////////////

boost::iterator_range<Component::iterator> Component::children()
{
  return boost::make_iterator_range(begin(),end());
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::move_to ( Component& new_parent )
{
  Component::Ptr this_ptr = parent().remove_component( *this );
  new_parent.add_component( this_ptr );
  raise_path_changed();
}

////////////////////////////////////////////////////////////////////////////////////////////

Component& Component::access_component ( const URI& path )
{
  return *access_component_ptr_checked(path); // pointer is garuanteed to be valid
}

////////////////////////////////////////////////////////////////////////////////

const Component& Component::access_component ( const URI& path ) const
{
  Component::ConstPtr comp = access_component_ptr_checked(path);
  cf_assert( is_not_null(comp) );
  return *comp;
}

////////////////////////////////////////////////////////////////////////////////////////////

Component::Ptr Component::access_component_ptr ( const URI& path )
{
  Component::Ptr comp;
  if (!m_root.expired())  // root is available. This is a faster method.
  {
    URI lpath = path;

    complete_path(lpath); // ensure the path is complete

    // get the component from the root, always returns valid pointer
    comp = m_root.lock()->retrieve_component(lpath);
  }
  else // we are in the case with no root. Hence the path must be relative
  {
    using namespace boost::algorithm;

    cf_assert( path.is_relative() );

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
        look_comp = look_comp->parent().self();
      else
      {
        look_comp = look_comp->get_child_ptr(*el);
      }
      if (is_null(look_comp))
        return Ptr();
    }
    comp = look_comp;
  }
  return comp;
}

Component::ConstPtr Component::access_component_ptr ( const URI& path ) const
{
  Component::ConstPtr comp;
  if (!m_root.expired())  // root is available. This is a faster method.
  {
    URI lpath = path;

    complete_path(lpath); // ensure the path is complete

    // get the component from the root
    comp = m_root.lock()->retrieve_component(lpath);
  }
  else // we are in the case with no root. Hence the path must be relative
  {
    using namespace boost::algorithm;

    cf_assert( path.is_relative() );

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
        look_comp = look_comp->parent().self();
      else
      {
        look_comp = look_comp->get_child_ptr(*el);
      }
      if (is_null(look_comp))
        return ConstPtr();
    }
    comp = look_comp;
  }
  return comp;
}

////////////////////////////////////////////////////////////////////////////////

Component::Ptr Component::access_component_ptr_checked (const URI& path )
{
  Component::Ptr comp = access_component_ptr(path);
  if (is_null(comp))
    throw InvalidURI (FromHere(), "Component with path " + path.path() + " was not found in " + uri().path());
  return comp;
}

Component::ConstPtr Component::access_component_ptr_checked (const URI& path ) const
{
  Component::ConstPtr comp = access_component_ptr(path);
  if (is_null(comp))
    throw InvalidURI (FromHere(), "Component with path " + path.path() + " was not found in " + uri().path());
  return comp;
}

////////////////////////////////////////////////////////////////////////////////////////////

Component& Component::create_component (const std::string& name ,
                                        const std::string& builder_name )
{
  Component::Ptr comp = build_component(builder_name, name);
  return add_component( comp );
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signal_create_component ( SignalArgs& args  )
{
  SignalOptions options( args );

  std::string name  = "untitled";
  if (options.exists("name"))
    name = options.option<std::string>("name");

  std::string builder_name = options.option<std::string>("type");

  Component& comp = create_component( name, builder_name );

  if( options.exists("basic_mode") )
  {
    if (options.option<bool>("basic_mode"))
      comp.mark_basic();
  }
  else
  {
    comp.mark_basic();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signal_delete_component ( SignalArgs& args  )
{
//  XmlParams p ( node );

//  URI path = p.get_option<URI>("Component");
//  if( ! path.is_protocol("cpath") )
//    throw ProtocolError( FromHere(), "Wrong protocol to access the Domain component, expecting a \'cpath\' but got \'" + path.string() +"\'");

//  Component::Ptr comp = access_component_ptr( path.path() )->parent();
//  Component::Ptr parent = comp->parent();
//  parent->remove_component( comp->name() );

  // when goes out of scope it gets deleted
  // unless someone else shares it

  parent().remove_component( *this );
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signal_move_component ( SignalArgs& args  )
{
  SignalOptions options( args );

  URI path = options.option<URI>("Path");
  if( path.scheme() != URI::Scheme::CPATH )
    throw ProtocolError( FromHere(), "Wrong protocol to access the Domain component, expecting a \'cpath\' but got \'" + path.string() +"\'");

  this->move_to( access_component( path.path() ) );
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signal_print_info ( SignalArgs& args  )
{
  CFinfo << "Info on component \'" << uri().path() << "\'" << CFendl;

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

////////////////////////////////////////////////////////////////////////////////////////////

void Component::write_xml_tree( XmlNode& node, bool put_all_content )
{
  cf_assert( node.is_valid() );

  std::string type_name = derived_type_name();

//  CFinfo << "xml tree for " << name() << CFendl;

  if(type_name.empty())
    CFerror << "Unknown derived name for " << CF_DEMANGLE_TYPEID(*this)
            << ". Was this class added to the component builder?" << CFendl;
  else
  {
    XmlNode this_node = node.add_node( "node" );

    this_node.set_attribute( "name", name() );
    this_node.set_attribute( "atype", type_name );
    this_node.set_attribute( "mode", has_tag("basic") ? "basic" : "adv");

    CLink::Ptr lnk = boost::dynamic_pointer_cast<CLink>(shared_from_this());//this->as_ptr<CLink>();

    if( is_not_null(lnk.get()) ) // if it is a link, we put the target path as value
    {
      if ( lnk->is_linked() )
       this_node.content->value( this_node.content->document()->allocate_string( lnk->follow()->uri().string().c_str() ));
//      else
//        this_node.value( this_node.document()->allocate_string( "//Root" ));
    }
    else
    {
      if( put_all_content && !m_properties.store.empty() )
      {
        // add properties if needed
        SignalFrame sf(this_node);
        signal_list_properties( sf );
      }

      boost_foreach( CompStorage_t::value_type c, m_components )
      {
        c.second->write_xml_tree( this_node, put_all_content );
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signal_list_tree( SignalArgs& args )
{
  SignalFrame reply = args.create_reply( uri() );

  write_xml_tree(reply.main_map.content, false);
}

////////////////////////////////////////////////////////////////////////////////////////////

std::string Component::tree(Uint level) const
{
  std::string tree;
  for (Uint i=0; i<level; i++)
    tree += "  ";
  tree += name() ;

  if( is_link() && follow() )
    tree += " -> " + follow()->uri().string();

  tree += "\n";

  boost_foreach( CompStorage_t::value_type c, m_components )
  {
    tree += c.second->tree(level+1);
  }
  return tree;
}

////////////////////////////////////////////////////////////////////////////////////////////

size_t Component::count_children() const
{
  return m_components.size();
}

////////////////////////////////////////////////////////////////////////////////////////////

/// Adds an array to XML tree
/// @param map   Where to add the array
/// @param name  Name to give to the array
/// @param array Array to add. It must be an OptionArrayT<TYPE>
template<typename TYPE>
void add_array_to_xml(Map & map, const std::string & name,
                      boost::shared_ptr<OptionArray> array)
{
  boost::shared_ptr<OptionArrayT<TYPE> > optArray;
  bool basic = array->has_tag("basic");
  std::string desc = array->description();

  optArray = boost::dynamic_pointer_cast< OptionArrayT<TYPE> >(array);

  XmlNode array_node = map.set_array(name, optArray->value_vect(), " ; ");

  array_node.set_attribute( Protocol::Tags::attr_descr(), desc);
  array_node.set_attribute( "mode", (basic ? "basic" : "adv") );
  array_node.set_attribute( "is_option", "true" );
}

////////////////////////////////////////////////////////////////////////////////////////////

/// Adds a property to XML tree
/// @param map   Where to add the property
/// @param name  Name to give to the property
/// @param prop Property to add
template<typename TYPE>
void add_prop_to_xml(Map & map, const std::string & name,
                     boost::shared_ptr<Property> prop)
{

  if( prop->is_option() )
  {
    Option & opt = prop->as_option();
    XmlNode value_node = map.set_value( name, opt.value<TYPE>() );

    bool basic = opt.has_tag("basic");
    std::string desc = opt.description();

    value_node.set_attribute( Protocol::Tags::attr_descr(), desc);
    value_node.set_attribute( "is_option", to_str(prop->is_option()) );
    value_node.set_attribute( "mode", (basic ? "basic" : "adv") );

    if( opt.has_restricted_list() )
    {
      Map value_map(value_node);
      std::vector<TYPE> vect;
      std::vector<boost::any>::iterator it = opt.restricted_list().begin();

      for( ; it != opt.restricted_list().end() ; ++it )
        vect.push_back( boost::any_cast<TYPE>(*it) );

      value_map.set_array( Protocol::Tags::key_restricted_values(), vect, " ; " );
    }

    if(std::strcmp(opt.tag(), "uri") == 0)
    {
      std::vector<URI::Scheme::Type> prots = static_cast<OptionURI*>(&opt)->supported_protocols();
      std::vector<URI::Scheme::Type>::iterator it = prots.begin();

      for( ; it != prots.end() ; it++)
        value_node.set_attribute( Protocol::Tags::attr_uri_schemes(), URI::Scheme::Convert::instance().to_str(*it));
    }
  }
  else
  {
    map.set_value( name, prop->value<TYPE>() );
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signal_list_properties( SignalFrame& args )
{
  PropertyList::PropertyStorage_t::iterator it = m_properties.store.begin();

  Map & options = args.map( Protocol::Tags::key_properties() ).main_map;

  for( ; it != m_properties.store.end() ; it++)
  {
    std::string name = it->first;
    Property::Ptr prop = it->second;

    // if it is not an array
    if(std::strcmp(prop->tag(), Protocol::Tags::node_array()) != 0)
    {
      std::string type = prop->type();

      if(type == Protocol::Tags::type<std::string>())
        add_prop_to_xml<std::string>(options, name, prop);
      else if(type == Protocol::Tags::type<bool>())
        add_prop_to_xml<bool>(options, name, prop);
      else if(type == Protocol::Tags::type<int>())
        add_prop_to_xml<int>(options, name, prop);
      else if(type == Protocol::Tags::type<Uint>())
        add_prop_to_xml<Uint>(options, name, prop);
      else if(type == Protocol::Tags::type<Real>())
        add_prop_to_xml<Real>(options, name, prop);
      else if(type == Protocol::Tags::type<URI>())
        add_prop_to_xml<URI>(options, name, prop);
      else
        throw ShouldNotBeHere(FromHere(),
             std::string("Don't know how the manage \"") + type + "\" type.");
    }
    else
    {
      boost::shared_ptr<OptionArray> optArray;

      optArray = boost::dynamic_pointer_cast<OptionArray>(prop);

      const char * elem_type = optArray->elem_type();

      if(strcmp(elem_type, Protocol::Tags::type<std::string>()) == 0)
        add_array_to_xml< std::string >(options, name, optArray);
      else if(strcmp(elem_type, Protocol::Tags::type<bool>()) == 0)
        add_array_to_xml< bool >(options, name, optArray);
      else if(strcmp(elem_type, Protocol::Tags::type<int>()) == 0)
        add_array_to_xml< int >(options, name, optArray);
      else if(strcmp(elem_type, Protocol::Tags::type<Uint>()) == 0)
        add_array_to_xml< Uint >(options, name, optArray);
      else if(strcmp(elem_type, Protocol::Tags::type<Real>()) == 0)
        add_array_to_xml< Real >(options, name, optArray);
      else if(strcmp(elem_type, Protocol::Tags::type<URI>()) == 0)
        add_array_to_xml< URI >(options, name, optArray);
      else
        throw ShouldNotBeHere(FromHere(),
             std::string("Don't know how the manage OptionArrayT<") +
                  elem_type + ">.");
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signal_list_signals( SignalArgs& args )
{
  sigmap_t::iterator it = m_signals.begin();

  XmlNode value_node = args.main_map.content.add_node( Protocol::Tags::node_value() );

  value_node.set_attribute( Protocol::Tags::attr_key(), Protocol::Tags::key_signals() );

  for( ; it != m_signals.end() ; it++)
  {
    XmlNode signal_node = value_node.add_node( Protocol::Tags::node_map() );

    signal_node.set_attribute( Protocol::Tags::attr_key(), it->first );
    signal_node.set_attribute( Protocol::Tags::attr_descr(), it->second->description );
    signal_node.set_attribute( "name", it->second->readable_name );
    signal_node.set_attribute( "hidden", to_str(it->second->is_hidden) );
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signal_configure ( SignalArgs& args )
{
  using namespace rapidxml;

  if ( !args.has_map( Protocol::Tags::key_options() ) )
    throw  Common::XmlError( FromHere(), "ConfigObject received  XML without a \'" + std::string(Protocol::Tags::key_options()) + "\' map" );

  XmlNode opt_map = args.map( Protocol::Tags::key_options() ).main_map.content;

  // get the list of options
  PropertyList::PropertyStorage_t& options = m_properties.store;

  // loop on the param nodes
  for (xml_node<>* itr =  opt_map.content->first_node(); itr; itr = itr->next_sibling() )
  {
    // search for the attribute with key
    xml_attribute<>* att = itr->first_attribute( Protocol::Tags::attr_key() );
    if ( is_not_null(att) )
    {
      PropertyList::PropertyStorage_t::iterator opt = options.find( att->value() );
      if (opt != options.end() && opt->second->is_option())
      {
        XmlNode node(itr);
        opt->second->as_option().configure_option( node );
      }
    }
  }

  // add a reply frame
  SignalFrame reply = args.create_reply( uri() );
  Map map_node = reply.map( Protocol::Tags::key_options() ).main_map;

  opt_map.deep_copy( map_node.content );
}

////////////////////////////////////////////////////////////////////////////////////////////

const Property& Component::property( const std::string& optname ) const
{
  return m_properties.property(optname);
}

////////////////////////////////////////////////////////////////////////////////

Property& Component::property( const std::string& optname )
{
  return m_properties.property(optname);
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signal_rename_component ( SignalArgs& args )
{
  SignalOptions options( args );

  std::string new_name = options.option<std::string>("New name");

  rename(new_name);
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signal_save_tree ( SignalArgs& args )
{
  SignalOptions options( args );

  const URI filename = options.option<URI>("filename");

  if(filename.scheme() != URI::Scheme::FILE)
    throw InvalidURI(FromHere(), "A file was expected but got \'" + filename.string() + "\'");

  XmlDoc::Ptr xmldoc = Protocol::create_doc();
  XmlNode doc_node = Protocol::goto_doc_node(*xmldoc.get());

  write_xml_tree(doc_node, true);

  XML::to_file( doc_node, filename.path() );

  CFinfo << "Tree saved to '" << filename.path() << "'" << CFendl;

}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signal_list_content( SignalArgs& args )
{
  SignalFrame reply = args.create_reply( uri() );

  signal_list_properties(reply);
  signal_list_signals(reply);
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signal_signature( SignalArgs & args )
{
  SignalFrame reply = args.create_reply( uri() );
  SignalOptions options( args );

  try
  {
    ( *signal( options.option<std::string>("name") )->signature )(reply);
  }
  catch(Exception & e)
  {
    reply.node.content->parent()->remove_node(reply.node.content);
    throw;
  }

}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::raise_path_changed ()
{
  raise_event("tree_updated");
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::raise_event ( const std::string & name )
{
  if( !m_root.expired() )
  {
   CRoot::Ptr root = m_root.lock();

    root->raise_new_event(name, uri());
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

Component& Component::mark_basic()
{
  add_tag("basic");
  return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signature_create_component( SignalArgs& args )
{
  SignalOptions options( args );

  options.add("name", std::string("untitled") , "Name for created component.");
  options.add("type", std::string("CF.Common.CGroup"), "Concrete type of the component.");
  options.add("basic_mode", true, "Component will be visible in basic mode.");
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signature_rename_component( SignalArgs& args )
{
  SignalOptions options( args );

  options.add("name", std::string(), "Component new name.");
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signature_move_component( SignalArgs& args )
{
  SignalOptions options( args );

  options.add("path", std::string(), "Path to the new component to which this one will move to.");
}

////////////////////////////////////////////////////////////////////////////////

Component& Component::configure_property(const std::string& optname, const boost::any& val)
{
  m_properties.configure_property(optname,val);
  return *this;
}

////////////////////////////////////////////////////////////////////////////////

void Component::configure_option_recursively(const std::string& opt, const boost::any& val)
{
  if (properties().check(opt))
  {
    configure_property(opt,val);
  }

  foreach_container((std::string name) (boost::shared_ptr<Property> prop), properties())
  {
    if (prop->has_tag(opt))
      configure_property(name,val);
  }

  // configure all child's options recursively
  boost_foreach( Component& component, find_components_recursively(*this) )
  {
    if (component.properties().check(opt))
    {
      component.configure_property(opt,val);
    }
    foreach_container((std::string name) (boost::shared_ptr<Property> prop), component.properties())
    {
      if (prop->has_tag(opt))
        component.configure_property(name,val);
    }

  }
}

////////////////////////////////////////////////////////////////////////////////

void Component::change_property(const std::string args)
{
  // extract:   variable_name:type=value   or   variable_name:array[type]=value1,value2
  boost::regex expression(  "([[:word:]]+)(\\:([[:word:]]+)(\\[([[:word:]]+)\\])?=(.*))?"  );
  boost::match_results<std::string::const_iterator> what;

  std::string name;
  std::string type;
  std::string subtype; // in case of array<type>
  std::string value;

  if (regex_search(args,what,expression))
  {
    name=what[1];
    type=what[3];
    subtype=what[5];
    value=what[6];
    // CFinfo << name << ":" << type << (subtype.empty() ? std::string() : std::string("["+subtype+"]"))  << "=" << value << CFendl;

    if      (type == "bool")
      properties()[name]=from_str<bool>(value);
    else if (type == "unsigned")
      properties()[name]=from_str<Uint>(value);
    else if (type == "integer")
      properties()[name]=from_str<int>(value);
    else if (type == "real")
      properties()[name]=from_str<Real>(value);
    else if (type == "string")
      properties()[name]=value;
    else if (type == "uri")
      properties()[name]=from_str<URI>(value);
    else if (type == "array")
    {
      std::vector<std::string> array;

      // the strings could have comma's inside, brackets, etc...
      Uint in_brackets(0);
      std::string::iterator first = value.begin();
      std::string::iterator it = first;
      for ( ; it!=value.end(); ++it)
      {
        if (*it == '(') // opening bracket
          ++in_brackets;
        else if (*it == ')') // closing bracket
          --in_brackets;
        else if (*it == ',' && in_brackets == 0)
        {
          array.push_back(std::string(first,it));
          boost::algorithm::trim(array.back());
          first = it+1;
        }
      }
      array.push_back(std::string(first,it));
      boost::algorithm::trim(array.back());

      if (subtype == "bool")
      {
        std::vector<bool> vec; vec.reserve(array.size());
        boost_foreach(const std::string& str_val,array)
            vec.push_back(from_str<bool>(str_val));
        properties()[name]=vec;
      }
      else if (subtype == "unsigned")
      {
        std::vector<Uint> vec; vec.reserve(array.size());
        boost_foreach(const std::string& str_val,array)
            vec.push_back(from_str<Uint>(str_val));
        properties()[name]=vec;
      }
      else if (subtype == "integer")
      {
        std::vector<int> vec; vec.reserve(array.size());
        boost_foreach(const std::string& str_val,array)
            vec.push_back(from_str<int>(str_val));
        properties()[name]=vec;
      }
      else if (subtype == "real")
      {
        std::vector<Real> vec; vec.reserve(array.size());
        boost_foreach(const std::string& str_val,array)
            vec.push_back(from_str<Real>(str_val));
        properties()[name]=vec;
      }
      else if (subtype == "string")
      {
        properties()[name]=array;
      }
      else if (subtype == "uri")
      {
        std::vector<URI> vec; vec.reserve(array.size());
        boost_foreach(const std::string& str_val,array)
            vec.push_back(from_str<URI>(str_val));
        properties()[name]=vec;
      }

    }
    else
      throw ParsingFailed(FromHere(), "The type ["+type+"] of passed argument [" + args + "] for ["+ uri().path() +"] is invalid.\n"+
                          "Format should be:\n"
                          " -  for simple types:  variable_name:type=value\n"
                          " -  for array types:   variable_name:array[type]=value1,value2\n"
                          "  with possible type: [bool,unsigned,integer,real,string,uri]");
  }
  else
    throw ParsingFailed(FromHere(), "Could not parse [" + args + "] in ["+ uri().path() +"].\n"+
                        "Format should be:\n"
                        " -  for simple types:  variable_name:type=value\n"
                        " -  for array types:   variable_name:array[type]=value1,value2\n"
                        "  with possible type: [bool,unsigned,integer,real,string,uri]");
}

void Component::configure (const std::vector<std::string>& args)
{
  // extract:   variable_name:type=value   or   variable_name:array[type]=value1,value2
  boost::regex expression(  "([[:word:]]+)(\\:([[:word:]]+)(\\[([[:word:]]+)\\])?=(.*))?"  );
  boost::match_results<std::string::const_iterator> what;

  boost_foreach (const std::string& arg, args)
  {
    std::string name;
    std::string type;
    std::string subtype; // in case of array<type>
    std::string value;

    if (regex_search(arg,what,expression))
    {
      name=what[1];
      type=what[3];
      subtype=what[5];
      value=what[6];
      // CFinfo << name << ":" << type << (subtype.empty() ? std::string() : std::string("["+subtype+"]"))  << "=" << value << CFendl;
      if (properties().check(name) == false) // not found
        throw ValueNotFound(FromHere(), "Option with name ["+name+"] not found in "+ uri().path());

      if      (type == "bool")
        configure_property(name,from_str<bool>(value));
      else if (type == "unsigned")
        configure_property(name,from_str<Uint>(value));
      else if (type == "integer")
        configure_property(name,from_str<int>(value));
      else if (type == "real")
        configure_property(name,from_str<Real>(value));
      else if (type == "string")
        configure_property(name,value);
      else if (type == "uri")
        configure_property(name,from_str<URI>(value));
      else if (type == "array")
      {
        std::vector<std::string> array;

        // the strings could have comma's inside, brackets, etc...
        Uint in_brackets(0);
        std::string::iterator first = value.begin();
        std::string::iterator it = first;
        for ( ; it!=value.end(); ++it)
        {
          if (*it == '(') // opening bracket
            ++in_brackets;
          else if (*it == ')') // closing bracket
            --in_brackets;
          else if (*it == ',' && in_brackets == 0)
          {
            array.push_back(std::string(first,it));
            boost::algorithm::trim(array.back());
            first = it+1;
          }
        }
        array.push_back(std::string(first,it));
        boost::algorithm::trim(array.back());

        if (subtype == "bool")
        {
          std::vector<bool> vec; vec.reserve(array.size());
          boost_foreach(const std::string& str_val,array)
            vec.push_back(from_str<bool>(str_val));
          configure_property(name,vec);
        }
        else if (subtype == "unsigned")
        {
          std::vector<Uint> vec; vec.reserve(array.size());
          boost_foreach(const std::string& str_val,array)
            vec.push_back(from_str<Uint>(str_val));
          configure_property(name,vec);
        }
        else if (subtype == "integer")
        {
          std::vector<int> vec; vec.reserve(array.size());
          boost_foreach(const std::string& str_val,array)
            vec.push_back(from_str<int>(str_val));
          configure_property(name,vec);
        }
        else if (subtype == "real")
        {
          std::vector<Real> vec; vec.reserve(array.size());
          boost_foreach(const std::string& str_val,array)
            vec.push_back(from_str<Real>(str_val));
          configure_property(name,vec);
        }
        else if (subtype == "string")
        {
          configure_property(name,array);
        }
        else if (subtype == "uri")
        {
          std::vector<URI> vec; vec.reserve(array.size());
          boost_foreach(const std::string& str_val,array)
            vec.push_back(from_str<URI>(str_val));
          configure_property(name,vec);
        }

      }
      else
        throw ParsingFailed(FromHere(), "The type ["+type+"] of passed argument [" + arg + "] for ["+ uri().path() +"] is invalid.\n"+
          "Format should be:\n"
          " -  for simple types:  variable_name:type=value\n"
          " -  for array types:   variable_name:array[type]=value1,value2\n"
          "  with possible type: [bool,unsigned,integer,real,string,uri]");
    }
    else
      throw ParsingFailed(FromHere(), "Could not parse [" + arg + "] in ["+ uri().path() +"].\n"+
         "Format should be:\n"
         " -  for simple types:  variable_name:type=value\n"
         " -  for array types:   variable_name:array[type]=value1,value2\n"
         "  with possible type: [bool,unsigned,integer,real,string,uri]");
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

Component::Ptr build_component(const std::string& builder_name,
                               const std::string& name,
                               const std::string& factory_type_name )
{
  // get the factories

  Component::Ptr factories = Core::instance().root().get_child_ptr("Factories");
  if ( is_null(factories) )
    throw ValueNotFound( FromHere(), "CFactories \'Factories\' not found in "
                        + Core::instance().root().uri().string() );

  // get the factory holding the builder

  Component::Ptr factory = factories->get_child_ptr( factory_type_name );
  if ( is_null(factory) )
  {
    const std::string lib_name = CBuilder::extract_library_name(builder_name);
    try // to auto-load in case factory not there
    {
      CFinfo << "Auto-loading plugin " << lib_name << CFendl;
      OSystem::instance().lib_loader()->load_library(lib_name);
      factory = factories->get_child_ptr( factory_type_name );
    }
    catch(const std::exception& e)
    {
      throw ValueNotFound(FromHere(),
                          "Failed to auto-load plugin " + lib_name + ": " + e.what());
    }
  }

  if ( is_null(factory) )
    throw ValueNotFound( FromHere(),
                        "CFactory \'" + factory_type_name
                        + "\' not found in " + factories->uri().string() + "." );

  // get the builder

  Component::Ptr builder = factory->get_child_ptr( builder_name );
  if ( is_null(builder) )
  {
    const std::string lib_name = CBuilder::extract_library_name(builder_name);
    try // to auto-load in case builder not there
    {
      CFinfo << "Auto-loading plugin " << lib_name << CFendl;
      OSystem::instance().lib_loader()->load_library(lib_name);
      builder = factory->get_child_ptr( builder_name );
    }
    catch(const std::exception& e)
    {
      throw ValueNotFound(FromHere(),
                          "Failed to auto-load plugin " + lib_name + ": " + e.what());
    }
  }

  if ( is_null(builder) )
  {
    std::string msg = "CBuilder \'" + builder_name + "\' not found in factory \'"
        + factory_type_name + "\'. Probably forgot to load a library.\n"
        + "Possible builders:";
    boost_foreach(Component& comp, factory->children())
        msg += "\n  -  "+comp.name();

    throw ValueNotFound( FromHere(), msg );
  }

  // build the component

  Component::Ptr comp = builder->as_type<CBuilder>().build ( name );
  if ( is_null(comp) )
    throw NotEnoughMemory ( FromHere(),
                           "CBuilder \'" + builder_name
                           + "\' failed to allocate component with name \'" + name + "\'" );


  return comp;
}

////////////////////////////////////////////////////////////////////////////////////////////

Component::Ptr build_component(const std::string& builder_name,
                               const std::string& name )
{
  std::string builder_namespace = CBuilder::extract_namespace(builder_name);

  URI builder_path = Core::instance().libraries().uri()
                   / URI(builder_namespace)
                   / URI(builder_name);

  const CBuilder& builder =
      Core::instance().root().access_component( builder_path ).follow()
      ->as_type<CBuilder const>();

  Component::Ptr comp = builder.build( name );

  return comp;
}

////////////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
