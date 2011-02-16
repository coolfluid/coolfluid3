// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef CF_GUI_Client_UI_Graph_hpp
#define CF_GUI_Client_UI_Graph_hpp

////////////////////////////////////////////////////////////////////////////////

#include <QWidget>

#include "qwt/qwt_polygon.h"

class QLabel;

class QwtPlotZoomer;
class QwtPlotPicker;
class QwtPlotPanner;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientUI {

////////////////////////////////////////////////////////////////////////////////

class BodePlot;

class Graph: public QWidget
{
  Q_OBJECT

public:

  Graph(QWidget *parent = 0);

private slots:

  void moved(const QPoint &);

  void selected(const QwtPolygon &);

  //void print();
  void exportSVG();

  void enableZoomMode(bool);

private:

  void showInfo(QString text = QString::null);

  BodePlot *d_plot;

  QLabel * labelBottom;

  QwtPlotZoomer *d_zoomer[2];
  QwtPlotPicker *d_picker;
  QwtPlotPanner *d_panner;

}; // Graph

////////////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UI_Graph_hpp
