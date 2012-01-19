// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3.
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "rapidxml/rapidxml.hpp"

#include "common/BoostAnyConversion.hpp"
#include "common/Log.hpp"
#include "common/Signal.hpp"
#include "common/Foreach.hpp"
#include "common/Builder.hpp"
#include "common/BasicExceptions.hpp"
#include "common/EventHandler.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/OptionURI.hpp"
#include "common/PropertyList.hpp"
#include "common/StringConversion.hpp"
#include "common/FindComponents.hpp"
#include "common/Core.hpp"
#include "common/OSystem.hpp"
#include "common/LibLoader.hpp"
#include "common/PropertyList.hpp"
#include "common/ComponentIterator.hpp"
#include "common/UUCount.hpp"


#include "common/XML/Protocol.hpp"
#include "common/XML/FileOperations.hpp"
#include "common/XML/SignalOptions.hpp"

using namespace cf3::common::XML;

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////////////////

Component::Component ( const std::string& name ) :
    m_name (),
    m_properties(new PropertyList()),
    m_options(new OptionList()),
    m_parent(0)
{
  // accept name

  if (!URI::is_valid_element( name ))
    throw InvalidURI(FromHere(), "Component name ["+name+"] is invalid");
  m_name = name;

  // signals

  regist_signal( "create_component" )
      .connect( boost::bind( &Component::signal_create_component, this, _1 ) )
      .description("creates a component")
      .pretty_name("Create component")
      .signature( boost::bind(&Component::signature_create_component, this, _1) );

  regist_signal( "list_tree" )
      .connect( boost::bind( &Component::signal_list_tree, this, _1 ) )
      .hidden(true)
      .read_only(true)
      .description("lists the component tree inside this component")
      .pretty_name("List tree");

  regist_signal( "print_tree" )
      .connect( boost::bind( &Component::signal_print_tree, this, _1 ) )
      .hidden(false)
      .read_only(true)
      .description("Print the component tree inside this component")
      .pretty_name("Print tree")
      .signature( boost::bind(&Component::signature_print_tree, this, _1) );

  regist_signal( "list_properties" )
      .connect( boost::bind( &Component::signal_list_properties, this, _1 ) )
      .hidden(true)
      .description("lists the properties of this component")
      .pretty_name("List properties");

  regist_signal( "list_options" )
      .connect( boost::bind( &Component::signal_list_options, this, _1 ) )
      .hidden(true)
      .description("lists the options of this component")
      .pretty_name("List options");

  regist_signal( "list_signals" )
      .connect( boost::bind( &Component::signal_list_signals, this, _1 ) )
      .hidden(true)
      .description("lists the options of this component")
      .pretty_name("List signals");

  regist_signal( "configure" )
      .connect( boost::bind( &Component::signal_configure, this, _1 ) )
      .hidden(true)
      .description("configures this component")
      .pretty_name("Configure");

  regist_signal( "print_info" )
      .connect( boost::bind( &Component::signal_print_info, this, _1 ) )
      .description("prints info on this component")
      .pretty_name("Info");

  regist_signal( "rename_component" )
      .connect( boost::bind( &Component::signal_rename_component, this, _1 ) )
      .description("Renames this component")
      .pretty_name("Rename")
      .signature( boost::bind(&Component::signature_rename_component, this, _1) );

  regist_signal( "delete_component" )
      .connect( boost::bind( &Component::signal_delete_component, this, _1 ) )
      .description("Deletes a component")
      .pretty_name("Delete");

  regist_signal( "move_component" )
      .connect( boost::bind( &Component::signal_move_component, this, _1 ) )
      .description("Moves a component to another component")
      .pretty_name("Move")
      .signature( boost::bind(&Component::signature_move_component, this, _1) );

  regist_signal( "save_tree" )
      .connect( boost::bind( &Component::signal_save_tree, this, _1 ) )
      .hidden(true)
      .description("Saves the tree")
      .pretty_name("Save tree");

  regist_signal( "list_content" )
      .connect( boost::bind( &Component::signal_list_content, this, _1 ) )
      .hidden(true)
      .read_only(true)
      .description("Lists component content")
      .pretty_name("List content");

  regist_signal( "signal_signature" )
      .connect( boost::bind(&Component::signal_signature, this, _1))
      .hidden(true)
      .read_only(true)
      .description("Gives signature of a signal");


  // properties

  properties().add_property("brief", std::string("No brief description available"));
  properties().add_property("description", std::string("This component has not a long description"));
  properties().add_property("uuid", UUCount());

  // events
  EventHandler::instance().connect_to_event("ping", this, &Component::on_ping_event);
}


Component::~Component()
{
}


////////////////////////////////////////////////////////////////////////////////////////////


void Component::rename ( const std::string& name )
{
  if(name.empty())
    throw BadValue(FromHere(), "Empty new name given for " + uri().string());

  if(!URI::is_valid_element(name))
    throw BadValue(FromHere(), "Invalid new name given for " + uri().string());

  if(name == m_name) // skip if name does not change
    return;

  cf3_assert(m_component_lookup.size() == m_components.size());

  // notification should be done before the real renaming since the path changes
  raise_tree_updated_event();

  if(is_not_null(m_parent))
  {
    if(is_not_null(m_parent->get_child(name)))
      throw ValueExists(FromHere(), std::string("A component with name ") + this->name() + " already exists in " + uri().string());

    // Rename key in parent
    CompLookupT::iterator lookup = m_parent->m_component_lookup.find(m_name);
    const Uint idx = lookup->second;
    m_parent->m_component_lookup.erase(lookup);
    m_parent->m_component_lookup[name] = idx;
  }

  m_name = name;
}

////////////////////////////////////////////////////////////////////////////////////////////

URI Component::uri() const
{
  if(is_null(m_parent))
    return URI(std::string("/"), URI::Scheme::CPATH);

  return m_parent->uri() / URI(name(), URI::Scheme::CPATH);
}

Handle<Component> Component::parent() const
{
  if(m_parent)
    return m_parent->handle<Component>();

  return Handle<Component>();
}

////////////////////////////////////////////////////////////////////////////////////////////

Handle< const Component > Component::root() const
{
  Handle<Component const> result(handle<Component>());
  while(is_not_null(result->m_parent))
    result = result->parent();

  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////

Handle< Component > Component::root()
{
  Handle<Component> result(handle<Component>());
  while(is_not_null(result->m_parent))
    result = result->parent();

  return result;
}


////////////////////////////////////////////////////////////////////////////////////////////

Component& Component::add_component ( const boost::shared_ptr<Component>& subcomp )
{
  cf3_always_assert(is_not_null(subcomp));
  const std::string name = subcomp->name();

  const std::string unique_name = ensure_unique_name(*subcomp);
  if ( name != unique_name )
  {
    subcomp->m_name = unique_name; // change name to unique
    CFinfo << "Name conflict - component renamed from \'" << name
           << "\' to unique name \'" << unique_name << "\'"
           << CFendl;
  }

  m_component_lookup[unique_name] = m_components.size();
  m_components.push_back(subcomp);  // add to all component list

  cf3_assert(m_component_lookup.size() == m_components.size());

  subcomp->m_parent = this;

  raise_tree_updated_event();

  return *subcomp;
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::add_link(Component& linked_component)
{
  create_component<Link>(linked_component.name())->link_to(linked_component);
}

////////////////////////////////////////////////////////////////////////////////////////////

Component& Component::add_static_component ( const boost::shared_ptr< Component >& subcomp )
{
  std::string unique_name = ensure_unique_name(*subcomp);
  cf3_always_assert_desc("static components must always have a unique name", unique_name == subcomp->name());

  add_component(subcomp);

  subcomp->signal("rename_component")->hidden(true);
  subcomp->signal("delete_component")->hidden(true);
  subcomp->signal("move_component")->hidden(true);

  subcomp->add_tag(Tags::static_component());

  return *subcomp;
}

////////////////////////////////////////////////////////////////////////////////////////////

std::string Component::ensure_unique_name ( Component& subcomp )
{
  std::string new_name = subcomp.name();
  Uint count = 0;

  while(m_component_lookup.find(new_name) != m_component_lookup.end())
  {
    new_name = subcomp.name() + "_" + to_str(++count);
  }

  return new_name;
}

////////////////////////////////////////////////////////////////////////////////////////////


boost::shared_ptr<Component> Component::remove_component ( const std::string& name )
{
  // find the component exists
  Component::CompLookupT::iterator itr = m_component_lookup.find(name);

  if ( itr != m_component_lookup.end() )         // if exists
  {
    const Uint comp_idx = itr->second;
    boost::shared_ptr<Component> comp = m_components[comp_idx];             // get the component
    if(comp->has_tag(Tags::static_component()))
      throw BadValue(FromHere(), "Error removing component " + comp->uri().string() + ", it is static!");

    m_component_lookup.erase(itr);               // remove it from the lookup

    comp->change_parent( Handle<Component>() );                   // set parent to invalid

    // Create new storage to eliminate the removed component
    CompStorageT new_storage; new_storage.reserve(m_components.size() - 1);
    // Insert all components that were before the one that got removed
    new_storage.insert(new_storage.end(), m_components.begin(), m_components.begin() + (comp_idx));
    //Copy the ones after, adjusting their lookup index in the process
    const Uint after_deleted_begin = comp_idx + 1;
    const Uint after_deleted_end = m_components.size();
    for(Uint i = after_deleted_begin; i != after_deleted_end; ++i)
    {
      --m_component_lookup[m_components[i]->name()];
      new_storage.push_back(m_components[i]);
    }
    m_components = new_storage;

    raise_tree_updated_event();

    return comp;                                   // return it to client
  }
  else                                             // if does not exist
  {
    throw ValueNotFound(FromHere(), "Dynamic component with name '"
                        + name + "' does not exist in component '"
                        + this->name() + "' with path ["
                        + uri().path() + "]");
  }
}

//////////////////////////////////////////////////////////////////////////////

boost::shared_ptr<Component> Component::remove_component ( Component& subcomp )
{
  return remove_component(subcomp.name());
}

//////////////////////////////////////////////////////////////////////////////

void Component::complete_path ( URI& path ) const
{
  cf3_assert( path.scheme() == URI::Scheme::CPATH );

  path = access_component(path)->uri();
}

Handle<Component const> Component::get_child(const std::string& name) const
{
  const CompLookupT::const_iterator found = m_component_lookup.find(name);
  if(found != m_component_lookup.end())
    return Handle<Component const>(m_components[found->second]);
  return Handle<Component const>();
}

Handle<Component> Component::get_child(const std::string& name)
{
  const CompLookupT::iterator found = m_component_lookup.find(name);
  if(found != m_component_lookup.end())
    return Handle<Component>(m_components[found->second]);
  return Handle<Component>();
}

Handle<Component const> Component::get_child_checked(const std::string& name) const
{
  Handle<Component const> result = get_child(name);
  if(is_null(result))
    throw ValueNotFound( FromHere(), "Component with name " + name + " was not found inside component " + uri().string() );

  return result;
}

Handle<Component> Component::get_child_checked(const std::string& name)
{
  Handle<Component> result = get_child(name);
  if(is_null(result))
    throw ValueNotFound( FromHere(), "Component with name " + name + " was not found inside component " + uri().string() );

  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::change_parent(Handle<Component> to_parent)
{
  // modifiy the parent, may be NULL
  m_parent = to_parent.get();
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::move_to ( Component& new_parent )
{
  cf3_assert(m_parent);
  boost::shared_ptr<Component> this_ptr = m_parent->remove_component( *this );
  new_parent.add_component( this_ptr );
  raise_tree_updated_event();
}

////////////////////////////////////////////////////////////////////////////////////////////

Handle<Component> Component::access_component(const URI& path) const
{
  // Return self for trivial path or at end of recursion.
  if(path.path() == "." || path.empty())
    return const_cast<Component*>(this)->handle<Component>();

  // If the path is absolute, make it relative and pass it to the root
  if(path.is_absolute())
  {
    // String without protocol
    std::string new_path = path.path();

    // Remove any leading /
    boost::algorithm::trim_left_if(new_path, boost::algorithm::is_any_of("/"));

    if(new_path.empty())
    {
      return const_cast<Component*>(root().get())->handle<Component>();
    }

    // Pass the rest to root
    return root()->access_component(URI(new_path, cf3::common::URI::Scheme::CPATH));
  }

  // Relative path
  std::string path_str = path.path();
  // Remove trailing /
  boost::algorithm::trim_right_if(path_str, boost::algorithm::is_any_of("/"));

  // Find the first separator
  const std::size_t first_sep = path_str.find("/");
  const bool has_no_separator = (first_sep == std::string::npos);

  // Current part of the path to parse
  const std::string current_part = has_no_separator ? path_str : path_str.substr(0, first_sep);

  // Remainder of the path to parse, set to "." if there were no more parts
  const std::string next_part = has_no_separator ? "." : path_str.substr(first_sep+1, path_str.size());

  // Dispatch to self
  if(current_part == "." || current_part.empty())
    return access_component(next_part);

  // Dispatch to parent
  if(current_part == "..")
    return m_parent ? m_parent->access_component(next_part) : Handle<Component>();

  // Dispatch to child
  Handle<Component const> child = get_child(current_part);
  if(is_not_null(child))
    return child->access_component(next_part);

  // Return null if not found
  return Handle<Component>();
}

//Handle<Component const> Component::access_component(const URI& path) const
//{
//  return const_cast<Component*>(this)->access_component(path);
//}

////////////////////////////////////////////////////////////////////////////////

Handle<Component> Component::access_component_checked (const cf3::common::URI& path)
{
  Handle<Component> comp = access_component(path);
  if (is_null(comp))
    throw InvalidURI (FromHere(), "Component with path " + path.path() + " was not found in " + uri().path());
  return comp;
}

Handle<Component const> Component::access_component_checked (const cf3::common::URI& path) const
{
  Handle<Component const> comp = access_component(path);
  if (is_null(comp))
    throw InvalidURI (FromHere(), "Component with path " + path.path() + " was not found in " + uri().path());
  return comp;
}

////////////////////////////////////////////////////////////////////////////////////////////

Handle< Component > Component::create_component (const std::string& name ,
                                        const std::string& builder_name )
{
  boost::shared_ptr<Component> comp = build_component(builder_name, name);
  if(is_not_null(comp))
    add_component( comp );

  return Handle<Component>(comp);
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signal_create_component ( SignalArgs& args  )
{
  SignalOptions options( args );

  std::string name  = "untitled";
  if (options.check("name"))
    name = options.value<std::string>("name");

  std::string builder_name = options.value<std::string>("type");

  Handle<Component> comp = create_component( name, builder_name );

  if( options.check("basic_mode") )
  {
    if (options["basic_mode"].value<bool>())
      comp->mark_basic();
  }
  else
  {
    comp->mark_basic();
  }

  SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add_option("created_component", comp->uri());
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signal_delete_component ( SignalArgs& args  )
{
  cf3_assert(m_parent);
  m_parent->remove_component( *this );
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signal_move_component ( SignalArgs& args  )
{
  SignalOptions options( args );

  URI path = options.value<URI>("path");
  if( path.scheme() != URI::Scheme::CPATH )
    throw ProtocolError( FromHere(), "Wrong protocol to access the Domain component, expecting a \'cpath\' but got \'" + path.string() +"\'");

  this->move_to( *access_component( path.path() ) );
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signal_print_info ( SignalArgs& args  ) const
{
  CFinfo << "Info on component \'" << uri().path() << "\'" << CFendl;

  CFinfo << "  sub components:" << CFendl;
  BOOST_FOREACH( const Component& c, *this )
  {
    if (c.has_tag(Tags::static_component()))
      CFinfo << "  + [static]  ";
    else
      CFinfo << "  + [dynamic] ";

    CFinfo << c.name() << " / " << c.derived_type_name() << CFendl;
  }

  CFinfo << "  options:" << CFendl;
  CFinfo << options().list_options() << CFendl;

  CFinfo << "  properties:" << CFendl;
  typedef std::pair<const std::string,boost::any> Property_t;
  boost_foreach(const Property_t& property, properties() )
    CFinfo << property.first << "=" << properties().value_str(property.first) << CFendl;

}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::write_xml_tree( XmlNode& node, bool put_all_content ) const
{
  cf3_assert( node.is_valid() );

  std::string type_name = derived_type_name();

//  CFinfo << "xml tree for " << name() << CFendl;

  if(type_name.empty())
    CFerror << "Unknown derived name for " << cf3::common::demangle( typeid( *this ).name() )
            << ". Was this class added to the component builder?" << CFendl;
  else
  {
    XmlNode this_node = node.add_node( "node" );

    this_node.set_attribute( "name", name() );
    this_node.set_attribute( "atype", type_name );
    this_node.set_attribute( "mode", has_tag("basic") ? "basic" : "adv");
    this_node.set_attribute( "uuid", properties().value_str("uuid") );

    const Link* lnk = dynamic_cast<const Link*>(this);
    if( is_not_null(lnk) ) // if it is a link, we put the target path as value
    {
      if ( lnk->is_linked() )
       this_node.content->value( this_node.content->document()->allocate_string( lnk->follow()->uri().string().c_str() ));
//      else
//        this_node.value( this_node.document()->allocate_string( "/" ));
    }
    else
    {
      if( put_all_content && !properties().store.empty() )
      {
        // add properties if needed
        SignalFrame sf(this_node);
        signal_list_properties( sf );
        signal_list_options( sf );
      }

      boost_foreach( const Component& c, *this )
      {
        c.write_xml_tree( this_node, put_all_content );
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signal_list_tree( SignalArgs& args ) const
{
  SignalFrame reply = args.create_reply( uri() );

  write_xml_tree(reply.main_map.content, false);
}

////////////////////////////////////////////////////////////////////////////////////////////

std::string Component::tree(bool basic_mode, Uint depth, Uint recursion_level) const
{
  std::string tree;
  if (recursion_level<=depth || depth==0)
  {
    if ( !basic_mode || has_tag("basic") )
    {
      for (Uint i=0; i<recursion_level; i++)
        tree += "  ";
      tree += name() ;

      if( is_not_null(dynamic_cast<Link const*>(this)) )
      {
        Handle<Component const> linked = follow_link(*this);
        tree += " -> " + (is_null(linked) ? "": linked->uri().string());
      }

      tree += "\n";

      boost_foreach( const Component& c, *this )
      {
        tree += c.tree(basic_mode,depth,recursion_level+1);
      }
    }
  }
  return tree;
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signal_print_tree( SignalArgs& args ) const
{
  SignalOptions options( args );
  CFinfo << tree(options.value<bool>("basic_mode"),options.value<Uint>("depth")) << CFendl;
}

void Component::signature_print_tree( SignalArgs& args ) const
{
  SignalOptions options( args );

  options.add_option("basic_mode", false )
      .description("If false, only components marked as basic will be printed");
  options.add_option("depth", 0u )
      .description("Define howmany levels will be printed");
}

////////////////////////////////////////////////////////////////////////////////////////////

size_t Component::count_children() const
{
  return m_components.size();
}

////////////////////////////////////////////////////////////////////////////////////////////

PropertyList& Component::properties()
{
  return *m_properties;
}


////////////////////////////////////////////////////////////////////////////////////////////

const PropertyList& Component::properties() const
{
  return *m_properties;
}

////////////////////////////////////////////////////////////////////////////////////////////

OptionList& Component::options()
{
  return *m_options;
}

////////////////////////////////////////////////////////////////////////////////////////////

const OptionList& Component::options() const
{
  return *m_options;
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signal_list_properties( SignalFrame& args ) const
{
  PropertyList::PropertyStorage_t::const_iterator it = properties().store.begin();

 Map & options = args.map( Protocol::Tags::key_properties() ).main_map;

 for( ; it != properties().store.end() ; it++)
 {
   std::string name = it->first;
   boost::any value = it->second;

   std::string type = class_name_from_typeinfo( value.type() );

   if(type == Protocol::Tags::type<std::string>())
     options.set_value<std::string>( name, any_to_value<std::string>(value) );
   else if(type == Protocol::Tags::type<bool>())
     options.set_value<bool>( name, any_to_value<bool>(value) );
   else if(type == Protocol::Tags::type<int>())
     options.set_value<int>( name, any_to_value<int>(value) );
   else if(type == Protocol::Tags::type<Uint>())
     options.set_value<Uint>( name, any_to_value<Uint>(value) );
   else if(type == Protocol::Tags::type<Real>())
     options.set_value<Real>( name, any_to_value<Real>(value) );
   else if(type == Protocol::Tags::type<URI>())
     options.set_value<URI>( name, any_to_value<URI>(value) );
   else if(type == Protocol::Tags::type<UUCount>())
     options.set_value<UUCount>( name, any_to_value<UUCount>(value) );
   else
     throw ShouldNotBeHere(FromHere(),
                           std::string("Don't know how the manage [" + type + "] type."));
 }
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signal_list_options ( SignalArgs& args ) const
{
  Map & options = args.map( Protocol::Tags::key_properties() ).main_map;
  SignalOptions::add_to_map( options, *m_options );
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signal_list_signals( SignalArgs& args ) const
{
  SignalHandler::storage_t::const_iterator it = m_signals.begin();

  XmlNode value_node = args.main_map.content.add_node( Protocol::Tags::node_value() );

  value_node.set_attribute( Protocol::Tags::attr_key(), Protocol::Tags::key_signals() );

  for( ; it != m_signals.end(); ++it )
  {
    XmlNode signal_node = value_node.add_node( Protocol::Tags::node_map() );

    signal_node.set_attribute( Protocol::Tags::attr_key(), (*it)->name() );
    signal_node.set_attribute( Protocol::Tags::attr_descr(), (*it)->description() );
    signal_node.set_attribute( "name", (*it)->pretty_name() );
    signal_node.set_attribute( "hidden", to_str( (*it)->is_hidden() ) );
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signal_configure ( SignalArgs& args )
{
  using namespace rapidxml;

 if ( !args.has_map( Protocol::Tags::key_options() ) )
   throw  common::XmlError( FromHere(), "ConfigObject received  XML without a \'" + std::string(Protocol::Tags::key_options()) + "\' map" );

 XmlNode opt_map = args.map( Protocol::Tags::key_options() ).main_map.content;

 // get the list of options
 OptionList::OptionStorage_t& options = m_options->store;

 // loop on the param nodes
 for (xml_node<>* itr =  opt_map.content->first_node(); itr; itr = itr->next_sibling() )
 {
   // search for the attribute with key
   xml_attribute<>* att = itr->first_attribute( Protocol::Tags::attr_key() );
   if ( is_not_null(att) )
   {
     OptionList::iterator opt = options.find( att->value() );
     if (opt != options.end() )
     {
       XmlNode node(itr);
       opt->second->configure_option( node );
     }
     else
     {
       CFdebug << "Option " << att->value() << " not found in " << uri().string() << CFendl;
     }
   }
 }

 // add a reply frame
 SignalFrame reply = args.create_reply( uri() );
 Map map_node = reply.map( Protocol::Tags::key_options() ).main_map;

 opt_map.deep_copy( map_node.content );
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signal_rename_component ( SignalArgs& args )
{
  SignalOptions options( args );

  std::string new_name = options.value<std::string>("name");

  rename(new_name);
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signal_save_tree ( SignalArgs& args )
{
  SignalOptions options( args );

  const URI filename = options.value<URI>("filename");

  if(filename.scheme() != URI::Scheme::FILE)
    throw InvalidURI(FromHere(), "A file was expected but got \'" + filename.string() + "\'");

  boost::shared_ptr<XmlDoc> xmldoc = Protocol::create_doc();
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
  signal_list_options(reply);
  signal_list_signals(reply);
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signal_signature( SignalArgs & args )
{
  SignalFrame reply = args.create_reply( uri() );
  SignalOptions options( args );

  try
  {
    // execute the registered signal (if any) for this signature
    ( * signal( options["name"].value<std::string>() )->signature() )( reply );
  }
  catch(Exception & e)
  {
    // an exception occured, we remove the reply node...
    reply.node.content->parent()->remove_node(reply.node.content);
    throw; // ... and we forward the exception to an upper level
  }

}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::raise_tree_updated_event ()
{
  SignalFrame frame ( "tree_updated", uri(), uri() );
  EventHandler::instance().raise_event("tree_updated", frame ); // no error if event doesn't exist
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

  options.add_option("name", std::string("untitled") )
      .description("Name for created component.");
  options.add_option("type", std::string("cf3.common.Group") )
      .description("Concrete type of the component.");
  options.add_option("basic_mode", true )
      .description("Component will be visible in basic mode.");
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signature_rename_component( SignalArgs& args )
{
  SignalOptions options( args );

  options.add_option("name", std::string() )
      .description("Component new name.");
}

////////////////////////////////////////////////////////////////////////////////////////////

void Component::signature_move_component( SignalArgs& args )
{
  SignalOptions options( args );

  options.add_option<URI>("path")
      .pretty_name("Path")
      .description("Path to the new component to which this one will move to.");
}

////////////////////////////////////////////////////////////////////////////////

void Component::configure_option_recursively(const std::string& opt_name, const boost::any& val)
{

//  CFinfo << "+++ recurse config option [" << opt_name << "] from [" << uri().string() << "]" << CFendl;

  if (m_options->check(opt_name) && !(*m_options)[opt_name].has_tag("norecurse"))
  {
    options().configure_option(opt_name,val);
  }

  foreach_container((std::string name) (boost::shared_ptr<Option> opt), options())
  {
    if (opt->has_tag(opt_name) && !opt->has_tag("norecurse"))
      options().configure_option(name,val);
  }

  // configure all child's options recursively

  boost_foreach( Component& component, find_components_recursively(*this) )
  {

//    CFinfo << "        in [" << component.name() << "]" << CFendl;

    // configure the option that matches the name

    if (component.options().check(opt_name) && !component.options().option(opt_name).has_tag("norecurse"))
    {
      component.options().configure_option(opt_name,val);
    }

    // configure the options that matches the tags

    foreach_container((std::string name) (boost::shared_ptr<Option> opt), component.options())
    {
      if (opt->has_tag(opt_name) && !opt->has_tag("norecurse"))
        component.options().configure_option(name,val);
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
      if ( !options().check(name) ) // not found
        throw ValueNotFound(FromHere(), "Option with name [" + name +
                            "] not found in "+ uri().path());

      if      (type == "bool")
        options().configure_option(name,from_str<bool>(value));
      else if (type == "unsigned")
        options().configure_option(name,from_str<Uint>(value));
      else if (type == "integer")
        options().configure_option(name,from_str<int>(value));
      else if (type == "real")
        options().configure_option(name,from_str<Real>(value));
      else if (type == "string")
        options().configure_option(name,value);
      else if (type == "uri")
        options().configure_option(name,from_str<URI>(value));
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
          options().configure_option(name,vec);
        }
        else if (subtype == "unsigned")
        {
          std::vector<Uint> vec; vec.reserve(array.size());
          boost_foreach(const std::string& str_val,array)
            vec.push_back(from_str<Uint>(str_val));
          options().configure_option(name,vec);
        }
        else if (subtype == "integer")
        {
          std::vector<int> vec; vec.reserve(array.size());
          boost_foreach(const std::string& str_val,array)
            vec.push_back(from_str<int>(str_val));
          options().configure_option(name,vec);
        }
        else if (subtype == "real")
        {
          std::vector<Real> vec; vec.reserve(array.size());
          boost_foreach(const std::string& str_val,array)
            vec.push_back(from_str<Real>(str_val));
          options().configure_option(name,vec);
        }
        else if (subtype == "string")
        {
          options().configure_option(name,array);
        }
        else if (subtype == "uri")
        {
          std::vector<URI> vec; vec.reserve(array.size());
          boost_foreach(const std::string& str_val,array)
            vec.push_back(from_str<URI>(str_val));
          options().configure_option(name,vec);
        }

      }
      else
        throw ParsingFailed(FromHere(), "The type [" + type + "] of passed argument ["
          + arg + "] for ["+ uri().path() +"] is invalid.\n"+
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

boost::shared_ptr<Component> build_component(const std::string& builder_name,
                               const std::string& name,
                               const std::string& factory_type_name )
{
  // get the factories

  Handle<Component> factories = Core::instance().root().get_child("Factories");
  if ( is_null(factories) )
    throw ValueNotFound( FromHere(), "Factories \'Factories\' not found in "
                        + Core::instance().root().uri().string() );

  // get the factory holding the builder
  Handle<Component> factory = factories->get_child( factory_type_name );


  if ( is_null( factory ) || is_null( factory->get_child( builder_name ) ) )
  {
    if(is_null(Core::instance().libraries().autoload_library_with_builder( builder_name )))
      throw ValueNotFound(FromHere(), "Library for builder " + builder_name + " could not be autoloaded");
  }

  factory = factories->get_child( factory_type_name );

  if ( is_null(factory) )
    throw ValueNotFound( FromHere(),
                        "Factory \'" + factory_type_name
                        + "\' not found in " + factories->uri().string() + "." );

  // get the builder
  Handle<Builder> builder(factory->get_child( builder_name ));
  if ( is_null(builder) )
  {
    std::string msg =
        "Builder \'" + builder_name + "\' not found in factory \'"
        + factory_type_name + "\'. Probably forgot to load a library.\n"
        + "Possible builders:";
    boost_foreach(Component& comp, *factory)
        msg += "\n  -  "+comp.name();

    throw ValueNotFound( FromHere(), msg );
  }

  // build the component

  boost::shared_ptr<Component> comp = builder->build ( name );
  if ( is_null(comp) )
    throw NotEnoughMemory ( FromHere(),
                           "Builder \'" + builder_name
                           + "\' failed to allocate component with name \'" + name + "\'" );


  return comp;
}

////////////////////////////////////////////////////////////////////////////////////////////

boost::shared_ptr<Component> build_component_reduced(const std::string& builder_name,
                                       const std::string& name,
                                       const std::string& factory_type_name )
{
  // get the factories

  Handle<Component> factories = Core::instance().root().get_child("Factories");
  if ( is_null(factories) )
    throw ValueNotFound( FromHere(), "Factories \'Factories\' not found in "
                        + Core::instance().root().uri().string() );

  // get the factory holding the builder

  Handle<Factory> factory(factories->get_child( factory_type_name ));
  if ( is_null(factory) )
    throw ValueNotFound( FromHere(),
                        "Factory \'" + factory_type_name
                        + "\' not found in " + factories->uri().string() + "." );

  // get the builder
  Builder& cbuilder = factory->find_builder_with_reduced_name(builder_name);

  // build the component

  boost::shared_ptr<Component> comp = cbuilder.build ( name );
  if ( is_null(comp) )
    throw NotEnoughMemory ( FromHere(),
                           "Builder \'" + builder_name
                           + "\' failed to allocate component with name \'" + name + "\'" );


  return comp;
}

////////////////////////////////////////////////////////////////////////////////////////////

boost::shared_ptr<Component> build_component(const std::string& builder_name,
                               const std::string& name )
{
  std::string libnamespace = Builder::extract_namespace(builder_name);

  URI builder_path = Core::instance().libraries().uri()
                   / URI(libnamespace)
                   / URI(builder_name);



  Handle<Builder> cbuilder( follow_link(Core::instance().root().access_component( builder_path )) );

  if( is_null(cbuilder) ) // try to load the library that contains the builder
  {
    if(is_null(Core::instance().libraries().autoload_library_with_builder( builder_name )))
      throw ValueNotFound(FromHere(), "Library for builder " + builder_name + " could not be autoloaded");

    cbuilder = Handle<Builder>(follow_link(Core::instance().root().access_component( builder_path )));
  }

  if( is_null(cbuilder) ) // if still fails, then give up
    throw ValueNotFound( FromHere(), "Could not find builder \'" + builder_name + "\'"
                                     " neither a plugin library that contains it." );

  boost::shared_ptr<Component> comp = cbuilder->build( name );

  return comp;
}

boost::shared_ptr< Component > build_component_nothrow(const std::string& builder_name, const std::string& name)
{
  std::string libnamespace = Builder::extract_namespace(builder_name);

  URI builder_path = Core::instance().libraries().uri()
                   / URI(libnamespace)
                   / URI(builder_name);

  Handle<Builder> cbuilder( follow_link(Core::instance().root().access_component( builder_path )) );

  if( is_null(cbuilder) ) // try to load the library that contains the builder
  {
    if(is_null(Core::instance().libraries().autoload_library_with_builder( builder_name )))
      return boost::shared_ptr<Component>();

    cbuilder = Handle<Builder>(follow_link(Core::instance().root().access_component( builder_path )));
  }

  if( is_null(cbuilder) ) // if still fails, then give up
    return boost::shared_ptr<Component>();

  boost::shared_ptr<Component> comp = cbuilder->build( name );

  return comp;
}


////////////////////////////////////////////////////////////////////////////////////////////

void Component::on_ping_event(SignalArgs& args)
{
  CFdebug << "Ping response: " << uri().path() << " of type " << derived_type_name() << CFendl;
}

////////////////////////////////////////////////////////////////////////////////////////////

Component::iterator Component::begin()
{
  std::vector<boost::shared_ptr<Component> > vec;
  put_components<Component>(vec, false); // not recursive
  return Component::iterator(vec, 0);    // begin
}

////////////////////////////////////////////////////////////////////////////////////////////

Component::iterator Component::end()
{
  std::vector<boost::shared_ptr<Component> > vec;
  put_components<Component>(vec, false);        // not recursive
  return Component::iterator(vec, vec.size());  // end
}

////////////////////////////////////////////////////////////////////////////////////////////

Component::const_iterator Component::begin() const
{
  std::vector<boost::shared_ptr<Component const> > vec;
  put_components<Component>(vec, false); // not recursive
  return Component::const_iterator(vec, 0);    // begin
}

////////////////////////////////////////////////////////////////////////////////////////////

Component::const_iterator Component::end() const
{
  std::vector<boost::shared_ptr<Component const> > vec;
  put_components<Component>(vec, false);        // not recursive
  return Component::const_iterator(vec, vec.size());  // end
}

////////////////////////////////////////////////////////////////////////////////////////////

Component::iterator Component::recursive_begin()
{
  std::vector<boost::shared_ptr<Component> > vec;
  put_components<Component>(vec, true);  // recursive
  return Component::iterator(vec, 0);    // begin
}

////////////////////////////////////////////////////////////////////////////////////////////

Component::iterator Component::recursive_end()
{
  std::vector<boost::shared_ptr<Component> > vec;
  put_components<Component>(vec, true);         // recursive
  return Component::iterator(vec, vec.size());  // end
}

////////////////////////////////////////////////////////////////////////////////////////////

Component::const_iterator Component::recursive_begin() const
{
  std::vector<boost::shared_ptr<Component const> > vec;
  put_components<Component>(vec, true);  // recursive
  return Component::const_iterator(vec, 0);    // begin
}

////////////////////////////////////////////////////////////////////////////////////////////

Component::const_iterator Component::recursive_end() const
{
  std::vector<boost::shared_ptr<Component const> > vec;
  put_components<Component>(vec, true);         // recursive
  return Component::const_iterator(vec, vec.size());  // end
}

////////////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
