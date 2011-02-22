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
#include "qwt/qwt_plot_curve.h"

// headers
#include "GUI/Client/Core/NLog.hpp"
#include "GUI/Client/UI/GraphOption.hpp"
#include "GUI/Client/UI/ColorSelector.hpp"
#include "fparser/fparser.hh"
#include "GUI/Client/Core/ClientRoot.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientUI {

////////////////////////////////////////////////////////////////////////////////


  GraphOption::GraphOption(std::vector< std::vector<double> > & fcts,
                           std::vector<QString> & fct_label,
                           QwtPlot * ptr_plot,QWidget *parent) :
  //
          QWidget(parent)
  {
      m_ptr_plot = ptr_plot;

      QVBoxLayout * verticalLayout = new QVBoxLayout();
      this->setLayout(verticalLayout);
      QHBoxLayout * horisontal_function_layout = new QHBoxLayout();

      //creating and initialising the option table with 5 columns
      m_tableau = new QTableWidget(0,6,this);

      //labels of the columns
      QStringList tableau_labels;
      tableau_labels.push_back(QString(""));
      tableau_labels.push_back(QString("Labels"));
      tableau_labels.push_back(QString("X or Y"));
      tableau_labels.push_back(QString("Color"));
      tableau_labels.push_back(QString("Line Type"));
      tableau_labels.push_back(QString("Generated Function"));

      //seting the columns labels to he table
      m_tableau->setHorizontalHeaderLabels(tableau_labels);

      //selection type
      m_tableau->setSelectionMode(QAbstractItemView::SingleSelection);

      //adding the table to the vertical label
      verticalLayout->addWidget(m_tableau);

      QPushButton * button_draw = new QPushButton("Draw fonction(s)");

      in_line_function_name = new QLineEdit();
      in_line_function = new QLineEdit();

      QPushButton * button_generate_function = new QPushButton("Generate Function");

      horisontal_function_layout->addWidget(in_line_function_name);
      horisontal_function_layout->addWidget(in_line_function);
      horisontal_function_layout->addWidget(button_generate_function);

      set_data(fcts,fct_label);

      verticalLayout->addWidget(button_draw);

      verticalLayout->addLayout(horisontal_function_layout);

      connect (button_draw, SIGNAL (clicked()), this, SLOT (draw_action()));
      connect (button_generate_function, SIGNAL (clicked()), this, SLOT (generate_function()));


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
                      new QwtPlotCurve(((QLabel *)m_tableau->cellWidget(i,1))->text());
                  //temp->setRenderHint(QwtPlotItem::RenderAntialiased);
                  temp->setPen(QPen(((ColorSelector *)m_tableau->cellWidget(i,3))->
                                    get_color()));
                  temp->setStyle((QwtPlotCurve::CurveStyle)
                                 (((QComboBox *)m_tableau->cellWidget(i,4))->
                                 currentIndex()+1));
                  temp->setData(&m_fcts[find_x()][0],
                               &m_fcts[i][0],
                               m_fcts[i].size());

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

  void GraphOption::set_data(std::vector< std::vector<double> > & fcts,
                             std::vector<QString> & fct_label){
    m_fcts = fcts;

    //adding row for eatch function
    m_tableau->setRowCount(m_fcts.size());

    for(int i = 0; i < m_tableau->rowCount(); ++i ){
        m_tableau->setCellWidget(i,0,new QCheckBox());
        m_tableau->setCellWidget(i,1,new QLabel(fct_label[i]));
        m_tableau->setCellWidget(i,2,new QComboBox());
        ((QComboBox *)m_tableau->cellWidget(i,2))->addItem("y",0);
        ((QComboBox *)m_tableau->cellWidget(i,2))->addItem("x",1);
        m_tableau->setCellWidget(i,3,new ColorSelector());
        m_tableau->setCellWidget(i,4,new QComboBox());
        ((QComboBox *)m_tableau->cellWidget(i,4))->addItem("Lines",1);
        ((QComboBox *)m_tableau->cellWidget(i,4))->addItem("Sticks",2);
        ((QComboBox *)m_tableau->cellWidget(i,4))->addItem("Steps",3);
        ((QComboBox *)m_tableau->cellWidget(i,4))->addItem("Dots",4);
        //((QComboBox *)tableau->cellWidget(i,4))->addItem("dotted line",5);
        m_tableau->setCellWidget(i,5,new QLabel());
    }
  }


  void GraphOption::add_data(std::vector<double> & fct, QString formule,
                             QString function_name){

    m_fcts.push_back(fct);

    //adding row for eatch function
    int old_row_count = m_tableau->rowCount();
    m_tableau->setRowCount(1 + old_row_count);

    for(int i = m_tableau->rowCount()-1; i < m_tableau->rowCount(); ++i ){
      m_tableau->setCellWidget(i,0,new QCheckBox());
      m_tableau->setCellWidget(i,1,new QLabel(function_name));
      m_tableau->setCellWidget(i,2,new QComboBox());
      ((QComboBox *)m_tableau->cellWidget(i,2))->addItem("y",0);
      ((QComboBox *)m_tableau->cellWidget(i,2))->addItem("x",1);
      m_tableau->setCellWidget(i,3,new ColorSelector());
      m_tableau->setCellWidget(i,4,new QComboBox());
      ((QComboBox *)m_tableau->cellWidget(i,4))->addItem("Lines",1);
      ((QComboBox *)m_tableau->cellWidget(i,4))->addItem("Sticks",2);
      ((QComboBox *)m_tableau->cellWidget(i,4))->addItem("Steps",3);
      ((QComboBox *)m_tableau->cellWidget(i,4))->addItem("Dots",4);
      //((QComboBox *)tableau->cellWidget(i,4))->addItem("dotted line",5);
      m_tableau->setCellWidget(i,5,new QLabel(formule));
    }
  }

  void  GraphOption::generate_function(){

    FunctionParser fparser;

    QString variable = "";
    for(int i=0; i < m_fcts.size(); ++i){
      if(!(((QLabel *)m_tableau->cellWidget(i,1))->text()).isEmpty()){
        if(!variable.isEmpty()){
          variable += ",";
        }
        variable.append(((QLabel *)m_tableau->cellWidget(i,1))->text());
      }
    }

    int res = fparser.Parse(in_line_function->text().toStdString().c_str(),
                            variable.toStdString().c_str());

    if(res > 0){
      ClientCore::NLog::globalLog()->addError("The funtion is not reconized.");
      return;
    }


    int max_it = 0;
    if(m_fcts.size()>0)
    {
      max_it = m_fcts[0].size();
      for(int i=1;i<m_fcts.size();++i)
      {
        max_it = std::min((int)max_it,(int)m_fcts[i].size());
      }
    }else{
      return;
    }

    std::vector<double> vector_temp(max_it);

    double vals[m_fcts.size()];
    for(int i=0;i<max_it;++i)
    {

      for(int j=0;j<m_fcts.size();++j)
      {
        vals[j] = m_fcts[j][i];
      }

      vector_temp[i] = fparser.Eval(vals);
    }

    this->add_data(vector_temp,in_line_function->text(),in_line_function_name->text());

  }


////////////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF
