// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

// Qt headers
#include <QCheckBox>
#include <QComboBox>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>

#include <QDebug>

// Qwt headers
#include <qwt/qwt_plot_curve.h>

// headers
#include "GUI/Client/UI/GraphOption.hpp"
#include "GUI/Client/UI/ColorSelector.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientUI {

////////////////////////////////////////////////////////////////////////////////


  GraphOption::GraphOption(std::vector< std::vector<double> > & fcts,
                           QwtPlot * ptr_plot,QWidget *parent) :
          QWidget(parent)
  {
      m_ptr_plot = ptr_plot;
      m_ptr_fcts = fcts;

      QVBoxLayout * verticalLayout = new QVBoxLayout();
      this->setLayout(verticalLayout);

      //creating and initialising the option table with 5 columns
      m_tableau = new QTableWidget(0,5,this);

      //labels of the columns
      QStringList tableau_labels;
      tableau_labels.push_back(QString(""));
      tableau_labels.push_back(QString("Labels"));
      tableau_labels.push_back(QString("X or Y"));
      tableau_labels.push_back(QString("Color"));
      tableau_labels.push_back(QString("Line Type"));

      //seting the columns labels to he table
      m_tableau->setHorizontalHeaderLabels(tableau_labels);

      //selection type
      m_tableau->setSelectionMode(QAbstractItemView::SingleSelection);

      //adding the table to the vertical label
      verticalLayout->addWidget(m_tableau);

      //adding row for eatch function
      m_tableau->setRowCount(m_ptr_fcts.size());

      QPushButton * boutonDraw = new QPushButton("Draw fonction(s)");

      for(int i = 0; i < m_tableau->rowCount(); ++i ){
          m_tableau->setCellWidget(i,0,new QCheckBox());
          m_tableau->setCellWidget(i,1,new QLineEdit("label")); //to do
          m_tableau->setCellWidget(i,2,new QComboBox());
          ((QComboBox *)m_tableau->cellWidget(i,2))->addItem("y",0);
          ((QComboBox *)m_tableau->cellWidget(i,2))->addItem("x",1);
          m_tableau->setCellWidget(i,4,new QComboBox());
          ((QComboBox *)m_tableau->cellWidget(i,4))->addItem("Lines",1);
          ((QComboBox *)m_tableau->cellWidget(i,4))->addItem("Sticks",2);
          ((QComboBox *)m_tableau->cellWidget(i,4))->addItem("Steps",3);
          ((QComboBox *)m_tableau->cellWidget(i,4))->addItem("Dots",4);
          //((QComboBox *)tableau->cellWidget(i,4))->addItem("dotted line",5);
          m_tableau->setCellWidget(i,3,new ColorSelector());
      }

      verticalLayout->addWidget(boutonDraw);

      connect (boutonDraw, SIGNAL (clicked()), this, SLOT (draw_action()));

  }


  void GraphOption::draw_action(){

      m_ptr_plot->clear(); //detache all curve and clear the legend
      m_ptr_plot->replot(); //remove the drawed curves

      if(find_x() == -1){ //if no X set or X line not checked
          QMessageBox::information(this,"Missing argument",
                                   "Pleas chose ONE X axes and check it's line",0);
      }else{
          for(int i = 0; i < m_tableau->rowCount(); ++i ){
              if(((QCheckBox *)m_tableau->cellWidget(i,0))->isChecked() && (i!=find_x())){
                  ////new curve
                  QwtPlotCurve * temp =
                      new QwtPlotCurve(((QLineEdit *)m_tableau->cellWidget(i,1))->text());
                  //temp->setRenderHint(QwtPlotItem::RenderAntialiased);
                  temp->setPen(QPen(((ColorSelector *)m_tableau->cellWidget(i,3))->
                                    get_color()));
                  temp->setStyle((QwtPlotCurve::CurveStyle)
                                 (((QComboBox *)m_tableau->cellWidget(i,4))->
                                 currentIndex()+1));
                  temp->setData(&m_ptr_fcts[find_x()][0],
                               &m_ptr_fcts[i][0],
                               m_ptr_fcts[i].size());

                  temp->attach(m_ptr_plot);

                  m_ptr_plot->replot();
              }
          }
      }
      m_ptr_plot->setAxisAutoScale(QwtPlot::xBottom);
      m_ptr_plot->setAxisAutoScale(QwtPlot::yLeft);
  }

  //the first checked X axis line is returned
  int GraphOption::find_x(){
      for(int i = 0; i < m_tableau->rowCount(); ++i ){
          if((((QComboBox *)m_tableau->cellWidget(i,2))->
              itemData(((QComboBox *)m_tableau->cellWidget(i,2))->currentIndex()) == 1) &&
             (((QCheckBox *)m_tableau->cellWidget(i,0))->isChecked())){
              return i;
          }
      }
      return -1;
  }

  void GraphOption::set_data(std::vector< std::vector<double> > & fcts){
    m_ptr_fcts = fcts;

    //adding row for eatch function
    m_tableau->setRowCount(m_ptr_fcts.size());

    QPushButton * boutonDraw = new QPushButton("Draw fonction(s)");

    for(int i = 0; i < m_tableau->rowCount(); ++i ){
        m_tableau->setCellWidget(i,0,new QCheckBox());
        m_tableau->setCellWidget(i,1,new QLineEdit("label")); //to do
        m_tableau->setCellWidget(i,2,new QComboBox());
        ((QComboBox *)m_tableau->cellWidget(i,2))->addItem("y",0);
        ((QComboBox *)m_tableau->cellWidget(i,2))->addItem("x",1);
        m_tableau->setCellWidget(i,4,new QComboBox());
        ((QComboBox *)m_tableau->cellWidget(i,4))->addItem("Lines",1);
        ((QComboBox *)m_tableau->cellWidget(i,4))->addItem("Sticks",2);
        ((QComboBox *)m_tableau->cellWidget(i,4))->addItem("Steps",3);
        ((QComboBox *)m_tableau->cellWidget(i,4))->addItem("Dots",4);
        //((QComboBox *)tableau->cellWidget(i,4))->addItem("dotted line",5);
        m_tableau->setCellWidget(i,3,new ColorSelector());
    }
  }

////////////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF
