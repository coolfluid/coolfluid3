// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef CF_GUI_Client_UI_Graph_hpp
#define CF_GUI_Client_UI_Graph_hpp

////////////////////////////////////////////////////////////////////////////////

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>

#include "qwt/qwt_polygon.h"
#include "GUI/Client/UI/GraphOption.hpp"

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

  /// Graph's constructor.
  /// @param parent Parent of this QWidget.
  Graph(QWidget *parent = 0);

  /// Graph's destructor
  ~Graph();

  /*
  /// Add data to the current graph.
  /// @param xs The new xs to add to the graph.
  /// @param xy The new xy to add to the graph (corresponding with the xs).
  void set_xy_data(std::vector<double> & xs, std::vector<double> & ys);

  /// Erase existing data and put the new one on the current graph.
  /// @param xs The new xs to set.
  /// @param xy The new xy to set (corresponding with the xs).
  void add_xy_data(std::vector<double> & xs, std::vector<double> & ys);
  */

  /// Add data to the current graph.
  /// @param fcts The new data in a 2d vector.
  void add_xy_data(std::vector< std::vector<double> > & fcts);

  /// Erase existing data and put the new one on the current graph.
  /// @param fcts The new data in a 2d vector.
  void set_xy_data(std::vector< std::vector<double> > & fcts);

private: //function

  /// Show the string in the m_label_bottom, or recomendation if string is empty.
  /// @param text Text to show.
  void show_info(QString text = QString::null);


private: //data
  /// The BodePlot who is shown in the graph x,y axes area.
  BodePlot * m_plot;

  /// QLabel that stand on the bottom where we display informations
  QLabel * m_label_bottom;

  /// QLayout that contant the option menu
  GraphOption * graph_option;

  /// Array that content the 2 type of zoom ( rectangle zoom, whole zoom )
  QwtPlotZoomer * m_zoomer[2];
  /// Represent the cursor when over the canvas
  QwtPlotPicker * m_picker;
  /// Panner that contain the canvas
  QwtPlotPanner * m_panner;

  QLineEdit * m_line_x1;
  QLineEdit * m_line_x2;
  QLineEdit * m_line_y1;
  QLineEdit * m_line_y2;
  QPushButton * m_bt_zoom_coord;

private slots:

  /// Called when moving while clicking
  void moved(const QPoint &);

  /// Called on mous click
  void selected(const QwtPolygon &);

  /// Called by btn_svg button
  void export_svg();

  /// Enable or disable zoom mode, when enable, the mouse is used to zoom
  /// @param zoom_enable Enale zoom.
  void enable_zoom_mode(bool zoom_enable);

  /// Set the x,y scale of the graph.
  void set_scale();

}; // Graph

////////////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UI_Graph_hpp
