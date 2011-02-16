// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef CF_GUI_Client_UI_BodePlot_hpp
#define CF_GUI_Client_UI_BodePlot_hpp

////////////////////////////////////////////////////////////////////////////////

#include "qwt/qwt_plot.h"
#include "qwt/qwt_plot_curve.h"

#include "Common/CF.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientUI {

////////////////////////////////////////////////////////////////////////////////

  class BodePlot: public QwtPlot
  {
    Q_OBJECT

  public:

  //constructor
      //construteur min
      BodePlot(QWidget *parent);
      //constructeur full
      /*
      BodePlot(QWidget *parent,const QString labelX, const QString labelY, const QString title, const QString fctName, QColor fctColor, QColor background);
      */
      //constructeur mid
      /*
      BodePlot(QWidget *parent,const QString labelX, const QString labelY, const QString title, const QString fctName);
      */
      //constructeur avec vecteur
      /*
      BodePlot(QWidget *parent, vector<double> xs, vector<double> ys, const QString labelX, const QString labelY);
      */
      //construteur test
      BodePlot(QWidget *parent,bool test);


  //fonction
      void setXLabel(QString newLabelX){
        labelX = newLabelX;
        drawGraph();
      }

      void setYLabel(QString newLabelY){
        labelY = newLabelY;
        drawGraph();
      }

      void showXYLabelOnGraph(bool show){
        showXYLabel = show;
        drawGraph();
      }

      void setBackgroundColor(QColor color){
        backgroundColor = color;
        drawGraph();
      }

      void setFctColor(QColor color){
        fctColor = color;
        drawGraph();
      }

      void setFctName(QString newFctName){
        fctName = newFctName;
        drawGraph();
      }

      void showFctNameOnGraph(bool show){
        showFctName = show;
        drawGraph();
      }

      //void setCursorLabelColor(QColor color);

      void setGraphTitle(QString newTitle){
        title = newTitle;
        drawGraph();
      }

      void showTitleOnGraph(bool show){
        showTitle = show;
        drawGraph();
      }

      void showLegendOnGraph(bool show){
        showLegend = show;
        drawGraph();
      }

      void setGridColor(QColor color){
        gridColor = color;
        drawGraph();
      }

      void logaritmicScaleOnGraph(bool logaritmic){
        logaritmicScale = logaritmic;
        drawGraph();
      }

  private:
      QwtPlotCurve *d_crv3;
      void sinus();

      std::vector<double> xs;
      std::vector<double> ys;

      QString labelX;
      QString labelY;
      bool showXYLabel;
      QColor backgroundColor;
      QColor fctColor;
      QString fctName;
      bool showFctName;
      QString title;
      bool showTitle;
      bool showLegend;
      QColor gridColor;
      bool logaritmicScale;

      void drawGraph();

  };

////////////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF

#endif // CF_GUI_Client_UI_BodePlot_hpp
