// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <cmath>

#include <boost/assign/list_of.hpp>

#include "Common/CBuilder.hpp"
#include "Common/Signal.hpp"
#include "Common/XML/MultiArray.hpp"

#include "Mesh/CTable.hpp"

#include "Solver/LibSolver.hpp"

#include "Solver/CPlotXY.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace boost::assign;
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
    SignalFrame reply = args.create_reply( uri() );
    SignalFrame& options = reply.map( Protocol::Tags::key_options() );
//    std::vector<Real> data(8000);
    CTable<Real>& table = *m_data.get();
    std::vector<std::string> labels =
        list_of<std::string>("x")("y")("z")("u")("v")("w")("p")("t");

    add_multi_array_in(options.main_map, "Table", m_data->array(), ";", labels);

//    for(Uint row = 0 ; row < 1000 ; ++row)
//    {
//      for(Uint col = 0 ; col < 8 ; ++col)
//        data[ (row * 8) + col ] = table[row][col];
//    }

//    XmlNode node = options.add("Table", data, " ; ");

//    node.set_attribute("dimensions", "8");
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
