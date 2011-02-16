// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QSvgGenerator>
#include <QToolBar>
#include <QToolButton>

// Qwt headers
#include "qwt/qwt_picker.h"
#include "qwt/qwt_plot_canvas.h"
#include "qwt/qwt_plot_panner.h"
#include "qwt/qwt_plot_zoomer.h"

#include "GUI/Client/UI/PixMaps.hpp"
#include "GUI/Client/UI/BodePlot.hpp"

#include "GUI/Client/UI/Graph.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientUI {

////////////////////////////////////////////////////////////////////////////////

  class Zoomer: public QwtPlotZoomer
  {
  public:
    Zoomer(int xAxis, int yAxis, QwtPlotCanvas *canvas):
        QwtPlotZoomer(xAxis, yAxis, canvas)
    {
      setSelectionFlags(QwtPicker::DragSelection | QwtPicker::CornerToCorner);
      setTrackerMode(QwtPicker::AlwaysOff);
      setRubberBand(QwtPicker::NoRubberBand);

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
    this->setLayout(new QGridLayout());

    QHBoxLayout * layoutH = new QHBoxLayout();
    QHBoxLayout * layoutH2 = new QHBoxLayout();
    QHBoxLayout * layoutH3 = new QHBoxLayout();


    ((QGridLayout *)this->layout())->addLayout(layoutH,0,0,1,1);
    ((QGridLayout *)this->layout())->addLayout(layoutH2,1,0,1,1);
    ((QGridLayout *)this->layout())->addLayout(layoutH3,2,0,1,1);


    d_plot = new BodePlot(this,true);
    d_plot->setMargin(5);

    setContextMenuPolicy(Qt::NoContextMenu);

    d_zoomer[0] = new Zoomer( QwtPlot::xBottom, QwtPlot::yLeft,
                              d_plot->canvas());
    d_zoomer[0]->setRubberBand(QwtPicker::RectRubberBand);
    d_zoomer[0]->setRubberBandPen(QColor(Qt::green));
    d_zoomer[0]->setTrackerMode(QwtPicker::ActiveOnly);
    d_zoomer[0]->setTrackerPen(QColor(Qt::white));

    d_zoomer[1] = new Zoomer(QwtPlot::xTop, QwtPlot::yRight,
                             d_plot->canvas());

    d_panner = new QwtPlotPanner(d_plot->canvas());
    d_panner->setMouseButton(Qt::MidButton);

    d_picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                                 QwtPicker::PointSelection | QwtPicker::DragSelection,
                                 QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
                                 d_plot->canvas());
    d_picker->setRubberBandPen(QColor(Qt::green));
    d_picker->setRubberBand(QwtPicker::CrossRubberBand);
    d_picker->setTrackerPen(QColor(Qt::black));

    //setCentralWidget(d_plot);
    //this->layout()->addWidget(d_plot);
    layoutH2->addWidget(d_plot);


    QToolBar *toolBar = new QToolBar(this);

    QToolButton *btnZoom = new QToolButton(toolBar);

    btnZoom->setText("Zoom");
    btnZoom->setIcon(QIcon(zoom_xpm));
    btnZoom->setCheckable(true);
    btnZoom->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

#ifdef QT_SVG_LIB
    QToolButton *btnSVG = new QToolButton(toolBar);
    btnSVG->setText("SVG");
    btnSVG->setIcon(QIcon(print_xpm));
    btnSVG->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
#endif

    toolBar->addWidget(btnZoom);

#ifdef QT_SVG_LIB
    toolBar->addWidget(btnSVG);
#endif

    layoutH->addWidget(toolBar);

    labelBottom = new QLabel(this);

    layoutH3->addWidget(labelBottom);

    /*
  #ifndef QT_NO_STATUSBAR
    (void)statusBar();
  #endif
  */
    enableZoomMode(false);
    showInfo();

#ifdef QT_SVG_LIB
    connect(btnSVG, SIGNAL(clicked()), SLOT(exportSVG()));
#endif

    connect(btnZoom, SIGNAL(toggled(bool)), SLOT(enableZoomMode(bool)));

    connect(d_picker, SIGNAL(moved(const QPoint &)),
            SLOT(moved(const QPoint &)));
    connect(d_picker, SIGNAL(selected(const QwtPolygon &)),
            SLOT(selected(const QwtPolygon &)));
  }


  void Graph::exportSVG()
  {
    QString fileName = "bode.svg";

#ifdef QT_SVG_LIB
#ifndef QT_NO_FILEDIALOG
    fileName = QFileDialog::getSaveFileName(
        this, "Export File Name", QString(),
        "SVG Documents (*.svg)");
#endif
    if ( !fileName.isEmpty() )
    {
      QSvgGenerator generator;
      generator.setFileName(fileName);
      generator.setSize(QSize(800, 600));

      d_plot->print(generator);
    }
#endif
  }

  void Graph::enableZoomMode(bool on)
  {
    d_panner->setEnabled(on);

    d_zoomer[0]->setEnabled(on);
    d_zoomer[0]->zoom(0);

    d_zoomer[1]->setEnabled(on);
    d_zoomer[1]->zoom(0);

    d_picker->setEnabled(!on);

    showInfo();
  }


  void Graph::showInfo(QString text)
  {

    if ( text == QString::null )
    {
      if ( d_picker->rubberBand() )
        text = "Cursor Pos: Press left mouse button in plot region";
      else
        text = "Zoom: Press mouse button and drag";
    }

    labelBottom->setText(text);

    /*
  #ifndef QT_NO_STATUSBAR
      statusBar()->showMessage(text);
  #endif
      */
  }


  void Graph::moved(const QPoint &pos)
  {
    QString info;
    info.sprintf("Freq=%g, Ampl=%g, Phase=%g",
                 d_plot->invTransform(QwtPlot::xBottom, pos.x()),
                 d_plot->invTransform(QwtPlot::yLeft, pos.y()),
                 d_plot->invTransform(QwtPlot::yRight, pos.y())
                 );
    showInfo(info);
  }

  void Graph::selected(const QwtPolygon &)
  {
    showInfo();
  }

////////////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF

