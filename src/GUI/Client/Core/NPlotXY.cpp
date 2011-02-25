// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QDebug>

#include <iostream>

#include <boost/multi_array/storage_order.hpp>

#include "GUI/Network/ComponentNames.hpp"
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

//  for(int row = 0; row < nbRows; ++row)
//  {
////    for(m_array_t::index col = 0 ; col < nbCols ; ++nbCols)
////      plot[row][col] = (col + 1) * 1000 + row ;

//    plot[row][0] = 1000 + row ;
//    plot[row][1] = 2000 + row ;
//    plot[row][2] = 3000 + row ;
//    plot[row][3] = 4000 + row ;
//    plot[row][4] = 5000 + row ;
//    plot[row][5] = 6000 + row ;
//    plot[row][6] = 7000 + row ;
//    plot[row][7] = 8000 + row ;
//  }
/*
  for (int x = 0; x < nbRows; ++x)
  {
    for (int y = 0; y < nbCols; ++y)
    {
      std::cout << plot[x][y] << ' ' ;
    }

    std::cout << std::endl;
  }

  std::cout << std::endl << std::endl;
*/
  //+++++++++++++++++++++++++++++++++++++++++++++++
/*
  std::cout << "Number of lines: " << plot.size() << std::endl;

  for (int x = 0; x < plot.size(); ++x)
  {
    std::cout << "Number of columns for array " << x << ": " << plot[x].size() << std::endl ;
  }
*/
  //+++++++++++++++++++++++++++++++++++++++++++++++
/*
  std::cout << std::endl << std::endl;

  double * x = &plot[0][0];

  std::cout << plot.num_elements() << std::endl;

  for (int i = 0; i < plot.num_elements(); ++i,++x)
  {
    if( i % 1000 == 0 )
      std::cout << std::endl << std::endl;

    //double * x = plot[i * plot.size()].origin();

    std::cout << *x << " ";

//    for (int y = 0; y < plot.size(); ++y, ++x)
//    {
//      std::cout << *x << ' ' ;
//    }

  }
  std::cout << std::endl << std::endl;
*/
/***********

  SignalFrame& options = node.map( Protocol::Tags::key_options() );

  std::vector<QString> fct_label;

  fct_label.push_back("x");
  fct_label.push_back("y");
  fct_label.push_back("z");
  fct_label.push_back("u");
  fct_label.push_back("v");
  fct_label.push_back("w");
  fct_label.push_back("p");
  fct_label.push_back("t");

  std::vector<Real> data = options.get_array<Real>("table");
  Uint nbRows = 25;
  Uint nbCols = 8;
  Uint nbEntries = nbRows * nbCols;

//  CTable<Real>::Ptr table_ptr( new CTable<Real>("table"));
//  table_ptr->set_row_size( nbCols );
//  CTable<Real>::Buffer buffer = table_ptr->create_buffer( nbEntries );

//  std::vector<Real> row( nbCols );
//  for(Uint index = 0 ; index < nbEntries ; ++index )
//  {
//    Uint currCol = index % nbCols;
//    row[ currCol ] = data[ index ];

//    if(currCol == nbCols - 1 ) // if we just filled in a row, we add it to the buffer
//      buffer.add_row(row);
//  }

//  buffer.flush();


  // Store last dimension, then first, then middle
  PlotData::size_type ordering[] = {0,1};
  // Store the first dimension(dimension 0) in descending order
  bool ascending[] = {false,false};

  PlotDataPtr plot( new PlotData(boost::extents[nbRows][nbCols],
                                 boost::general_storage_order<2>(ordering, ascending)) );

  PlotData& plot_data = *plot;


//  for(PlotData::index row = 0; row != nbRows; ++row)
//  {
//    for(PlotData::index col = 0; col != nbCols; ++col)
//      plot_data[row][col] = data[(row * nbCols) + col];
//  }

  for(int row = 0; row < nbRows; ++row)
  {
//    for(m_array_t::index col = 0 ; col < nbCols ; ++nbCols)
//      plot[row][col] = (col + 1) * 1000 + row ;

    plot_data[row][0] = 1000 + row ;
    plot_data[row][1] = 2000 + row ;
    plot_data[row][2] = 3000 + row ;
    plot_data[row][3] = 4000 + row ;
    plot_data[row][4] = 5000 + row ;
    plot_data[row][5] = 6000 + row ;
    plot_data[row][6] = 7000 + row ;
    plot_data[row][7] = 8000 + row ;
  }

  for(int row = 0; row < nbRows; ++row)
  {
//    for(m_array_t::index col = 0 ; col < nbCols ; ++nbCols)
//      plot[row][col] = (col + 1) * 1000 + row ;

    qDebug() << plot_data[row][0] << plot_data[row][1] << plot_data[row][2] <<
        plot_data[row][3] << plot_data[row][4] << plot_data[row][5] <<
        plot_data[row][6] << plot_data[row][7] ;
  }


  qDebug() << "Number of rows:" << plot_data.size();

  for (int i = 0; i < plot_data[0].size(); ++i)
  {
    double * x = plot_data[i].origin();
    qDebug() << "Number of rows for column" << i << ":" << plot_data[i].size() << "---->\t"
        << *(x++) << *(x++) << *(x++) << *(x++) << *(x++) << *(x++) << *(x++) << *(x++)
        << *(x++) << *(x++) << *(x++) << *(x++) << *(x++) << *(x++) << *(x++) << *(x++)
        << *(x++) << *(x++) << *(x++) << *(x++) << *(x++) << *(x++) << *(x++) << *(x++) << *(x++);


//    std::cout << std::endl;
  }

//  for(int i = 0 ; i < 1000 ; i++)
////          qDebug() << curr_index1 << *(x++) /*<< *(++y)*/;
//    qDebug()
//        << plot_data[i][0] << '\t'
//        << plot_data[i][1] << '\t'
//        << plot_data[i][2] << '\t'
//        << plot_data[i][3] << '\t'
//        << plot_data[i][4] << '\t'
//        << plot_data[i][5] << '\t'
//        << plot_data[i][6] << '\t'
//        << plot_data[i][7] ;


//  std::vector< std::vector<Real> > fcts(2);
//  fcts[0] = table;
//  fcts[1] = y_axis;

  NPlotXYNotifier::instance().notify_history(plot ,fct_label);

  /*
  *
    for( int x = 0 ; x < x_axis.size() ; x++)
      NLog::globalLog()->addMessage("Avant parsing");
    *

    ****************/
}

//////////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF
