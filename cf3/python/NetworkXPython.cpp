// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/**
 * @file NewtorkXPython.cpp Implementation of signals and printing functions for creating graph of the component system
 * @author Tamas Banyai
**/

#include "python/BoostPython.hpp"
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

  regist_signal( "get_tag_graph" )
      .connect  ( boost::bind( &NetworkXPython::signal_get_tag_graph, this,  _1 ))
      .signature( boost::bind( &NetworkXPython::signature_get_tag_graph, this, _1))
      .description("Outputs the add_node and add_edge commands in order to build the graph if tags for NetworkX as a string")
      .pretty_name("GetTagGraph");
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

  // tags
  coll += "-----TAGS---------------\n";
  BOOST_FOREACH(const std::string &t, c->get_tags())
    coll += "     " + t + "\n";

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
  reply_options.add("return_value", coll);
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signature_get_detailed_info( common::SignalArgs& args )
{
  SignalOptions options( args );
  options.add( "uri", URI("//") )
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
  reply_options.add("return_value", coll);
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signature_get_component_graph( SignalArgs& args )
{
  SignalOptions options( args );
  options.add( "uri", URI("//") )
    .description("URI of the component to start from")
    .pretty_name("uri");
  options.add( "depth", 1000 )
    .description("Level up to look into the subtree of the component")
    .pretty_name("Depth");
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_component_nodes_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth)
{
  coll.append("G.add_node('c:" + c.uri().path() + "',depth=" + boost::lexical_cast<std::string>(depth) + ",tag='component',hidden=" + pybool(!c.has_tag("basic")) + ")\n");
  coll.append("nodecaption.update({'c:" + c.uri().path() + "':'" + c.name() + "'})\n");
  // slipping in master parent under tag='parent'
  if (depth==0)
    if (c.parent()!=nullptr)
    {
      Handle<Component> p=c.parent();
      coll.append("G.add_node('c:" + p->uri().path() + "',depth=" + boost::lexical_cast<std::string>(depth-1) + ",tag='parent',hidden=" + pybool(!p->has_tag("basic")) + ")\n");
      coll.append("nodecaption.update({'c:" + p->uri().path() + "':'" + p->name() + "'})\n");
    }
  if (depth<depthlimit) BOOST_FOREACH(const Component& subc, c )
    append_component_nodes_recursive(subc, coll, depthlimit, depth+1);
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_component_edges_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth)
{
  if (depth==0)
    if (c.parent()!=nullptr)
    {
      Handle<Component> p=c.parent();
      coll.append("G.add_edge('c:" + p->uri().path() + "','c:" + c.uri().path() + "',depth=" + boost::lexical_cast<std::string>(depth) + ",tag='parent',hidden=" + pybool(!p->has_tag("basic")) + ")\n");
    }
  if (depth<depthlimit) BOOST_FOREACH(const Component& subc, c )
  {
    coll.append("G.add_edge('c:" + c.uri().path() + "','c:" + subc.uri().path() + "',depth=" + boost::lexical_cast<std::string>(depth) + ",tag='component',hidden=" + pybool(!c.has_tag("basic")) + ")\n");
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
  reply_options.add("return_value", coll);
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signature_get_option_graph( SignalArgs& args )
{
  SignalOptions options( args );
  options.add( "uri", URI("//") )
    .description("URI of the component to start from")
    .pretty_name("uri");
  options.add( "depth", 1000 )
    .description("Level up to look into the subtree of the component")
    .pretty_name("Depth");
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_option_nodes_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth)
{
  BOOST_FOREACH(const OptionList::OptionStorage_t::value_type &ot, c.options())
  {
    Option &o = *ot.second;
    coll.append("G.add_node('o:" + c.uri().path() + "/" + o.name() + "',depth=" + boost::lexical_cast<std::string>(depth+1) + ",tag='option',hidden=" + pybool(!o.has_tag("basic")) + ")\n");
    coll.append("nodecaption.update({'o:" + c.uri().path() + "/" + o.name() + "':'" + o.name() + "'})\n");
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
    coll.append("G.add_edge('c:" + c.uri().path() + "','o:" + c.uri().path() + "/" + o.name() + "',depth=" + boost::lexical_cast<std::string>(depth+1) + ",tag='option',hidden=" + pybool(!o.has_tag("basic")) + ")\n");
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
  reply_options.add("return_value", coll);
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signature_get_signal_graph( SignalArgs& args )
{
  SignalOptions options( args );
  options.add( "uri", URI("//") )
    .description("URI of the component to start from")
    .pretty_name("uri");
  options.add( "depth", 1000 )
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
    coll.append("G.add_node('s:" + c.uri().path() + "/" + s->name() + "',depth=" + boost::lexical_cast<std::string>(depth+1) + ",tag='signal',hidden=" + pybool(s->is_hidden()) + ")\n");
    coll.append("nodecaption.update({'s:" + c.uri().path() + "/" + s->name() + "':'" + s->name() + "'})\n");
  }
  if (depth<depthlimit) BOOST_FOREACH(const Component& subc, c )
    append_signal_nodes_recursive(subc, coll, depthlimit, depth+1);
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_signal_edges_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth)
{
  BOOST_FOREACH(const common::SignalPtr s, c.signal_list())
    coll.append("G.add_edge('c:" + c.uri().path() + "','s:" + c.uri().path() + "/" + s->name() + "',depth=" + boost::lexical_cast<std::string>(depth+1) + ",tag='signal',hidden=" + pybool(s->is_hidden()) + ")\n");
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
  reply_options.add("return_value", coll);
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signature_get_field_graph( SignalArgs& args )
{
  SignalOptions options( args );
  options.add( "uri", URI("//") )
    .description("URI of the component to start from")
    .pretty_name("uri");
  options.add( "depth", 1000 )
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
      coll.append("G.add_node('f:" + n + "',depth=" + boost::lexical_cast<std::string>(depth+1) + ",tag='field',hidden=False)\n");
      coll.append("nodecaption.update({'f:" + n + "':'" + f.var_name(i) + "_" + boost::lexical_cast<std::string>(f.size()) + "'})\n");
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
      coll.append("G.add_edge('c:" + f.uri().path() + "','f:" + n + "',depth=" + boost::lexical_cast<std::string>(depth+1) + ",tag='field',hidden=False)\n");
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
  reply_options.add("return_value", coll);
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signature_get_link_graph( SignalArgs& args )
{
  SignalOptions options( args );
  options.add( "uri", URI("//") )
    .description("URI of the component to start from")
    .pretty_name("uri");
  options.add( "depth", 1000 )
    .description("Level up to look into the subtree of the component")
    .pretty_name("Depth");
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_link_nodes_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth, std::string printroot)
{
  // has to address by c: because l: and c: can be mixed, the edge does not know if target node is in scope with c: or out of scope with l:
  // this is tricky, there are two reasons why a node needs to be added
  // 1. target component is outside the subtree
  // 2. target component is behind the depthlimit
  if (IsComponentType<common::Link>()(c))
  {
    const common::Link& l=dynamic_cast<const common::Link&>(c);
    std::string t=l.follow()->uri().path();
    if (t.find(printroot)==t.npos)
    {
      coll.append("G.add_node('c:" + t + "',depth=" + boost::lexical_cast<std::string>(depth+1) + ",tag='link',hidden=" + pybool(!l.has_tag("basic")) + ")\n");
      coll.append("nodecaption.update({'c:" + t + "':'" + t + "'})\n");
    }
    else
    {
      Handle<const Component> d=l.follow();
      for (int i=1; i>-1; ++i)
      {
        d=d->parent();
        if (d==nullptr) break;
        if ((d->uri().path()==printroot)&&(i>depthlimit))
        {
          coll.append("G.add_node('c:" + t + "',depth=" + boost::lexical_cast<std::string>(depth+1) + ",tag='link',hidden=" + pybool(!l.has_tag("basic")) + ")\n");
          coll.append("nodecaption.update({'c:" + t + "':'" + t + "'})\n");
          break;
        }
      }
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
    coll.append("G.add_edge('c:" + c.uri().path() + "','c:" + l.follow()->uri().path() + "',depth=" + boost::lexical_cast<std::string>(depth) + ",tag='link',hidden=" + pybool(!l.has_tag("basic")) + ")\n");
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
  const int  depthlimit = options.option("depth").value<const int>();
  std::string coll("");
  append_property_nodes_recursive(*printroot,coll,depthlimit,0);
  append_property_edges_recursive(*printroot,coll,depthlimit,0);
  SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("return_value", coll);
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signature_get_property_graph( SignalArgs& args )
{
  SignalOptions options( args );
  options.add( "uri", URI("//") )
    .description("URI of the component to start from")
    .pretty_name("uri");
  options.add( "depth", 1000 )
    .description("Level up to look into the subtree of the component")
    .pretty_name("Depth");
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_property_nodes_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth)
{
  BOOST_FOREACH(const PropertyList::PropertyStorage_t::value_type &pt, c.properties())
  {
    std::string name = pt.first;
    coll.append("G.add_node('p:" + c.uri().path() + "/" + name + "',depth=" + boost::lexical_cast<std::string>(depth+1) + ",tag='property',hidden=" + pybool(!c.has_tag("basic")) + ")\n");
    coll.append("nodecaption.update({'p:" + c.uri().path() + "/" + name + "':'" + name + "'})\n");
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
    coll.append("G.add_edge('c:" + c.uri().path() + "','p:" + c.uri().path() + "/" + name + "',depth=" + boost::lexical_cast<std::string>(depth+1) + ",tag='property',hidden=" + pybool(!c.has_tag("basic")) + ")\n");
  }
  if (depth<depthlimit) BOOST_FOREACH(const Component& subc, c )
    append_property_edges_recursive(subc, coll, depthlimit, depth+1);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signal_get_tag_graph( SignalArgs& args )
{
  SignalOptions options( args );
  Handle<Component> printroot = access_component_checked(options.option("uri").value<URI>());
  const int depthlimit = options.option("depth").value<const int>();
  std::string coll("");
  append_tag_nodes_recursive(*printroot,coll,depthlimit,0);
  append_tag_edges_recursive(*printroot,coll,depthlimit,0);
  SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("return_value", coll);
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signature_get_tag_graph( SignalArgs& args )
{
  SignalOptions options( args );
  options.add( "uri", URI("//") )
    .description("URI of the component to start from")
    .pretty_name("uri");
  options.add( "depth", 1000 )
    .description("Level up to look into the subtree of the component")
    .pretty_name("Depth");
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_tag_nodes_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth)
{
  BOOST_FOREACH(const std::string &t, c.get_tags())
  {
    coll.append("G.add_node('t:" + c.uri().path() + "/" + t + "',depth=" + boost::lexical_cast<std::string>(depth+1) + ",tag='tag',hidden=" + pybool(!c.has_tag("basic")) + ")\n");
    coll.append("nodecaption.update({'t:" + c.uri().path() + "/" + t + "':'" + t + "'})\n");
  }
  if (depth<depthlimit) BOOST_FOREACH(const Component& subc, c )
    append_tag_nodes_recursive(subc, coll, depthlimit, depth+1);
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_tag_edges_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth)
{
  BOOST_FOREACH(const std::string &t, c.get_tags())
    coll.append("G.add_edge('c:" + c.uri().path() + "','t:" + c.uri().path() + "/" + t + "',depth=" + boost::lexical_cast<std::string>(depth+1) + ",tag='tag',hidden=" + pybool(!c.has_tag("basic")) + ")\n");
  if (depth<depthlimit) BOOST_FOREACH(const Component& subc, c )
    append_tag_edges_recursive(subc, coll, depthlimit, depth+1);
}

////////////////////////////////////////////////////////////////////////////////

} // namespace python
} // namespace cf3
