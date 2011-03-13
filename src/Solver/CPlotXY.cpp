// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <cmath>

#include "Common/CBuilder.hpp"
#include "Common/Signal.hpp"

#include "Mesh/CTable.hpp"

#include "Solver/LibSolver.hpp"

#include "Solver/CPlotXY.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Mesh;

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Solver {

/////////////////////////////////////////////////////////////////////////////////

ComponentBuilder < CPlotXY, Component, LibSolver> CPlotXY_Builder;

/////////////////////////////////////////////////////////////////////////////////

CPlotXY::CPlotXY(const std::string& name) :
    Component(name),
    m_num_it(10000)
{
  regist_signal("convergence_history", "Lists convergence history", "Get history")->
      signal->connect( boost::bind( &CPlotXY::convergence_history, this, _1));

  // hide some signals from the GUI
  signal("create_component")->is_hidden = true;
//  signal("delete_component")->is_hidden = true;
//  signal("move_component")->is_hidden = true;
//  signal("rename_component")->is_hidden = true;
}

/////////////////////////////////////////////////////////////////////////////////////

CPlotXY::~CPlotXY()
{

}

/////////////////////////////////////////////////////////////////////////////////////

void CPlotXY::convergence_history( SignalArgs & args )
{
  if( is_not_null(m_data.get()) )
  {
    SignalFrame reply = args.create_reply( full_path() );
    SignalFrame& options = reply.map( Protocol::Tags::key_options() );
    std::vector<Real> data(8000);
    CTable<Real>& table = *m_data.get();

    for(Uint row = 0 ; row < 1000 ; ++row)
    {
      for(Uint col = 0 ; col < 8 ; ++col)
        data[ (row * 8) + col ] = table[row][col];
    }

    XmlNode node = options.set_array("Table", data, " ; ");

    node.set_attribute("dimensions", "8");
  }
  else
    throw SetupError( FromHere(), "Data to plot not setup" );
}

/////////////////////////////////////////////////////////////////////////////////////

void CPlotXY::set_data(const URI &uri)
{
  cf_assert( !m_root.expired() );

  m_data = m_root.lock()->access_component(uri).as_ptr< CTable<Real> >();
}

/////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
