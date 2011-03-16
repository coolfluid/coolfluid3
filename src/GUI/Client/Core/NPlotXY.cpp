// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/multi_array/storage_order.hpp>

#include "Common/Signal.hpp"

#include "Common/XML/Protocol.hpp"

#include "GUI/Network/ComponentNames.hpp"

#include "GUI/Client/Core/TreeThread.hpp"

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
      signal->connect( boost::bind( &NPlotXY::convergence_history, this, _1));
}

//////////////////////////////////////////////////////////////////////////////

QString NPlotXY::toolTip() const
{
  return getComponentType();
}

//////////////////////////////////////////////////////////////////////////////

void NPlotXY::convergence_history ( SignalArgs& node )
{
  SignalFrame& options = node.map( Protocol::Tags::key_options() );


  int nbRows = 1000;
  int nbCols = 8;


  std::vector<QString> fct_label(9);

  fct_label[0] = "#";
  fct_label[1] = "x";
  fct_label[2] = "y";
  fct_label[3] = "z";
  fct_label[4] = "u";
  fct_label[5] = "v";
  fct_label[6] = "w";
  fct_label[7] = "p";
  fct_label[8] = "t";

  std::vector<Real> data = options.get_array<Real>("Table");

  // Store last dimension, then first, then middle
  PlotData::size_type ordering[] = {0,1};
  // Store the first dimension(dimension 0) in descending order
  bool ascending[] = {true,true};

  PlotDataPtr plot( new PlotData(boost::extents[nbRows][nbCols+1],
                  boost::general_storage_order<2>(ordering, ascending)) );

  for(PlotData::index row = 0;row<nbRows;++row)
  {
    (*plot)[row][0] = row;
  }

  for(PlotData::index row = 0; row != nbRows; ++row)
  {
    for(PlotData::index col = 0; col != nbCols; ++col)
      (*plot)[row][col+1] = data[(row * nbCols) + col];
  }

  NPlotXYNotifier::instance().notify_history(plot ,fct_label);

}

//////////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF
