// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <cmath>

#include <boost/assign/list_of.hpp>

#include "common/Builder.hpp"
#include "common/Signal.hpp"
#include "common/XML/MultiArray.hpp"
#include "common/Table.hpp"

#include "solver/LibSolver.hpp"

#include "solver/PlotXY.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace boost::assign;
using namespace cf3::common;
using namespace cf3::common::XML;

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {

/////////////////////////////////////////////////////////////////////////////////

ComponentBuilder < PlotXY, Component, LibSolver> PlotXY_Builder;

/////////////////////////////////////////////////////////////////////////////////

PlotXY::PlotXY(const std::string& name) :
    Component(name),
    m_num_it(10000)
{
  regist_signal( "convergence_history" )
    .connect( boost::bind( &PlotXY::convergence_history, this, _1 ) )
    .description("Lists convergence history")
    .pretty_name("Get history");

  // hide some signals from the GUI
  signal("create_component")->hidden(true);
//  signal("delete_component")->hidden(true);
//  signal("move_component")->hidden(true);
//  signal("rename_component")->hidden(true);
}

/////////////////////////////////////////////////////////////////////////////////////

PlotXY::~PlotXY()
{

}

/////////////////////////////////////////////////////////////////////////////////////

void PlotXY::convergence_history( SignalArgs & args )
{
  if( is_not_null(m_data.get()) )
  {
    SignalFrame reply = args.create_reply( uri() );
    SignalFrame& options = reply.map( Protocol::Tags::key_options() );
//    std::vector<Real> data(8000);
    Table<Real>& table = *m_data.get();
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

void PlotXY::set_data(const URI &uri)
{
  m_data = Handle< Table<Real> >(access_component(uri));
}

/////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
