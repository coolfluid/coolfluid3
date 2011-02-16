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
  d_crv3 = new QwtPlotCurve("");
  d_crv3->setRenderHint(QwtPlotItem::RenderAntialiased);
  d_crv3->setPen(QPen(Qt::red));
  d_crv3->setYAxis(QwtPlot::yLeft);
  d_crv3->attach(this);

  const bool doReplot = autoReplot();
  setAutoReplot(false);

  sinus();

  setAutoReplot(doReplot);

  replot();

  setAutoReplot(true);
}

////////////////////////////////////////////////////////////////////////////////

void BodePlot::sinus(){
  //to do
  xs = vector<double>(100);
  ys = vector<double>(100);
  for (double x = 0; x < 100; ++x)
  {
    xs.push_back(x);
    ys.push_back((sin(x/10) * 20));
  }
  //

  d_crv3->setData(&xs[0],&ys[0],xs.size());

}

////////////////////////////////////////////////////////////////////////////////

BodePlot::BodePlot(QWidget *parent,bool test):
    QwtPlot(parent),
    labelX("axe X"),
    labelY("axe Y"),
    showXYLabel(false),
    backgroundColor(Qt::white),
    fctColor(Qt::red),
    fctName(""),
    showFctName(false),
    title(""),
    showTitle(false),
    showLegend(false),
    gridColor(Qt::gray),
    logaritmicScale(false)
{
  drawGraph();
}

////////////////////////////////////////////////////////////////////////////////

void BodePlot::drawGraph()
{
  setAutoReplot(false);

  setTitle(title);

  setCanvasBackground(backgroundColor);

  // legend
  if(showLegend){
    QwtLegend *legend = new QwtLegend;
    legend->setFrameStyle(QFrame::Box|QFrame::Sunken);
    insertLegend(legend, QwtPlot::BottomLegend);
  }

  // grid
  QwtPlotGrid *grid = new QwtPlotGrid;
  grid->enableXMin(true);
  grid->setMajPen(QPen(gridColor, 0, Qt::DotLine));
  grid->setMinPen(QPen(gridColor, 0 , Qt::DotLine));
  grid->attach(this);

  // axes
  setAxisTitle(QwtPlot::xBottom, labelX);
  setAxisTitle(QwtPlot::yLeft, labelY);

  setAxisMaxMajor(QwtPlot::xBottom, 6); //?
  setAxisMaxMinor(QwtPlot::xBottom, 10); //?
  if(logaritmicScale){
    setAxisScaleEngine(QwtPlot::xBottom,  new QwtLog10ScaleEngine); //echelle lineaire ou log
  }else{
    setAxisScaleEngine(QwtPlot::xBottom,  new QwtLinearScaleEngine); //echelle lineaire ou log
  }


  // curves
  d_crv3 = new QwtPlotCurve(fctName);
  d_crv3->setRenderHint(QwtPlotItem::RenderAntialiased);
  d_crv3->setPen(QPen(fctColor));
  d_crv3->setYAxis(QwtPlot::yLeft);
  d_crv3->attach(this);

  const bool doReplot = autoReplot();
  setAutoReplot(false);

  sinus();

  setAutoReplot(doReplot);

  replot();

  setAutoReplot(true);
}

////////////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF
