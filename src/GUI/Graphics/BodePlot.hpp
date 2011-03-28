// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef CF_GUI_Graphics_BodePlot_hpp
#define CF_GUI_Graphics_BodePlot_hpp

////////////////////////////////////////////////////////////////////////////////

// Qwt header
#include "qwt/qwt_plot.h"
#include "qwt/qwt_plot_curve.h"
#include "qwt/qwt_plot_grid.h"

// header
#include "Common/CF.hpp"
#include "GUI/Graphics/LibGraphics.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Graphics {

////////////////////////////////////////////////////////////////////////////////

/// @brief This is a QwtPlot with easiest way to acces the options and add curves.
/// @author Wertz Gil
class Graphics_API BodePlot: public QwtPlot
{
    Q_OBJECT

public:
    //constructor
    /// Constructor minimum.
    /// @param parent Parent of this wtPlot.
    BodePlot(QWidget *parent);

    /// Constructor test.
    /// @param parent Parent of this wtPlot.
    /// @param test Variable test.
    BodePlot(QWidget *parent,bool test);

    //destructor
    /// Destructor
    ~BodePlot();

public:
    //Function
    /// Add data to the current graph.
    /// @param xs The new xs to add to the graph.
    /// @param xy The new xy to add to the graph (corresponding with the xs).
    void add_xy_data_on_graph(std::vector<double> & xs,std::vector<double> & ys);

    /// Erase existing data and put the new one on the current graph.
    /// @param xs The new xs to set.
    /// @param xy The new xy to set (corresponding with the xs).
    void set_xy_data_on_graph(std::vector<double> & xs,std::vector<double> & ys);

    /// Set the x axe label.
    /// @param new_label_x The new x axe's label.
    void set_x_label(QString new_label_x);

    /// Set the y axe label.
    /// @param new_label_y The new y axe's label.
    void set_y_label(QString new_label_y);

    /// Set to show or not the axes labels.
    /// @param show Show the axes labels.
    void show_xy_label_on_graph(bool show);

    /// Set the background color.
    /// @param color The new background color.
    void set_background_color(QColor color);

    /// Set the Function color.
    /// @param color The new Function color.
    void set_fct_color(QColor color);

    /// Set the Function name.
    /// @param new_fct_name The new Function name.
    void set_fct_name(QString new_fct_name);

    /// Set to show or not the Function name.
    /// @param show Show the the Function name.
    void show_fct_name_on_graph(bool show);

    /// Set the title.
    /// @param new_title The new title.
    void set_graph_title(QString new_title);

    /// Set to show or not the title.
    /// @param show Show the the title.
    void show_title_on_graph(bool show);

    /// Set to show or not the legend.
    /// @param show Show the the legend.
    void show_legend_on_graph(bool show);

    /// Set the grid color.
    /// @param color The new grid color.
    void set_grid_color(QColor color);

    /// Set the logarithmic graph type, if false then linear type.
    /// @param logaritmic Set graph type to logarithmic or not.
    void logaritmic_scale_on_graph(bool logaritmic);


private:
    //Function

    /// draw the graph.
    void draw_graph();

private:
    //data
    /// The curve to draw.
    QwtPlotCurve * m_curve;

    /// Vector of x positions.
    std::vector<double> m_xs;

    /// Vector of y positions.
    std::vector<double> m_ys;

    /// X axe label.
    QString m_label_x;
    /// Y axe label.
    QString m_label_y;
    /// Show axes label.
    bool m_show_xy_label;
    /// Background color
    QColor m_background_color;
    /// Function color.
    QColor m_fct_color;
    /// Function name.
    QString m_fct_name;
    /// Show Function name.
    bool m_show_fct_name;
    QString m_title;
    /// Show title.
    bool m_show_title;
    /// Show legend.
    bool m_show_legend;
    /// Grid color.
    QColor m_grid_color;
    /// Set logarit;ic or linear grid
    bool m_logaritmic_scale;
    /// grid of the graph
    QwtPlotGrid * m_grid;
    /// Set the antialiasing
    bool m_antialiasing;
    /// Legend
    QwtLegend * m_legend;

  }; // BodePlot

////////////////////////////////////////////////////////////////////////////////

} // Graphics
} // GUI
} // CF

#endif // CF_GUI_Graphics_BodePlot_hpp
