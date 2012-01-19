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
  regist_signal( "print_component_graph" )
      .connect  ( boost::bind( &NetworkXPython::signal_print_component_graph, this,  _1 ))
      .signature( boost::bind( &NetworkXPython::signature_print_component_graph, this, _1))
      .description("Outputs the add_node and add_edge commands in order to build the graph in NetworkX")
      .pretty_name("Prints commands to buld NetworkX graph.");
}

NetworkXPython::~NetworkXPython()
{
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signal_print_component_graph( SignalArgs& args )
{
  SignalOptions options( args );
  Handle<Component> printroot = access_component_checked(options.option("uri").value<URI>());
  CFinfo << "uri().path()=        " << printroot->uri().path()        << CFendl
         << "derived_type_name()= " << printroot->derived_type_name() << CFendl
         << "name()=              " << printroot->name()              << CFendl
         << "uri().base_path()=   " << printroot->uri().base_path()   << CFendl
         << "uri().base_name()=   " << printroot->uri().base_name()   << CFendl
         << "uri().name()=        " << printroot->uri().name()        << CFendl
         << "uri().path()=        " << printroot->uri().path()        << CFendl
         << CFflush << CFendl;

  std::string coll;
  append_components_recursive(*printroot,coll,0);

}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::signature_print_component_graph( SignalArgs& args )
{
  SignalOptions options( args );

  options.add_option( "uri", URI("//") )
    .description("URI of the component to start from")
    .pretty_name("uri");
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::print_to_python_stdout(std::string what)
{
  PySys_WriteStdout(what.c_str());
}

////////////////////////////////////////////////////////////////////////////////

void NetworkXPython::append_components_recursive(const Component &c, std::string &coll, int depth)
{

/*
void Component::signal_list_tree_recursive( SignalArgs& args) const
{
  CFinfo << uri().path() << " [" << derived_type_name() << "]" << CFendl;
//  BOOST_FOREACH(const Component& c, *this )
//  {
//    CFinfo << "G.add_edge('" << uri().path() << "','" << c.uri().path() << "',weight=1.0)" << CFflush << CFendl;
//    c.signal_list_tree_recursive( args );
//  }
}

uri().path()=        /Model/RDSolver
derived_type_name()= cf3.RDM.RDSolver
name()=              RDSolver
uri().base_path()=   cpath:/Model
uri().base_name()=   RDSolver
uri().name()=        RDSolver
uri().path()=        /Model/RDSolver
*/

  CFinfo << "HI: " << c.uri().path() << CFendl;

  BOOST_FOREACH(const Component& subc, c )
  {
    append_components_recursive(subc, coll, depth+1);
  }

}

////////////////////////////////////////////////////////////////////////////////

} // namespace python
} // namespace cf3
