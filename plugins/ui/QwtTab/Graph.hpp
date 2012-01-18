// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UI_QwtTab_Graph_hpp
#define cf3_UI_QwtTab_Graph_hpp

////////////////////////////////////////////////////////////////////////////////

// Qt
#include <QPointer>

// Qwt header
#include "qwt/qwt_polygon.h"

// header
#include "ui/QwtTab/GraphOption.hpp"

////////////////////////////////////////////////////////////////////////////////

// forward declaration to avoid incuding files
// Qwt
class QwtPlotZoomer;
class QwtPlotPicker;
class QwtPlotPanner;
// Qt
class QLabel;
class QWidget;
class QLineEdit;
class QPushButton;
class QHBoxLayout;
class QWheelEvent;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace QwtTab {

////////////////////////////////////////////////////////////////////////////////

class BodePlot;

/// @brief This hold the XY graph and the options graph.
/// @author Wertz Gil
class Graphics_API Graph: public QWidget
{
  Q_OBJECT

public: //function

  /// Graph's constructor.
  /// @param parent Parent of this QWidget.
  Graph(QWidget *parent = 0);

  /// Graph's destructor
  ~Graph();

  /// @warning nasty fix !! resolves compilation with Boost 1.48.
  /// should be removed Qt's moc tool will be fixed.
#ifndef Q_MOC_RUN
  /// Erase existing data and put the new one on the current graph.
  /// @param fcts The new data in a 2d vector.
  /// @param fct_label Label of each data set.
  void set_xy_data(NPlotXY::PlotDataPtr fcts, std::vector<QString> & fct_label);
#endif

private: //function

  /// Show the string in the m_label_bottom, or recomendation if string is empty.
  /// @param text Text to show.
  void show_info(QString text = QString::null);


private: //data
  /// The BodePlot who is shown in the graph x,y axes area.
  QPointer<BodePlot> m_plot;

  /// QLabel that stand on the bottom where we display informations
  QPointer<QLabel> m_label_bottom;

  /// QLayout that contant the option menu
  QPointer<GraphOption> m_graph_option;

  /// Zoomer ( rectangle zoom )
  QPointer<QwtPlotZoomer> m_zoomer;

  /// Represent the cursor when over the canvas
  QPointer<QwtPlotPicker> m_picker;

  /// Panner that contain the canvas
  QPointer<QwtPlotPanner> m_panner;

  /// minimum x scale
  QPointer<QLineEdit> m_line_min_x;

  /// maximum x scale
  QPointer<QLineEdit> m_line_max_x;

  /// minimum y scale
  QPointer<QLineEdit> m_line_min_y;

  /// maximum y scale
  QPointer<QLineEdit> m_line_max_y;

  /// Line Edit labels
  QPointer<QLabel> m_label_min_x;
  QPointer<QLabel> m_label_max_x;
  QPointer<QLabel> m_label_min_y;
  QPointer<QLabel> m_label_max_y;

  /// Set scale button
  QPointer<QPushButton> m_button_set_scale;

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

  /// Zoom on mouse scroll
  /// @param event The event emited when scrolling
  void zoomWheel(QWheelEvent* event);

}; // Graph

////////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_UI_QwtTab_Graph_hpp
