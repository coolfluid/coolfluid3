// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/multi_array/storage_order.hpp>

#include "Common/Signal.hpp"

#include "Common/XML/Protocol.hpp"
#include "Common/XML/MultiArray.hpp"

#include "UI/UICommon/ComponentNames.hpp"

#include "UI/Core/TreeThread.hpp"

#include "UI/QwtTab/NPlotXY.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Common::XML;

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace QwtTab {

//////////////////////////////////////////////////////////////////////////////

NPlotXY::NPlotXY(const std::string & name) :
    CNode( name, "NPlotXY", CNode::STANDARD_NODE )
{
  regist_signal("convergence_history", "Lists convergence history", "Get history")->
      signal->connect( boost::bind( &NPlotXY::convergence_history, this, _1));
}

//////////////////////////////////////////////////////////////////////////////

QString NPlotXY::toolTip() const
{
  return componentType();
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

  NPlotXYNotifier::instance().notify_history(plot ,fct_label);

}

//////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // CF
