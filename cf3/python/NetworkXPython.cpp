// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/**
 * @file NewtorkXPython.cpp Implementation of signals and printing functions for creating graph of the component system
 * @author Tamas Banyai
**/

#include "boost/python.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/tokenizer.hpp"

#include "common/BoostAnyConversion.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/Signal.hpp"
#include "common/SignalHandler.hpp"
#include "common/Builder.hpp"
#include "common/URI.hpp"
#include "common/FindComponents.hpp"
#include "common/ComponentIterator.hpp"
#include "common/PropertyList.hpp"
#include "common/XML/FileOperations.hpp"

#include "mesh/Field.hpp"

#include "python/LibPython.hpp"
#include "python/NetworkXPython.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;

namespace cf3 {
namespace python {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < NetworkXPython, Component, LibPython > NetworkXPython_Builder;

////////////////////////////////////////////////////////////////////////////////

NetworkXPython::NetworkXPython( const std::string& name ) :
  Component(name)
{
  regist_signal( "get_detailed_info" )
      .connect  ( boost::bind( &NetworkXPython::signal_get_detailed_info, this,  _1 ))
      .signature( boost::bind( &NetworkXPython::signature_get_detailed_info, this, _1))
      .description("Gathers info on component specified by option 'uri' and returns in string.")
      .pretty_name("GetDetailedInfo");

  regist_signal( "get_component_graph" )
      .connect  ( boost::bind( &NetworkXPython::signal_get_component_graph, this,  _1 ))
      .signature( boost::bind( &NetworkXPython::signature_get_component_graph, this, _1))
      .description("Outputs the add_node and add_edge commands in order to build the graph if components for NetworkX as a string")
      .pretty_name("GetComponentGraph");

  regist_signal( "get_option_graph" )
      .connect  ( boost::bind( &NetworkXPython::signal_get_option_graph, this,  _1 ))
      .signature( boost::bind( &NetworkXPython::signature_get_option_graph, this, _1))
      .description("Outputs the add_node and add_edge commands in order to build the graph if options for NetworkX as a string")
      .pretty_name("GetComponentGraph");

  regist_signal( "get_signal_graph" )
      .connect  ( boost::bind( &NetworkXPython::signal_get_signal_graph, this,  _1 ))
      .signature( boost::bind( &NetworkXPython::signature_get_signal_graph, this, _1))
      .description("Outputs the add_node and add_edge commands in order to build the graph if signals for NetworkX as a string")
      .pretty_name("GetSignalGraph");

  regist_signal( "get_field_graph" )
      .connect  ( boost::bind( &NetworkXPython::signal_get_field_graph, this,  _1 ))
      .signature( boost::bind( &NetworkXPython::signature_get_field_graph, this, _1))
      .description("Outputs the add_node and add_edge commands in order to build the graph if fields for NetworkX as a string")
      .pretty_name("GetFieldGraph");

  regist_signal( "get_link_graph" )
      .connect  ( boost::bind( &NetworkXPython::signal_get_link_graph, this,  _1 ))
      .signature( boost::bind( &NetworkXPython::signature_get_link_graph, this, _1))
      .description("Outputs the add_node and add_edge commands in order to build the graph if links for NetworkX as a string")
      .pretty_name("GetLinkGraph");

  regist_signal( "get_property_graph" )
      .connect  ( boost::bind( &NetworkXPython::signal_get_property_graph, this,  _1 ))
      .signature( boost::bind( &NetworkXPython::signature_get_property_graph, this, _1))
      .description("Outputs the add_node and add_edge commands in order to build the graph if propertys for NetworkX as a string")
      .pretty_name("GetPropertyGraph");
}

////////////////////////////////////////////////////////////////////////////////

NetworkXPython::~NetworkXPython()
{
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signal_get_detailed_info(SignalArgs& args)
{
  // get target component's uri
  SignalOptions options( args );
  Handle<Component> c = access_component_checked(options.option("uri").value<URI>());
  std::string coll("");

  // header
  coll += "-----NAME---------------\n     " + c->name() + "\n";
  coll += "-----TYPE---------------\n     " + c->derived_type_name() + "\n";
  coll += "-----PATH---------------\n     " + c->uri().path() + "\n";

  // options
  coll += "-----OPTIONS------------\n";
  BOOST_FOREACH(const OptionList::OptionStorage_t::value_type &ot, c->options())
  {
    common::Option &o = *ot.second;
    coll += "     " + o.name()+ " (" + o.type() + ")=" + o.value_str() + "\n";
    if (o.has_restricted_list())
    {
      coll += "          restricted to:";
      std::vector<boost::any>::iterator ir = o.restricted_list().begin();
      for( ; ir != o.restricted_list().end() ; ++ir )
        coll += " " + boost::any_cast<std::string>(*ir);
      coll += "\n";
    }
  }

  // properties
  coll += "-----PROPERTIES---------\n";
  BOOST_FOREACH(const PropertyList::PropertyStorage_t::value_type &pt, c->properties())
  {
    std::string name = pt.first;
    boost::any value = pt.second;
    std::string valuestr="<String Conversion Failed>";
    try {
      valuestr=boost::any_cast<std::string>(value);
    } catch(...) {}
    coll += "     " + name + " (" + value.type().name() + ")=" + valuestr + "\n";
  }

  // child components
  coll += "-----COMPONENTS---------\n";
  BOOST_FOREACH(const Component& subc, *c )
    coll += "     " + subc.name() + " (" + subc.derived_type_name() + ")\n";

  // signals
  coll += "-----SIGNALS------------\n";
  BOOST_FOREACH(const common::SignalPtr s, c->signal_list())
  {
    coll += "     " + s->name() + ": " + s->description() + "\n";
    common::SignalArgs node;
    ( * s->signature() ) ( node );
    common::XML::SignalOptions options(node);
    for(common::OptionList::iterator iopt = options.begin(); iopt != options.end(); ++iopt)
    {
      boost::shared_ptr<common::Option> o=iopt->second;
      coll += "          " + iopt->first + " (" + o->type() + ")=" + o->value_str() + ": " + o->description() + "\n";
      if (o->has_restricted_list())
      {
        coll += "               restricted to:";
        std::vector<boost::any>::iterator ir = o->restricted_list().begin();
        for( ; ir != o->restricted_list().end() ; ++ir )
          coll += " " + boost::any_cast<std::string>(*ir);
        coll += "\n";
      }
    }
  }

  // send coll to python
  SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add_option("return_value", coll);
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signature_get_detailed_info( common::SignalArgs& args )
{
  SignalOptions options( args );
  options.add_option( "uri", URI("//") )
    .description("URI of the component to start from")
    .pretty_name("uri");
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signal_get_component_graph(SignalArgs& args)
{
  SignalOptions options( args );
  Handle<Component> printroot = access_component_checked(options.option("uri").value<URI>());
  const int depthlimit = options.option("depth").value<const int>();
  std::string coll("");
  append_component_nodes_recursive(*printroot,coll,depthlimit,0);
  append_component_edges_recursive(*printroot,coll,depthlimit,0);
  SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add_option("return_value", coll);
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signature_get_component_graph( SignalArgs& args )
{
  SignalOptions options( args );
  options.add_option( "uri", URI("//") )
    .description("URI of the component to start from")
    .pretty_name("uri");
  options.add_option( "depth", 1000 )
    .description("Level up to look into the subtree of the component")
    .pretty_name("Depth");
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_component_nodes_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth)
{
  coll.append("G.add_node('" + c.uri().path() + "',depth=" + boost::lexical_cast<std::string>(depth) + ",tag='component')\n");
  coll.append("nodecaption.update({'" + c.uri().path() + "':'" + c.name() + "'})\n");
  if (depth<depthlimit) BOOST_FOREACH(const Component& subc, c )
    append_component_nodes_recursive(subc, coll, depthlimit, depth+1);
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_component_edges_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth)
{
  if (depth<depthlimit) BOOST_FOREACH(const Component& subc, c )
  {
    coll.append("G.add_edge('" + c.uri().path() + "','" + subc.uri().path() + "',depth=" + boost::lexical_cast<std::string>(depth) + ",tag='component')\n");
    append_component_edges_recursive(subc, coll, depthlimit, depth+1);
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signal_get_option_graph(SignalArgs& args)
{
  SignalOptions options( args );
  Handle<Component> printroot = access_component_checked(options.option("uri").value<URI>());
  const int depthlimit = options.option("depth").value<const int>();
  std::string coll("");
  append_option_nodes_recursive(*printroot,coll,depthlimit,0);
  append_option_edges_recursive(*printroot,coll,depthlimit,0);
  SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add_option("return_value", coll);
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signature_get_option_graph( SignalArgs& args )
{
  SignalOptions options( args );
  options.add_option( "uri", URI("//") )
    .description("URI of the component to start from")
    .pretty_name("uri");
  options.add_option( "depth", 1000 )
    .description("Level up to look into the subtree of the component")
    .pretty_name("Depth");
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_option_nodes_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth)
{
  BOOST_FOREACH(const OptionList::OptionStorage_t::value_type &ot, c.options())
  {
    Option &o = *ot.second;
    coll.append("G.add_node('" + c.uri().path() + "/" + o.name() + "',depth=" + boost::lexical_cast<std::string>(depth+1) + ",tag='option')\n");
    coll.append("nodecaption.update({'" + c.uri().path() + "/" + o.name() + "':'" + o.name() + "'})\n");
  }
  if (depth<depthlimit) BOOST_FOREACH(const Component& subc, c )
    append_option_nodes_recursive(subc, coll, depthlimit, depth+1);
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_option_edges_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth)
{
  BOOST_FOREACH(const OptionList::OptionStorage_t::value_type &ot, c.options())
  {
    Option &o = *ot.second;
    coll.append("G.add_edge('" + c.uri().path() + "','" + c.uri().path() + "/" + o.name() + "',depth=" + boost::lexical_cast<std::string>(depth+1) + ",tag='option')\n");
  }
  if (depth<depthlimit) BOOST_FOREACH(const Component& subc, c )
    append_option_edges_recursive(subc, coll, depthlimit, depth+1);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signal_get_signal_graph(SignalArgs& args)
{
  SignalOptions options( args );
  Handle<Component> printroot = access_component_checked(options.option("uri").value<URI>());
  const int depthlimit = options.option("depth").value<const int>();
  std::string coll("");
  append_signal_nodes_recursive(*printroot,coll,depthlimit,0);
  append_signal_edges_recursive(*printroot,coll,depthlimit,0);
  SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add_option("return_value", coll);
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signature_get_signal_graph( SignalArgs& args )
{
  SignalOptions options( args );
  options.add_option( "uri", URI("//") )
    .description("URI of the component to start from")
    .pretty_name("uri");
  options.add_option( "depth", 1000 )
    .description("Level up to look into the subtree of the component")
    .pretty_name("Depth");
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_signal_nodes_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth)
{
  BOOST_FOREACH(const common::SignalPtr s, c.signal_list())
  {
    common::SignalArgs node;
    ( * s->signature() ) ( node );
    common::XML::SignalOptions options(node);
    std::string doc_str("");
    for(common::OptionList::iterator option_it = options.begin(); option_it != options.end(); ++option_it)
      doc_str += option_it->first + " "; //+ ": " + option_it->second->description() ;// + "\n";
    coll.append("G.add_node('" + c.uri().path() + "/" + s->name() + "',depth=" + boost::lexical_cast<std::string>(depth+1) + ",tag='signal')\n");
    coll.append("nodecaption.update({'" + c.uri().path() + "/" + s->name() + "':'" + s->name() + "'})\n");
  }
  if (depth<depthlimit) BOOST_FOREACH(const Component& subc, c )
    append_signal_nodes_recursive(subc, coll, depthlimit, depth+1);
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_signal_edges_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth)
{
  BOOST_FOREACH(const common::SignalPtr s, c.signal_list())
    coll.append("G.add_edge('" + c.uri().path() + "','" + c.uri().path() + "/" + s->name() + "',depth=" + boost::lexical_cast<std::string>(depth+1) + ",tag='signal')\n");
  if (depth<depthlimit) BOOST_FOREACH(const Component& subc, c )
    append_signal_edges_recursive(subc, coll, depthlimit, depth+1);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signal_get_field_graph(SignalArgs& args)
{
  SignalOptions options( args );
  Handle<Component> printroot = access_component_checked(options.option("uri").value<URI>());
  const int depthlimit = options.option("depth").value<const int>();
  std::string coll("");
  append_field_nodes_recursive(*printroot,coll,depthlimit,0);
  append_field_edges_recursive(*printroot,coll,depthlimit,0);
  SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add_option("return_value", coll);
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signature_get_field_graph( SignalArgs& args )
{
  SignalOptions options( args );
  options.add_option( "uri", URI("//") )
    .description("URI of the component to start from")
    .pretty_name("uri");
  options.add_option( "depth", 1000 )
    .description("Level up to look into the subtree of the component")
    .pretty_name("Depth");
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_field_nodes_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth)
{
  if (IsComponentType<mesh::Field>()(c))
  {
    const mesh::Field& f=dynamic_cast<const mesh::Field&>(c);
    for ( int i=0; i<(const int)f.nb_vars(); i++){
      std::string n=f.uri().path() + "/" + boost::lexical_cast<std::string>(i) + f.var_name(i);
      coll.append("G.add_node('" + n + "',depth=" + boost::lexical_cast<std::string>(depth+1) + ",tag='field')\n");
      coll.append("nodecaption.update({'" + n + "':'" + f.name() + "/" + f.var_name(i) + "'})\n");
    }
  }
  if (depth<depthlimit) BOOST_FOREACH(const Component& subc, c )
    append_field_nodes_recursive(subc, coll, depthlimit, depth+1);
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_field_edges_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth)
{
  if (IsComponentType<mesh::Field>()(c))
  {
    const mesh::Field& f=dynamic_cast<const mesh::Field&>(c);
    for ( int i=0; i<(const int)f.nb_vars(); i++){
      std::string n=f.uri().path() + "/" + boost::lexical_cast<std::string>(i) + f.var_name(i);
      coll.append("G.add_edge('" + f.uri().path() + "','" + n + "',depth=" + boost::lexical_cast<std::string>(depth+1) + ",tag='field')\n");
    }
  }
  if (depth<depthlimit) BOOST_FOREACH(const Component& subc, c )
    append_field_edges_recursive(subc, coll, depthlimit, depth+1);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signal_get_link_graph(SignalArgs& args)
{
  SignalOptions options( args );
  Handle<Component> printroot = access_component_checked(options.option("uri").value<URI>());
  const int depthlimit = options.option("depth").value<const int>();
  std::string coll("");
  append_link_nodes_recursive(*printroot,coll,depthlimit,0,printroot->uri().path());
  append_link_edges_recursive(*printroot,coll,depthlimit,0);
  SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add_option("return_value", coll);
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signature_get_link_graph( SignalArgs& args )
{
  SignalOptions options( args );
  options.add_option( "uri", URI("//") )
    .description("URI of the component to start from")
    .pretty_name("uri");
  options.add_option( "depth", 1000 )
    .description("Level up to look into the subtree of the component")
    .pretty_name("Depth");
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_link_nodes_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth, std::string printroot)
{
  /// @todo since depth limitation is now possible, its possible that link points on actual branch below the depth limit -> needs to be cured
  if (IsComponentType<common::Link>()(c))
  {
    const common::Link& l=dynamic_cast<const common::Link&>(c);
    std::string t=l.follow()->uri().path();
    if (t.find(printroot)==t.npos) {
      coll.append("G.add_node('" + t + "',depth=" + boost::lexical_cast<std::string>(depth+1) + ",tag='link')\n");
      coll.append("nodecaption.update({'" + t + "':'" + t + "'})\n");
    }
  }
  if (depth<depthlimit) BOOST_FOREACH(const Component& subc, c )
    append_link_nodes_recursive(subc, coll, depthlimit, depth+1, printroot);
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_link_edges_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth)
{
  if (IsComponentType<common::Link>()(c))
  {
    const common::Link& l=dynamic_cast<const common::Link&>(c);
    coll.append("G.add_edge('" + c.uri().path() + "','" + l.follow()->uri().path() + "',depth=" + boost::lexical_cast<std::string>(depth) + ",tag='link')\n");
  }
  if (depth<depthlimit) BOOST_FOREACH(const Component& subc, c )
    append_link_edges_recursive(subc, coll, depthlimit, depth+1);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signal_get_property_graph(SignalArgs& args)
{
  SignalOptions options( args );
  Handle<Component> printroot = access_component_checked(options.option("uri").value<URI>());
  const int depthlimit = options.option("depth").value<const int>();
  std::string coll("");
  append_property_nodes_recursive(*printroot,coll,depthlimit,0);
  append_property_edges_recursive(*printroot,coll,depthlimit,0);
  SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add_option("return_value", coll);
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signature_get_property_graph( SignalArgs& args )
{
  SignalOptions options( args );
  options.add_option( "uri", URI("//") )
    .description("URI of the component to start from")
    .pretty_name("uri");
  options.add_option( "depth", 1000 )
    .description("Level up to look into the subtree of the component")
    .pretty_name("Depth");
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_property_nodes_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth)
{
  BOOST_FOREACH(const PropertyList::PropertyStorage_t::value_type &pt, c.properties())
  {
    std::string name = pt.first;
    coll.append("G.add_node('" + c.uri().path() + "/" + name + "',depth=" + boost::lexical_cast<std::string>(depth+1) + ",tag='property')\n");
    coll.append("nodecaption.update({'" + c.uri().path() + "/" + name + "':'" + name + "'})\n");
  }
  if (depth<depthlimit) BOOST_FOREACH(const Component& subc, c )
    append_property_nodes_recursive(subc, coll, depthlimit, depth+1);
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_property_edges_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth)
{
  BOOST_FOREACH(const PropertyList::PropertyStorage_t::value_type &pt, c.properties())
  {
    std::string name = pt.first;
    coll.append("G.add_edge('" + c.uri().path() + "','" + c.uri().path() + "/" + name + "',depth=" + boost::lexical_cast<std::string>(depth+1) + ",tag='property')\n");
  }
  if (depth<depthlimit) BOOST_FOREACH(const Component& subc, c )
    append_property_edges_recursive(subc, coll, depthlimit, depth+1);
}

////////////////////////////////////////////////////////////////////////////////

} // namespace python
} // namespace cf3
