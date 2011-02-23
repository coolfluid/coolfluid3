// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CPlotXY.hpp"
#include "Common/CBuilder.hpp"
#include "Common/LibCommon.hpp"
#include "Common/Log.hpp"

#include "Common/CPlotter.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

ComponentBuilder < CPlotter, Component, LibCommon > CPlotter_Builder;

////////////////////////////////////////////////////////////////////////////////

CPlotter::CPlotter(const std::string & name) :
   Component(name)
{
  // signals
  regist_signal("create_xyplot", "Creates an XY-Plot", "New XY-Plot")
      ->connect( boost::bind(&CPlotter::signal_create_xyplot, this, _1) );

  // signatures
  m_signals["create_xyplot"].signature->connect( boost::bind(&CPlotter::signature_create_xyplot, this, _1) );
}

////////////////////////////////////////////////////////////////////////////////

void CPlotter::signal_create_xyplot(Signal::arg_t &args)
{
  XML::SignalFrame& options = args.map( XML::Protocol::Tags::key_options() );

  std::string name = options.get_option<std::string>("Plot name");
  URI parent = options.get_option<URI>("Parent");

  // some checks
  if(name.empty())
    throw BadValue(FromHere(), "The plot name is empty.");

  if(parent.empty())
    throw BadValue(FromHere(), "The parent is empty.");

  if(parent.scheme() != URI::Scheme::CPATH)
    throw BadValue(FromHere(), "The parent scheme is not CPATH");

  // create and add the component
  Component::Ptr parent_comp = Core::instance().root()->look_component(parent);
  CPlotXY::Ptr plot(new CPlotXY(name));
  parent_comp->add_component( plot );
  plot->mark_basic();
}

////////////////////////////////////////////////////////////////////////////////


void CPlotter::signature_create_xyplot(Signal::arg_t &args)
{
  XML::SignalFrame& options = args.map( XML::Protocol::Tags::key_options() );

  options.set_option("Plot name", std::string(), "Name for the new plot");
  options.set_option("Parent", Core::instance().root()->full_path(), "Parent of the new component");
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

