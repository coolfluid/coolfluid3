// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <cmath>

#include "Common/CreateComponent.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/CBuilder.hpp"
#include "Common/LibCommon.hpp"
#include "Common/Log.hpp"

#include "Common/CPlotXY.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common::XML;

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

ComponentBuilder < CPlotXY, Component, LibCommon > CPlotXY_Builder;

/////////////////////////////////////////////////////////////////////////////////////

CPlotXY::CPlotXY(const std::string& name) :
    Component(name),
    m_num_it(10000)
{
  regist_signal("convergence_history", "Lists convergence history", "Get history")->
      connect( boost::bind( &CPlotXY::convergence_history, this, _1));

  // hide some signals from the GUI
  signal("create_component").is_hidden = true;
  signal("delete_component").is_hidden = true;
  signal("move_component").is_hidden = true;
  signal("rename_component").is_hidden = true;
}

/////////////////////////////////////////////////////////////////////////////////////

CPlotXY::~CPlotXY()
{

}

/////////////////////////////////////////////////////////////////////////////////////

void CPlotXY::convergence_history( Signal::arg_t & args )
{
  SignalFrame reply = args.create_reply( full_path() );
  SignalFrame& options = reply.map( Protocol::Tags::key_options() );

  sine(m_num_it);

  options.set_array("x_axis", m_x_axis, " ; ");
  options.set_array("y_axis", m_y_axis, " ; ");
}

/////////////////////////////////////////////////////////////////////////////////////

void CPlotXY::sine(int points)
{
  m_x_axis = std::vector<double>(points);
  m_y_axis = std::vector<double>(points);

  for (double x = 0; x < points; ++x)
  {
    m_x_axis[x] = x; // x between 0 and points
    m_y_axis[x] = (std::sin(x/(points/10)) * 20); // y between 20 and -20
  }

  m_num_it += m_num_it;
}

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
