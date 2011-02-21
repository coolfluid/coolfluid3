// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "qwt/qwt_legend.h"
#include "qwt/qwt_plot_curve.h"
#include "qwt/qwt_plot_grid.h"
#include "qwt/qwt_scale_engine.h"

#include "GUI/Client/UI/BodePlot.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace std;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientUI {

////////////////////////////////////////////////////////////////////////////////

BodePlot::BodePlot(QWidget *parent):
    QwtPlot(parent)
{

  setAutoReplot(false);

  setCanvasBackground(QColor(Qt::white));

  // grid
  QwtPlotGrid *grid = new QwtPlotGrid;
  grid->enableXMin(true);
  grid->setMajPen(QPen(Qt::white, 0, Qt::DotLine));
  grid->setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
  grid->attach(this);

  // axes
  setAxisTitle(QwtPlot::xBottom, "axe X");
  setAxisTitle(QwtPlot::yLeft, "axe Y");

  setAxisMaxMajor(QwtPlot::xBottom, 6); //?
  setAxisMaxMinor(QwtPlot::xBottom, 10); //?
  setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine); //echelle lineaire ou log

  // curves
  m_curve = new QwtPlotCurve("");
  m_curve->setRenderHint(QwtPlotItem::RenderAntialiased);
  m_curve->setPen(QPen(Qt::red));
  m_curve->setYAxis(QwtPlot::yLeft);
  m_curve->attach(this);

  const bool doReplot = autoReplot();
  setAutoReplot(false);

  setAutoReplot(doReplot);

  replot();

  setAutoReplot(true);

}

BodePlot::~BodePlot(){
/*
  if(m_curve != 0){
    delete m_curve;
    m_curve = 0;
  }
*/
}

////////////////////////////////////////////////////////////////////////////////

BodePlot::BodePlot(QWidget *parent,bool test):
    QwtPlot(parent),
    m_label_x("axe X"),
    m_label_y("axe Y"),
    m_show_xy_label(false),
    m_background_color(Qt::white),
    m_fct_color(Qt::red),
    m_fct_name(""),
    m_show_fct_name(false),
    m_title(""),
    m_show_title(false),
    m_show_legend(false),
    m_grid_color(Qt::gray),
    m_logaritmic_scale(false),
    m_antialiasing(false),
    m_legend(0)
{
  setAutoReplot(false);

  draw_graph();

  show_xy_label_on_graph(true);
  set_fct_name("Gil's function");
  show_fct_name_on_graph(true);
  show_legend_on_graph(true);
  set_graph_title("Gil's awsome graph");
  show_title_on_graph(true);
}

////////////////////////////////////////////////////////////////////////////////

void BodePlot::draw_graph()
{
  setTitle(m_title);

  setCanvasBackground(m_background_color);

  // legend
  if(m_show_legend){
    if(m_legend != 0){
      m_legend->~QwtLegend();
    }
    m_legend = new QwtLegend;
    m_legend->setFrameStyle(QFrame::Box|QFrame::Sunken);
    insertLegend(m_legend, QwtPlot::BottomLegend);
  }

  // grid
  m_grid = new QwtPlotGrid;
  m_grid->enableXMin(true);
  m_grid->setMajPen(QPen(m_grid_color, 0, Qt::DotLine));
  m_grid->setMinPen(QPen(m_grid_color, 0 , Qt::DotLine));
  m_grid->attach(this);

  // axes
  if(m_show_xy_label){
    setAxisTitle(QwtPlot::xBottom, m_label_x);
    setAxisTitle(QwtPlot::yLeft, m_label_y);
  }

  setAxisMaxMajor(QwtPlot::xBottom, 6); //?
  setAxisMaxMinor(QwtPlot::xBottom, 10); //?
  if(m_logaritmic_scale){
    setAxisScaleEngine(QwtPlot::xBottom,  new QwtLog10ScaleEngine); //echelle lineaire ou log
  }else{
    setAxisScaleEngine(QwtPlot::xBottom,  new QwtLinearScaleEngine); //echelle lineaire ou log
  }


  // curves
  m_curve = new QwtPlotCurve(m_fct_name);
  if(m_antialiasing){
    m_curve->setRenderHint(QwtPlotItem::RenderAntialiased);
  }
  m_curve->setPen(QPen(m_fct_color));
  m_curve->setYAxis(QwtPlot::yLeft);
  m_curve->attach(this);

  //sinus();

  replot();
}

////////////////////////////////////////////////////////////////////////////////

void BodePlot::add_xy_data_on_graph(std::vector<double> & xs,std::vector<double> & ys){
  if(m_curve != 0){

    //add new data to vectors, deep copy
    for (int i = 0; i < xs.size(); ++i)
    {
      m_xs.push_back(xs[i]);
      m_ys.push_back(ys[i]);
    }

    m_curve->setRawData(&m_xs[0],&m_ys[0],m_xs.size());

    replot();
  }
}

void BodePlot::set_xy_data_on_graph(std::vector<double> & xs,std::vector<double> & ys){
  if(m_curve != 0){
    /*
    //clear vector
    m_xs.clear();
    m_ys.clear();

    //deep copy
    for (int i = 0; i < xs.size(); ++i)
    {
      m_xs.push_back(xs[i]);
      m_ys.push_back(ys[i]);
    }

    m_curve->setData(&m_xs[0],&m_ys[0],m_xs.size());
    */

    //m_curve->setData(&xs[0],&ys[0],xs.size());

    m_xs = xs;
    m_ys = ys;

    m_curve->setRawData(&m_xs[0],&m_ys[0],m_xs.size());

    replot();
  }
}

void BodePlot::set_x_label(QString new_label_x){
  m_label_x = new_label_x;
  setAxisTitle(QwtPlot::xBottom, m_label_x);
  replot();
}

void BodePlot::set_y_label(QString new_label_y){
  m_label_y = new_label_y;
  setAxisTitle(QwtPlot::yLeft, m_label_y);
  replot();
}

void BodePlot::show_xy_label_on_graph(bool show){
  m_show_xy_label = show;
  if(m_show_xy_label){
    setAxisTitle(QwtPlot::xBottom, m_label_x);
    setAxisTitle(QwtPlot::yLeft, m_label_y);
  }else{
    setAxisTitle(QwtPlot::xBottom, "");
    setAxisTitle(QwtPlot::yLeft, "");
  }
  replot();
}

void BodePlot::set_background_color(QColor color){
  m_background_color = color;
  setCanvasBackground(m_background_color);
  replot();
}

void BodePlot::set_fct_color(QColor color){
  m_fct_color = color;
  m_curve->setPen(QPen(m_fct_color));
  replot();
}

void BodePlot::set_fct_name(QString new_fct_name){
  m_fct_name = new_fct_name;
  show_fct_name_on_graph(m_show_fct_name);
  show_legend_on_graph(m_show_legend);
  replot();
}

void BodePlot::show_fct_name_on_graph(bool show){
  m_show_fct_name = show;
  m_curve->setTitle(m_fct_name);
  replot();
}

void BodePlot::set_graph_title(QString new_title){
  m_title = new_title;
  show_title_on_graph(m_show_title);
  replot();
}

void BodePlot::show_title_on_graph(bool show){
  m_show_title = show;
  if(m_show_title){
    setTitle(m_title);
  }else{
    setTitle("");
  }
  replot();
}

void BodePlot::show_legend_on_graph(bool show){
  m_show_legend = show;
  if(m_show_legend){
    if(m_legend != 0){
      m_legend->~QwtLegend();
    }
    m_legend = new QwtLegend;
    m_legend->setFrameStyle(QFrame::Box|QFrame::Sunken);
    insertLegend(m_legend, QwtPlot::BottomLegend);
  }else{

  }
  replot();
}

void BodePlot::set_grid_color(QColor color){
  m_grid_color = color;
  m_grid->setMajPen(QPen(m_grid_color, 0, Qt::DotLine));
  m_grid->setMinPen(QPen(m_grid_color, 0 , Qt::DotLine));
  replot();
}

void BodePlot::logaritmic_scale_on_graph(bool logaritmic){
  m_logaritmic_scale = logaritmic;
  if(m_logaritmic_scale){
    setAxisScaleEngine(QwtPlot::xBottom,  new QwtLog10ScaleEngine); //echelle lineaire ou log
  }else{
    setAxisScaleEngine(QwtPlot::xBottom,  new QwtLinearScaleEngine); //echelle lineaire ou log
  }
  replot();
}

} // ClientUI
} // GUI
} // CF
