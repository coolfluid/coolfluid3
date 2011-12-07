// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

// header
#include <boost/multi_array/storage_order.hpp>
#include "common/BoostArray.hpp"

#include "common/Signal.hpp"
#include "common/XML/Protocol.hpp"
#include "common/XML/MultiArray.hpp"
#include "ui/uicommon/ComponentNames.hpp"
#include "ui/core/TreeThread.hpp"
#include "ui/graphics/TabBuilder.hpp"
#include "ui/QwtTab/Graph.hpp"
#include "ui/QwtTab/NPlotXY.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::ui::graphics;

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace QwtTab {

//////////////////////////////////////////////////////////////////////////////

NPlotXY::NPlotXY(const std::string & name) :
    CNode( name, "CPlotXY", CNode::STANDARD_NODE )
{
//  m_tabIndex = Graphics::TabBuilder::instance()->addTab(new Graph(), name.c_str());

//  TabBuilder::instance()->widget<Graph>(as_const());

  regist_signal( "convergence_history" )
      .connect( boost::bind( &NPlotXY::convergence_history, this, _1 ) )
      .description("Lists convergence history")
      .pretty_name("Get history");

  regist_signal("show_hide_plot")
      .connect( boost::bind( &NPlotXY::show_hide_plot, this, _1) )
      .description("Shows or hides the plot tab")
      .pretty_name("Show/Hide plot");

  regist_signal( "go_to_tab" )
      .connect( boost::bind( &NPlotXY::go_to_plot, this, _1 ) )
      .description("Activates the tab")
      .pretty_name("Switch to tab");

  m_local_signals << "show_hide_plot" << "go_to_tab";
}

//////////////////////////////////////////////////////////////////////////////

QString NPlotXY::tool_tip() const
{
  return component_type();
}

//////////////////////////////////////////////////////////////////////////////

void NPlotXY::disable_local_signals(QMap<QString, bool> & localSignals) const
{
  localSignals["show_hide_plot"] = false;
}

//////////////////////////////////////////////////////////////////////////////

void NPlotXY::show_hide_plot( common::SignalArgs& node )
{
//  TabBuilder & builder = *TabBuilder::instance();

//  if(builder.count() == 3)
//    builder.removeTab(m_tabIndex);
//  else
//    builder.insertTab(m_tabIndex, new Graph(), name().c_str());
}

//////////////////////////////////////////////////////////////////////////////

void NPlotXY::go_to_plot( common::SignalArgs& node )
{
  TabBuilder::instance()->show_tab( handle<CNode>() );
}

//////////////////////////////////////////////////////////////////////////////

void NPlotXY::setup_finished()
{
  TabBuilder::instance()->widget<Graph>( handle<CNode>() );
}

//////////////////////////////////////////////////////////////////////////////

void NPlotXY::convergence_history ( SignalArgs& node )
{
  SignalFrame& options = node.map( Protocol::Tags::key_options() );

  PlotDataPtr array( new PlotData() );
  std::vector<std::string> labels;

  get_multi_array(options.main_map, "Table", *array, labels);

  int nbRows = array->size();
  int nbCols = (*array)[0].size();
  std::vector<QString> fct_label(labels.size() + 1);

  fct_label[0] = "#";

  for(int i = 0 ; i < labels.size() ; ++i)
    fct_label[i+1] = labels[i].c_str();


  // Store last dimension, then first, then middle
  PlotData::size_type ordering[] = {0, 1};
  // Store the first dimension(dimension 0) in descending order
  bool ascending[] = {true, true};

  PlotDataPtr plot( new PlotData(boost::extents[nbRows][nbCols+1],
                  boost::general_storage_order<2>(ordering, ascending)) );

  for(PlotData::index row = 0;row<nbRows;++row)
  {
    (*plot)[row][0] = row;
  }

  for(PlotData::index row = 0; row != nbRows; ++row)
  {
    for(PlotData::index col = 0; col != nbCols; ++col)
      (*plot)[row][col+1] = (*array)[row][col];
  }

  TabBuilder::instance()->widget<Graph>(handle<CNode>())->set_xy_data(plot, fct_label);
}

//////////////////////////////////////////////////////////////////////////////

} // Core
} // ui
} // cf3
