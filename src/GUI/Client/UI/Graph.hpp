// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_UI_Graph_hpp
#define CF_GUI_Client_UI_Graph_hpp

////////////////////////////////////////////////////////////////////////////////

// Qt header
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>

// Qwt header
#include "qwt/qwt_polygon.h"

// header
#include "GUI/Client/UI/GraphOption.hpp"

// forward declaration to avoid incuding files
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

/// @brief This hold the XY graph and the options graph.
/// @author Wertz Gil
class ClientUI_API Graph: public QWidget
{
  Q_OBJECT

public: //function

  /// Graph's constructor.
  /// @param parent Parent of this QWidget.
  Graph(QWidget *parent = 0);

  /// Graph's destructor
  ~Graph();

  /// Erase existing data and put the new one on the current graph.
  /// @param fcts The new data in a 2d vector.
  /// @param fct_label Label of each data set.
  void set_xy_data(ClientCore::NPlotXY::PlotDataPtr & fcts,
                   std::vector<QString> & fct_label);

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

  /// Zoomer ( rectangle zoom )
  QwtPlotZoomer * m_zoomer;

  /// Represent the cursor when over the canvas
  QwtPlotPicker * m_picker;

  /// Panner that contain the canvas
  QwtPlotPanner * m_panner;

  /// minimum x scale
  QLineEdit * m_line_min_x;

  /// maximum x scale
  QLineEdit * m_line_max_x;

  /// minimum y scale
  QLineEdit * m_line_min_y;

  /// maximum y scale
  QLineEdit * m_line_max_y;

  /// Line Edit labels
  QLabel * m_label_min_x;
  QLabel * m_label_max_x;
  QLabel * m_label_min_y;
  QLabel * m_label_max_y;

  /// Set scale button
  QPushButton * m_button_set_scale;

public slots: //slots

  /// Reset the base scale zoom.
  void reset_base_zoom();

private slots: //slots

  /// Called when moving while clicking.
  /// @param point The point pointed by the pen/mouse.
  void moved(const QPoint & point);

  /// Called on mous click.
  /// @param polygon The selected polygon.
  void selected(const QwtPolygon & polygon);

  /// Export graph to an SVG file.
  void export_svg();

  /// Enable or disable zoom mode, when enable, the mouse is used to zoom.
  /// @param zoom_enable Enale zoom.
  void enable_zoom_mode(bool zoom_enable);

  /// show the graph's options
  void show_graph_option(bool);

  /// show the scale's options
  void show_scale_option(bool);

  /// Set the x,y scale of the graph.
  void set_scale();

}; // Graph

////////////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UI_Graph_hpp
