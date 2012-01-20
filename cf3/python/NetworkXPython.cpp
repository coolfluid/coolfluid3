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
  regist_signal( "get_component_graph" )
      .connect  ( boost::bind( &NetworkXPython::signal_get_component_graph, this,  _1 ))
      .signature( boost::bind( &NetworkXPython::signature_get_component_graph, this, _1))
      .description("Outputs the add_node and add_edge commands in order to build the graph if components in NetworkX as a string")
      .pretty_name("GetComponentGraph");

  regist_signal( "get_option_graph" )
      .connect  ( boost::bind( &NetworkXPython::signal_get_option_graph, this,  _1 ))
      .signature( boost::bind( &NetworkXPython::signature_get_option_graph, this, _1))
      .description("Outputs the add_node and add_edge commands in order to build the graph if options in NetworkX as a string")
      .pretty_name("GetComponentGraph");

  regist_signal( "get_signal_graph" )
      .connect  ( boost::bind( &NetworkXPython::signal_get_signal_graph, this,  _1 ))
      .signature( boost::bind( &NetworkXPython::signature_get_signal_graph, this, _1))
      .description("Outputs the add_node and add_edge commands in order to build the graph if signals in NetworkX as a string")
      .pretty_name("GetSignalGraph");

  regist_signal( "get_field_graph" )
      .connect  ( boost::bind( &NetworkXPython::signal_get_field_graph, this,  _1 ))
      .signature( boost::bind( &NetworkXPython::signature_get_field_graph, this, _1))
      .description("Outputs the add_node and add_edge commands in order to build the graph if fields in NetworkX as a string")
      .pretty_name("GetFieldGraph");

  regist_signal( "get_link_graph" )
      .connect  ( boost::bind( &NetworkXPython::signal_get_link_graph, this,  _1 ))
      .signature( boost::bind( &NetworkXPython::signature_get_link_graph, this, _1))
      .description("Outputs the add_node and add_edge commands in order to build the graph if links in NetworkX as a string")
      .pretty_name("GetLinkGraph");
}

////////////////////////////////////////////////////////////////////////////////

NetworkXPython::~NetworkXPython()
{
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signal_get_component_graph(SignalArgs& args)
{
  SignalOptions options( args );
  Handle<Component> printroot = access_component_checked(options.option("uri").value<URI>());
  std::string coll("");
  append_component_nodes_recursive(*printroot,coll,0);
  append_component_edges_recursive(*printroot,coll,0);
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
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_component_nodes_recursive(const Component &c, std::string &coll, int depth)
{
  coll.append("G.add_node('" + c.uri().path() + "',depth=" + boost::lexical_cast<std::string>(depth) + ",tag='component')\n");
  coll.append("nodecaption.update({'" + c.uri().path() + "':'" + c.name() + "'})\n");
  coll.append("nodenote.update({'" + c.uri().path() + "':'" + c.derived_type_name() + "'})\n");
  BOOST_FOREACH(const Component& subc, c )
    append_component_nodes_recursive(subc, coll, depth+1);
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_component_edges_recursive(const Component &c, std::string &coll, int depth)
{
  BOOST_FOREACH(const Component& subc, c )
  {
    coll.append("G.add_edge('" + c.uri().path() + "','" + subc.uri().path() + "',depth=" + boost::lexical_cast<std::string>(depth) + ",tag='component')\n");
    append_component_edges_recursive(subc, coll, depth+1);
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signal_get_option_graph(SignalArgs& args)
{
  SignalOptions options( args );
  Handle<Component> printroot = access_component_checked(options.option("uri").value<URI>());
  std::string coll("");
  append_option_nodes_recursive(*printroot,coll,0);
  append_option_edges_recursive(*printroot,coll,0);
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
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_option_nodes_recursive(const Component &c, std::string &coll, int depth)
{
//  coll.append("G.add_node('" + c.uri().path() + "',depth=" + boost::lexical_cast<std::string>(depth) + ",tag='component')\n");
//  coll.append("nodecaption.update({'" + c.uri().path() + "':'" + c.name() + "'})\n");
//  coll.append("nodenote.update({'" + c.uri().path() + "':'" + c.derived_type_name() + "'})\n");

  BOOST_FOREACH(const OptionList::OptionStorage_t::value_type &ot, c.options())
  {
    Option &o = *ot.second;
    //CFinfo << o.name() << "=" << o.value_str() << CFendl;
    if (o.has_restricted_list())
    {
      std::string rest_list="";
      std::vector<boost::any>::iterator ir = o.restricted_list().begin();
      for( ; ir != o.restricted_list().end() ; ++ir )
        rest_list= rest_list + " " + boost::any_cast<std::string>(*ir);
      CFinfo << rest_list << CFendl;
    }
  }

//  std::string opts = c.options().list_options();
//  if (opts!="")
//  {
//    boost::tokenizer< boost::char_separator<char> > tok(opts,boost::char_separator<char>("\n"));
//    for(boost::tokenizer< boost::char_separator<char> >::iterator opt=tok.begin(); opt!=tok.end();++opt){
//      CFinfo << c.uri().path() << "/" << *opt << CFendl;
//    }
//  }

  BOOST_FOREACH(const Component& subc, c )
    append_option_nodes_recursive(subc, coll, depth+1);

}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_option_edges_recursive(const Component &c, std::string &coll, int depth)
{
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signal_get_signal_graph(SignalArgs& args)
{
  SignalOptions options( args );
  Handle<Component> printroot = access_component_checked(options.option("uri").value<URI>());
  std::string coll("");
  append_signal_nodes_recursive(*printroot,coll,0);
  append_signal_edges_recursive(*printroot,coll,0);
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
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_signal_nodes_recursive(const Component &c, std::string &coll, int depth)
{
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_signal_edges_recursive(const Component &c, std::string &coll, int depth)
{
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signal_get_field_graph(SignalArgs& args)
{
  SignalOptions options( args );
  Handle<Component> printroot = access_component_checked(options.option("uri").value<URI>());
  std::string coll("");
  append_field_nodes_recursive(*printroot,coll,0);
  append_field_edges_recursive(*printroot,coll,0);
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
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_field_nodes_recursive(const Component &c, std::string &coll, int depth)
{
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_field_edges_recursive(const Component &c, std::string &coll, int depth)
{
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signal_get_link_graph(SignalArgs& args)
{
  SignalOptions options( args );
  Handle<Component> printroot = access_component_checked(options.option("uri").value<URI>());
  std::string coll("");
  append_link_nodes_recursive(*printroot,coll,0);
  append_link_edges_recursive(*printroot,coll,0);
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
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_link_nodes_recursive(const Component &c, std::string &coll, int depth)
{
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_link_edges_recursive(const Component &c, std::string &coll, int depth)
{
}

////////////////////////////////////////////////////////////////////////////////

} // namespace python
} // namespace cf3
