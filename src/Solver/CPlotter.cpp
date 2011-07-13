// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"
#include "Common/Signal.hpp"

#include "Common/XML/SignalOptions.hpp"

#include "Solver/CPlotXY.hpp"
#include "Solver/LibSolver.hpp"

#include "Solver/CPlotter.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Common::XML;

namespace CF {
namespace Solver {

////////////////////////////////////////////////////////////////////////////////

ComponentBuilder < CPlotter, Component, LibSolver > CPlotter_Builder;

////////////////////////////////////////////////////////////////////////////////

CPlotter::CPlotter(const std::string & name) :
   Component(name)
{
  // signals
  regist_signal("create_xyplot", "Creates an XY-Plot", "New XY-Plot")->
      signal->connect( boost::bind(&CPlotter::signal_create_xyplot, this, _1) );

  // hide some signals from the GUI
  signal("create_component")->is_hidden = true;
  signal("delete_component")->is_hidden = true;
  signal("move_component")->is_hidden = true;
  signal("rename_component")->is_hidden = true;

  // signatures
  signal("create_xyplot")->signature->connect( boost::bind(&CPlotter::signature_create_xyplot, this, _1) );
}

////////////////////////////////////////////////////////////////////////////////

void CPlotter::signal_create_xyplot(SignalArgs &args)
{
  SignalOptions options( args );

  std::string name = options.value<std::string>("Plot name");
  URI parent = options.value<URI>("Parent");

  // some checks
  if(name.empty())
    throw BadValue(FromHere(), "The plot name is empty.");

  if(parent.empty())
    throw BadValue(FromHere(), "The parent is empty.");

  if(parent.scheme() != URI::Scheme::CPATH)
    throw InvalidURI(FromHere(), "The parent scheme is not CPATH");

  // create and add the component
  Component::Ptr parent_comp = Core::instance().root().access_component_ptr(parent);
  CPlotXY::Ptr plot(new CPlotXY(name));
  parent_comp->add_component( plot );
  plot->mark_basic();
  plot->set_data(m_data);
}

////////////////////////////////////////////////////////////////////////////////

void CPlotter::signature_create_xyplot(SignalArgs &args)
{
  SignalOptions options( args );

  options.add_option< OptionT<std::string> >("Plot name", std::string() )
      ->set_description("Name for the new plot");

  options.add_option< OptionURI >("Parent", Core::instance().root().uri() )
      ->set_description("Parent of the new component")
      ->cast_to<OptionURI>()->supported_protocol( URI::Scheme::CPATH );
}

////////////////////////////////////////////////////////////////////////////////

void CPlotter::set_data_set(const URI &uri)
{
  cf_assert ( !uri.empty() );
  cf_assert ( uri.scheme() == URI::Scheme::CPATH );

  m_data = uri;
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

