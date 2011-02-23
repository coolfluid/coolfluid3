// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "GUI/Network/ComponentNames.hpp"

#include "Common/XML/SignalFrame.hpp"

#include "GUI/Client/Core/ClientRoot.hpp"
#include "GUI/Client/Core/NPlotXY.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Common::XML;

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientCore {

//////////////////////////////////////////////////////////////////////////////

NPlotXY::NPlotXY(const QString & name) :
    CNode( name, "NPlotXY", PLOTXY_NODE )
{
  regist_signal("convergence_history", "Lists convergence history", "Get history")->
      connect( boost::bind( &NPlotXY::convergence_history, this, _1));
}

//////////////////////////////////////////////////////////////////////////////

QString NPlotXY::toolTip() const
{
  return getComponentType();
}

//////////////////////////////////////////////////////////////////////////////

void NPlotXY::convergence_history ( Signal::arg_t& node )
{
  SignalFrame& options = node.map( Protocol::Tags::key_options() );

  std::vector<QString> fct_label;

  std::vector<Real> x_axis = options.get_array<Real>("x_axis");
  fct_label.push_back("x");
  std::vector<Real> y_axis = options.get_array<Real>("y_axis");
  fct_label.push_back("y");

  std::vector< std::vector<Real> > fcts(2);
  fcts[0] = x_axis;
  fcts[1] = y_axis;

  CHistoryNotifier::instance().notify_history(fcts,fct_label);

  /*
    for( int x = 0 ; x < x_axis.size() ; ++x)
      NLog::globalLog()->addMessage("Avant parsing");
    */
}

//////////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF
