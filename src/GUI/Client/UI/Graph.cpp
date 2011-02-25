// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

// Qt header
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QSvgGenerator>
#include <QToolBar>
#include <QToolButton>
#include <QLineEdit>
#include <QPushButton>
#include <QDoubleValidator>

// Qwt headers
#include "qwt/qwt_picker.h"
#include "qwt/qwt_plot_canvas.h"
#include "qwt/qwt_plot_panner.h"
#include "qwt/qwt_plot_zoomer.h"
#include "qwt/qwt_double_rect.h"
#include "qwt/qwt_scale_widget.h"
#include "qwt/qwt_polygon.h"

//header
#include "GUI/Client/Core/NPlotXY.hpp"
#include "GUI/Client/Core/NLog.hpp"
#include "GUI/Client/UI/PixMaps.hpp"
#include "GUI/Client/UI/BodePlot.hpp"
#include "GUI/Client/UI/Graph.hpp"

using namespace CF::GUI::ClientCore;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientUI {

////////////////////////////////////////////////////////////////////////////////

  //class that manage qwt zoom
  class Zoomer: public QwtPlotZoomer
  {
  public:
    Zoomer(int xAxis, int yAxis, QwtPlotCanvas *canvas):
        QwtPlotZoomer(xAxis, yAxis, canvas)
    {
      //if draged the mouse, get a canvas, if double clicking
      //use the whole curent view as canvas
      setSelectionFlags(QwtPicker::DragSelection | QwtPicker::CornerToCorner);
      setTrackerMode(QwtPicker::AlwaysOff);
      setRubberBand(QwtPicker::NoRubberBand);
      setMaxStackDepth(10); //max zoom

      // RightButton: zoom out by 1
      // Ctrl+RightButton: zoom out to full size
      setMousePattern(QwtEventPattern::MouseSelect2,
                      Qt::RightButton, Qt::ControlModifier);

      setMousePattern(QwtEventPattern::MouseSelect3,
                      Qt::RightButton);
    }
  };


  Graph::Graph(QWidget *parent) :
      QWidget(parent)
  {

    ////creation phase

    //creat a grid layout
    QGridLayout * layout_grid = new QGridLayout();

    //creat layout
    QHBoxLayout * layout_h = new QHBoxLayout();
    QHBoxLayout * layout_h2 = new QHBoxLayout();
    QHBoxLayout * layout_h3 = new QHBoxLayout();

    QHBoxLayout * layout_zoom = new QHBoxLayout();
    QHBoxLayout * layout_zoom_option_point = new QHBoxLayout();
    QHBoxLayout * layout_zoom_option_size = new QHBoxLayout();

    QVBoxLayout * layout_zoom_option = new QVBoxLayout();

    QHBoxLayout * layout_option = new QHBoxLayout();

    //creat the BodePlot
    m_plot = new BodePlot(this,true); //
    m_plot->setMargin(5);

    //create the toolbar that contain the widget's button
    QToolBar * tool_bar = new QToolBar(this);

    //cearte a zoom button
    /*
    QToolButton * btn_zoom = new QToolButton(tool_bar);
    btn_zoom->setText("Zoom");
    btn_zoom->setIcon(QIcon(zoom_xpm));
    btn_zoom->setCheckable(true);
    btn_zoom->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    */


    //create a SVG sae button
    QToolButton * btn_svg = new QToolButton(tool_bar);
    btn_svg->setText("SVG");
    btn_svg->setIcon(QIcon(print_xpm));
    btn_svg->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    //creating label to display information
    m_label_bottom = new QLabel(this);

    //set scale inline system
    m_line_min_x = new QLineEdit();
    m_line_min_x->setValidator(new QDoubleValidator(nullptr));
    m_line_max_x = new QLineEdit();
    m_line_max_x->setValidator(new QDoubleValidator(nullptr));
    m_line_min_y = new QLineEdit();
    m_line_min_y->setValidator(new QDoubleValidator(nullptr));
    m_line_max_y = new QLineEdit();
    m_line_max_y->setValidator(new QDoubleValidator(nullptr));
    m_button_set_scale = new QPushButton("Set scale");

    ////Placment and initialisation

    //assign the grid layout to this widget
    this->setLayout(layout_grid);

    //adding 3 horizontal layout to grid layout
    layout_grid->addLayout(layout_h,0,0,1,1);
    layout_grid->addLayout(layout_h2,1,0,1,1);
    layout_grid->addLayout(layout_h3,2,0,1,1);
    layout_grid->addLayout(layout_zoom,3,0,1,1);
    layout_grid->addLayout(layout_option,4,0,1,1);


    setContextMenuPolicy(Qt::NoContextMenu);

    //initialising the zoom mode
    m_zoomer = new Zoomer( QwtPlot::xBottom, QwtPlot::yLeft,
                              m_plot->canvas());
    m_zoomer->setRubberBand(QwtPicker::RectRubberBand);
    m_zoomer->setRubberBandPen(QColor(Qt::green));
    m_zoomer->setTrackerMode(QwtPicker::ActiveOnly);
    m_zoomer->setTrackerPen(QColor(Qt::white));


    //assign the canvas to the panner
    m_panner = new QwtPlotPanner(m_plot->canvas());
    m_panner->setMouseButton(Qt::MidButton);
    m_panner->setEnabled(true);

    //set the mouse over canvas style
    m_picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                                 QwtPicker::PointSelection | QwtPicker::DragSelection,
                                 QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
                                 m_plot->canvas());
    m_picker->setRubberBandPen(QColor(Qt::green));
    m_picker->setRubberBand(QwtPicker::CrossRubberBand);
    m_picker->setTrackerPen(QColor(Qt::black));

    //adding buttons to the toolbar
    //tool_bar->addWidget(btn_zoom);
    tool_bar->addWidget(btn_svg);

    //set the mouse to not be used for zoom
    enable_zoom_mode(false);

    //display first information ( default information )
    show_info();

    //adding widget to layout to make the composant visible and well placed
    layout_h->addWidget(tool_bar);
    layout_h2->addWidget(m_plot);
    layout_h3->addWidget(m_label_bottom);


    layout_zoom_option_point->addWidget(new QLabel("x min"));
    layout_zoom_option_point->addWidget(m_line_min_x);
    layout_zoom_option_point->addWidget(new QLabel("x max"));
    layout_zoom_option_point->addWidget(m_line_max_x);
    layout_zoom_option_size->addWidget(new QLabel("y min"));
    layout_zoom_option_size->addWidget(m_line_min_y);
    layout_zoom_option_size->addWidget(new QLabel("y max"));
    layout_zoom_option_size->addWidget(m_line_max_y);

    layout_zoom_option->addLayout(layout_zoom_option_point);
    layout_zoom_option->addLayout(layout_zoom_option_size);

    layout_zoom->addLayout(layout_zoom_option);
    layout_zoom->addWidget(m_button_set_scale);

    NPlotXY::PlotDataPtr plot_data( new NPlotXY::PlotData() );
    std::vector<QString> vector_temp2(0);
    graph_option = new GraphOption(plot_data,vector_temp2,m_plot);

    layout_option->addWidget(graph_option);

    ////Conncetion phase
    connect(btn_svg, SIGNAL(clicked()), SLOT(export_svg()));
    //connect(btn_zoom, SIGNAL(toggled(bool)), SLOT(enable_zoom_mode(bool)));
    connect(m_picker, SIGNAL(moved(const QPoint &)),
            SLOT(moved(const QPoint &)));
    connect(m_picker, SIGNAL(selected(const QwtPolygon &)),
            SLOT(selected(const QwtPolygon &)));
    connect(m_button_set_scale, SIGNAL(clicked()), SLOT(set_scale()));
    connect(m_line_min_x, SIGNAL(editingFinished()), SLOT(set_scale()));
    connect(m_line_max_x, SIGNAL(editingFinished()), SLOT(set_scale()));
    connect(m_line_min_y, SIGNAL(editingFinished()), SLOT(set_scale()));
    connect(m_line_max_y, SIGNAL(editingFinished()), SLOT(set_scale()));



    ////Connection Boost
    NPlotXYNotifier::instance().notify_history.connect(
        boost::bind(&Graph::set_xy_data, this, _1, _2) );

  }

  Graph::~Graph(){
    // just in case, but all Qt element will be destroyed proprely by the parent
    /*
    if(m_plot != 0){
      delete m_plot;
      m_plot = 0;
    }

    if(m_label_bottom != 0){
      delete m_label_bottom;
      m_label_bottom = 0;
    }

    if(m_zoomer != 0){
      delete[] m_zoomer;
      m_zoomer[0] = 0;
      m_zoomer[1] = 0;
    }

    if(m_picker != 0){
      delete m_picker;
      m_picker = 0;
    }
    if(m_panner != 0){
      delete m_panner;
      m_panner = 0;
    }
    */
  }

  void Graph::export_svg()
  {
    //default file name
    QString file_name = "bode.svg";

    //getting the file name and path for saving file
    file_name = QFileDialog::getSaveFileName(
        this, "Export File Name", QString(),
        "SVG Documents (*.svg)");

    if ( !file_name.isEmpty() )
    {
      //create generator with file spec
      QSvgGenerator generator;
      generator.setFileName(file_name);
      generator.setSize(QSize(800, 600));
      m_plot->print(generator);
    }
  }

  void Graph::enable_zoom_mode(bool on)
  {
    m_curently_zooming = on;

    m_zoomer->setEnabled(on);
    //m_zoomer[0]->zoom(0);

    m_picker->setEnabled(!on);

    show_info();
  }


  void Graph::show_info(QString text)
  {

    if ( text == QString::null )
    {
      if ( m_picker->isEnabled() )
      {
        text = "Cursor Pos: Press left mouse button in plot region";
      }
      else
      {
        text = "Zoom: Press mouse button and drag";
        text += " / Maximum zoom = ";
        text.append(QString("%1").arg(m_zoomer->maxStackDepth()));
        text += " / Current zoom = ";
        text.append(QString("%1").arg(m_zoomer->zoomRectIndex()));
      }

      text += " / Current zoom x = ";
      m_plot->updateAxes();
      text.append(QString("%1").arg((m_plot->axisScaleDiv(QwtPlot::xBottom)->lowerBound())));
      text += " / ";
      text.append(QString("%1").arg((m_plot->axisScaleDiv(QwtPlot::xBottom)->upperBound())));
      text += " / y = ";
      text.append(QString("%1").arg((m_plot->axisScaleDiv(QwtPlot::xBottom)->lowerBound())));
      text += " / ";
      text.append(QString("%1").arg((m_plot->axisScaleDiv(QwtPlot::xBottom)->upperBound())));
    }

    m_label_bottom->setText(text);
  }


  void Graph::moved(const QPoint &pos)
  {
/*
    if(m_curently_zooming)
    {
      if(m_zoom_point[0] == 0)
      {
        m_zoom_point[0] = new QwtDoublePoint(pos.x(),pos.y());
      }
      if(m_zoom_point[1] == 0)
      {
        m_plot->setAxisScale(QwtPlot::xBottom,m_zoom_point[0]->x(),pos.x());
        m_plot->setAxisScale(QwtPlot::yLeft,m_zoom_point[0]->y(),pos.y());

      }
    }
*/
    QString info;
    info.sprintf("X=%g, Y=%g",
        m_plot->invTransform(QwtPlot::xBottom, pos.x()),
        m_plot->invTransform(QwtPlot::yLeft, pos.y())
    );
    show_info(info);
  }

  void Graph::selected(const QwtPolygon & poly)
  {
    show_info();
  }

  void Graph::set_xy_data(NPlotXY::PlotDataPtr & fcts,
                          std::vector<QString> & fct_label){

    cf_assert( is_not_null(m_plot) );
    graph_option->set_data(fcts,fct_label);

    show_info();
  }

  void Graph::add_xy_data(std::vector< std::vector<double> > & fcts){

  }

  void Graph::set_scale(){

    //this provide full support of double input and give coherant result
    /*
    if(m_line_min_x->text().isEmpty() || m_line_max_x->text().isEmpty() ||
       m_line_min_y->text().isEmpty() || m_line_max_y->text().isEmpty())
    {
      ClientCore::NLog::globalLog()->addError("One or more Scale value are empty.");
      return;
    }
    */

    double x,y,weight,height;

    if(m_line_min_x->text().isEmpty())
    {
      x = m_plot->axisScaleDiv(QwtPlot::xBottom)->lowerBound();
    }else{
      x = m_line_min_x->text().toDouble();
    }

    if(m_line_max_x->text().isEmpty())
    {
      weight = m_plot->axisScaleDiv(QwtPlot::xBottom)->upperBound();
    }else{
      weight = m_line_max_x->text().toDouble();
    }


    if(m_line_min_y->text().isEmpty())
    {
      y = m_plot->axisScaleDiv(QwtPlot::yLeft)->lowerBound();
    }else{
      y = m_line_min_y->text().toDouble();
    }

    if(m_line_max_y->text().isEmpty())
    {
      height = m_plot->axisScaleDiv(QwtPlot::yLeft)->upperBound();
    }else{
      height = m_line_max_y->text().toDouble();
    }

    m_plot->setAxisScale(QwtPlot::xBottom,x,weight);
    m_plot->setAxisScale(QwtPlot::yLeft,y,height);

    m_plot->replot();

     show_info();
  }



////////////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF

