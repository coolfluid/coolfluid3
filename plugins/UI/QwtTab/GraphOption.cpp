// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

// Qt headers
#include <QCheckBox>
#include <QBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>
#include <QTextStream>
#include <QTableWidget>
#include <QComboBox>

// Qwt headers
#include "qwt/qwt_plot_curve.h"

// headers
#include "UI/Core/NLog.hpp"
#include "UI/QwtTab/GraphOption.hpp"
#include "UI/QwtTab/Graph.hpp"
#include "UI/QwtTab/ColorSelector.hpp"
#include "fparser/fparser.hh"

////////////////////////////////////////////////////////////////////////////////

using namespace CF::UI::Core;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace QwtTab {

////////////////////////////////////////////////////////////////////////////////

GraphOption::GraphOption(QwtPlot * ptr_plot,
                         QWidget *parent) :
QWidget(parent)
{
  m_ptr_plot = ptr_plot;
  m_graph_parent = (Graph *) parent;

  QPointer<QHBoxLayout> horisontal_table_layout = new QHBoxLayout();
  QPointer<QVBoxLayout> vertical_line_table_layout = new QVBoxLayout();
  QPointer<QVBoxLayout> vertical_line_button_layout = new QVBoxLayout();

  QPointer<QGridLayout> grid_data_table_layout = new QGridLayout();

  this->setLayout(horisontal_table_layout);

  //creating and initialising the option table with 5 columns
  m_line_table = new QTableWidget(0,5,this);
  //creating and initialising the data table with 2 columns
  m_data_table = new QTableWidget(0,2,this);

  //add line button
  QPointer<QPushButton> button_add_line = new QPushButton("Add");
  QPointer<QPushButton> button_select_all = new QPushButton("All");
  QPointer<QPushButton> button_clear_selection = new QPushButton("None");
  QPointer<QPushButton> button_remove_line = new QPushButton("Remove");

  //labels of the line_table's columns
  QStringList line_table_labels;
  line_table_labels.push_back(QString(""));
  line_table_labels.push_back(QString("x"));
  line_table_labels.push_back(QString("y"));
  line_table_labels.push_back(QString("Color"));
  line_table_labels.push_back(QString("Line Type"));

  //labels of the data_table's columns
  QStringList data_table_labels;
  data_table_labels.push_back(QString("name"));
  data_table_labels.push_back(QString("function"));

  //seting the columns labels to the table
  m_line_table->setHorizontalHeaderLabels(line_table_labels);
  m_data_table->setHorizontalHeaderLabels(data_table_labels);

  //selection type
  m_line_table->setSelectionMode(QAbstractItemView::MultiSelection);
  m_line_table->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_data_table->setSelectionMode(QAbstractItemView::NoSelection);

  m_button_draw = new QPushButton("Set AutoScale");

  m_line_function_name = new QLineEdit();
  m_line_function = new QLineEdit();

  m_button_generate_function = new QPushButton("Generate Function");

  //set_data(fcts,fcts_label);

  //adding widget & layout to layout
  vertical_line_table_layout->addWidget(m_line_table);
  vertical_line_table_layout->addWidget(m_button_draw);

  QPointer<QLabel> name_fct = new QLabel("Name :");
  QPointer<QLabel> fct_fct = new QLabel("Function :");

  //data part
  grid_data_table_layout->addWidget(m_data_table,0,0,1,2);
  grid_data_table_layout->addWidget(name_fct,1,0,1,1);
  grid_data_table_layout->addWidget(m_line_function_name,1,1,1,1);
  grid_data_table_layout->addWidget(fct_fct,2,0,1,1);
  grid_data_table_layout->addWidget(m_line_function,2,1,1,1);
  grid_data_table_layout->addWidget(m_button_generate_function,3,0,1,2);

  //set size inside the data layout to a small size
  m_data_table->setMaximumWidth(250);
  m_data_table->setColumnWidth(0,70);
  m_data_table->setColumnWidth(1,100);
  name_fct->setMaximumWidth(100);
  m_line_function_name->setMaximumWidth(150);
  fct_fct->setMaximumWidth(100);
  m_line_function->setMaximumWidth(150);
  m_button_generate_function->setMaximumWidth(250);


  //button line part
  vertical_line_button_layout->addWidget(button_add_line);
  vertical_line_button_layout->addWidget(button_select_all);
  vertical_line_button_layout->addWidget(button_clear_selection);
  vertical_line_button_layout->addWidget(button_remove_line);
  m_line_table->setColumnWidth(0,20);
  m_line_table->setColumnWidth(1,60);
  m_line_table->setColumnWidth(2,60);

  //main option layout
  horisontal_table_layout->addLayout(grid_data_table_layout);
  horisontal_table_layout->addLayout(vertical_line_table_layout);
  horisontal_table_layout->addLayout(vertical_line_button_layout);


  //connections
  connect (m_button_draw, SIGNAL (clicked()), this, SLOT (draw_and_resize()));
  connect (m_button_generate_function, SIGNAL (clicked()), this, SLOT (generate_function()));
  connect (button_add_line, SIGNAL (clicked()), this, SLOT (add_line()));
  connect (button_remove_line, SIGNAL (clicked()), this, SLOT (remove_line()));
  connect (button_clear_selection, SIGNAL (clicked()), this, SLOT
           (clear_line_table_selection()));
  connect (button_select_all, SIGNAL (clicked()), this, SLOT
           (select_all_line_table()));
}


void GraphOption::draw_action(){

  if(m_can_draw){

    m_can_draw = false;

    m_button_draw->setEnabled(false);

    m_ptr_plot->clear(); //detache all curve and clear the legend

    for(int i = 0; i < m_line_table->rowCount(); ++i ){

      if(((QCheckBox *)m_line_table->cellWidget(i,0))->isChecked()){
        ////new curve
        QwtPlotCurve * new_curve =
            new QwtPlotCurve(((QComboBox *)m_line_table->cellWidget(i,2))->
                             currentText());

        new_curve->setPen(QPen(((ColorSelector *)m_line_table->cellWidget(i,3))->
                               get_color()));

        new_curve->setStyle((QwtPlotCurve::CurveStyle)
                            (((QComboBox *)m_line_table->cellWidget(i,4))->
                             currentIndex()+1));

        new_curve->setData(&(*m_fcts)[0][((QComboBox *)m_line_table->cellWidget(i,1))->
                                         currentIndex()],
                           &(*m_fcts)[0][((QComboBox *)m_line_table->cellWidget(i,2))->
                                         currentIndex()],
                           m_fcts->size());

        new_curve->attach(m_ptr_plot);
      }

    }

    m_ptr_plot->replot();

    m_button_draw->setEnabled(true);

    m_can_draw = true;

    m_graph_parent->reset_base_zoom();

  }
}

void GraphOption::draw_and_resize()
{

  //force draw autorisation
  m_can_draw = true;

  //reinitialise the axis
  m_ptr_plot->setAxisAutoScale(QwtPlot::xBottom);
  m_ptr_plot->setAxisAutoScale(QwtPlot::yLeft);

  //draw
  draw_action();

}

void GraphOption::set_data(NPlotXY::PlotDataPtr & fcts,
                           std::vector<QString> & fcts_label)
{

  m_can_draw = false;

  //saving user's function
  std::vector<QString> user_fct_name;
  std::vector<QString> user_fct_fct;

  for(int i=0;i<m_data_table->rowCount();++i)
  {
    if(!(((QLabel *)m_data_table->cellWidget(i,1))->text()).isEmpty())
    {
      user_fct_name.push_back(((QLabel *)m_data_table->cellWidget(i,0))->text());
      user_fct_fct.push_back(((QLabel *)m_data_table->cellWidget(i,1))->text());
    }
  }

  //clear functions
  m_fcts = fcts;

  //add # function

  m_data_table->setRowCount(fcts_label.size());

  //adding row for eatch data
  for(int i = 0; i < m_data_table->rowCount(); ++i )
  {
    m_data_table->setCellWidget(i,0,new QLabel(fcts_label[i]));
    m_data_table->setCellWidget(i,1,new QLabel());
  }

  //regenerate existing user function with new values
  for(int i=0;i<user_fct_fct.size();++i)
  {
    generate_function(user_fct_name[i],user_fct_fct[i]);
  }



  //refresh line_table function list
  QStringList function_list;
  for(int i=0;i<m_data_table->rowCount();++i){
    function_list.append(((QLabel *)m_data_table->cellWidget(i,0))->text());
  }



  int current_index = 0;
  for(int i=0;i<m_line_table->rowCount();++i){
    current_index = ((QComboBox *)m_line_table->cellWidget(i,1))->currentIndex();
    ((QComboBox *)m_line_table->cellWidget(i,1))->clear();
    ((QComboBox *)m_line_table->cellWidget(i,1))->addItems(function_list);
    ((QComboBox *)m_line_table->cellWidget(i,1))->setCurrentIndex(current_index);

    current_index = ((QComboBox *)m_line_table->cellWidget(i,2))->currentIndex();
    ((QComboBox *)m_line_table->cellWidget(i,2))->clear();
    ((QComboBox *)m_line_table->cellWidget(i,2))->addItems(function_list);
    ((QComboBox *)m_line_table->cellWidget(i,2))->setCurrentIndex(current_index);
  }

  m_can_draw = true;

  //draw existing line with new data
  draw_action();

  //inform user
  Core::NLog::globalLog()->addMessage("New data set received.");
}


void GraphOption::add_data(std::vector<double> & fct,QString function_name, QString formula)
{

  //resize and add 1 column and add the function
  m_fcts->resize(boost::extents[m_fcts->size()][(*m_fcts)[0].size()+1]);
  for(int i=0;i<m_fcts->size();++i){
    (*m_fcts)[i][(*m_fcts)[0].size()-1] = fct[i];
  }

  //adding one row
  int old_row_count = m_data_table->rowCount();
  m_data_table->setRowCount(1 + old_row_count);

  m_data_table->setCellWidget(old_row_count,0,new QLabel(function_name));
  m_data_table->setCellWidget(old_row_count,1,new QLabel(formula));

  //adding the function to lines x,y lists
  for(int i=0;i<m_line_table->rowCount();++i)
  {
    ((QComboBox *)m_line_table->cellWidget(i,1))->addItem(function_name);
    ((QComboBox *)m_line_table->cellWidget(i,2))->addItem(function_name);
  }
}

void  GraphOption::generate_function(){
  generate_function(m_line_function_name->text(),m_line_function->text());
}

void  GraphOption::generate_function(QString name,QString formula){

  m_button_generate_function->setEnabled(false);


  //ferification of name and formula input
  if(name.isEmpty() || formula.isEmpty()){
    Core::NLog::globalLog()->addError("Please give function's name and formula.");
    m_button_generate_function->setEnabled(true);
    return;
  }

  //check if the function name already exist
  for(int i=0; i < m_data_table->rowCount(); ++i){
    if((((QLabel *)m_data_table->cellWidget(i,0))->text()) == name){
      Core::NLog::globalLog()->addError("The function name already exist.");
      m_button_generate_function->setEnabled(true);
      return;
    }
  }

  FunctionParser fparser;

  //get all fct name separated by ','
  QString variable = "";
  for(int i=1; i < m_data_table->rowCount(); ++i){
    if(!(((QLabel *)m_data_table->cellWidget(i,0))->text()).isEmpty()){
      if(!variable.isEmpty()){
        variable += ",";
      }
      variable.append(((QLabel *)m_data_table->cellWidget(i,0))->text());
    }
  }

  //parsing function
  int res = fparser.Parse(formula.toStdString().c_str(),
                          variable.toStdString().c_str());

  if(res > 0){
    Core::NLog::globalLog()->addError("The function is not recognized.");
    m_button_generate_function->setEnabled(true);
    return;
  }

  //<to remove>
  int max_it = 0;
  if(m_data_table->rowCount()>0)
  {
    max_it = m_fcts->size();
  }else{
    Core::NLog::globalLog()->addError("The function is not recognized.");
    m_button_generate_function->setEnabled(true);
    return;
  }
  //</to remove>

  //generate the new function's data
  std::vector<double> vector_temp(max_it);

  double vals[m_data_table->rowCount()-1];
  for(int i=0;i<max_it;++i)
  {
    for(int j=0;j<m_data_table->rowCount()-1;++j)
    {
      vals[j] = (*m_fcts)[i][j+1];
    }
    vector_temp[i] = fparser.Eval(vals);
  }

  //add the new function
  this->add_data(vector_temp,name,formula);

  //get back the button and clear the name and function line
  m_button_generate_function->setEnabled(true);
  m_line_function->clear();
  m_line_function_name->clear();

}

void GraphOption::add_line(){

  if(m_data_table->rowCount() <= 0){
    Core::NLog::globalLog()->addError("There are no data to set, you cannot add line");
    return;
  }

  //adding row for eatch function
  int old_row_count = m_line_table->rowCount();
  m_line_table->setRowCount(1 + old_row_count);

  QStringList function_list;
  for(int i=0;i<m_data_table->rowCount();++i){
    function_list.append(((QLabel *)m_data_table->cellWidget(i,0))->text());
  }

  m_line_table->setCellWidget(old_row_count,0,new QCheckBox());
  ((QCheckBox *)m_line_table->cellWidget(old_row_count,0))->setFixedWidth(20);

  m_line_table->setCellWidget(old_row_count,1,new QComboBox());
  ((QComboBox *)m_line_table->cellWidget(old_row_count,1))->addItems(function_list);


  m_line_table->setCellWidget(old_row_count,2,new QComboBox());
  ((QComboBox *)m_line_table->cellWidget(old_row_count,2))->addItems(function_list);


  m_line_table->setCellWidget(old_row_count,3,new ColorSelector(old_row_count));


  m_line_table->setCellWidget(old_row_count,4,new QComboBox());
  ((QComboBox *)m_line_table->cellWidget(old_row_count,4))->addItem("Lines",1);
  ((QComboBox *)m_line_table->cellWidget(old_row_count,4))->addItem("Sticks",2);
  ((QComboBox *)m_line_table->cellWidget(old_row_count,4))->addItem("Steps",3);
  ((QComboBox *)m_line_table->cellWidget(old_row_count,4))->addItem("Dots",4);



  connect(((QComboBox *)m_line_table->cellWidget(old_row_count,1)),
          SIGNAL(currentIndexChanged(int)), this,
          SLOT (draw_action()));
  connect(((QComboBox *)m_line_table->cellWidget(old_row_count,2)),
          SIGNAL(currentIndexChanged(int)), this,
          SLOT (draw_action()));
  connect(((QCheckBox *)m_line_table->cellWidget(old_row_count,0)),
          SIGNAL(stateChanged(int)), this, SLOT (checked_changed(int)));
  connect(((ColorSelector *)m_line_table->cellWidget(old_row_count,3)),
          SIGNAL(valueChanged(QColor,int)), this, SLOT (color_changed(QColor,int)));
  connect(((QComboBox *)m_line_table->cellWidget(old_row_count,4)),
          SIGNAL(currentIndexChanged(int)), this, SLOT (line_type_changed(int)));
}

void GraphOption::remove_line()
{
  while( m_line_table->selectionModel()->selectedRows().count() > 0)
  {
    m_line_table->removeRow(m_line_table->selectionModel()->selectedRows().at(0).row());
  }
  for(int i=0; i < m_line_table->rowCount(); ++i){
    ((ColorSelector *)m_line_table->cellWidget(i,3))->set_row(i);
  }
  draw_action();
}


void GraphOption::checked_changed(int check)
{

  for(int i=0;i<m_line_table->selectionModel()->selectedRows().count();++i)
  {
    //disconect checkbox to avoid looping
    disconnect(((QCheckBox *)m_line_table->cellWidget(
        m_line_table->selectionModel()->selectedRows().at(i).row(),0)),
               SIGNAL(stateChanged(int)), this, SLOT (checked_changed(int)));

    //change state
    ((QCheckBox *)m_line_table->cellWidget(
        m_line_table->selectionModel()->selectedRows().at(i).row(),0))->
        setCheckState((Qt::CheckState)check);

    //re-connect after changing state
    connect(((QCheckBox *)m_line_table->cellWidget(
        m_line_table->selectionModel()->selectedRows().at(i).row(),0)),
            SIGNAL(stateChanged(int)), this, SLOT (checked_changed(int)));
  }

  draw_action();
}

void GraphOption::color_changed(QColor color, int raw)
{
  bool redraw = false;

  //if change but no selection done redraw anyway
  if(m_line_table->selectionModel()->selectedRows().count() <= 0){
    if(((QCheckBox *)m_line_table->cellWidget(raw,0))->isChecked()){
      redraw = true;
    }
  }

  for(int i=0;i<m_line_table->selectionModel()->selectedRows().count();++i)
  {
    //disconect checkbox to avoid looping
    disconnect(((ColorSelector *)m_line_table->cellWidget(
        m_line_table->selectionModel()->selectedRows().at(i).row(),3)),
               SIGNAL(valueChanged(QColor,int)), this, SLOT (color_changed(QColor,int)));

    //change state
    ((ColorSelector *)m_line_table->cellWidget(
        m_line_table->selectionModel()->selectedRows().at(i).row(),3))->
        set_color(color);

    //re-connect after changing state
    connect(((ColorSelector *)m_line_table->cellWidget(
        m_line_table->selectionModel()->selectedRows().at(i).row(),3)),
            SIGNAL(valueChanged(QColor,int)), this, SLOT (color_changed(QColor,int)));

    //if one of the changed line is checked, then redraw, otherwise not.
    if(((QCheckBox *)m_line_table->cellWidget(
        m_line_table->selectionModel()->selectedRows().at(i).row(),0))->isChecked()){
      redraw = true;
    }
  }

  if(redraw)
    draw_action();
}

void GraphOption::line_type_changed(int current_index)
{
  bool redraw = false;

  //if change but no selection done redraw anyway
  if(m_line_table->selectionModel()->selectedRows().count() <= 0){
    redraw = true;
  }

  for(int i=0;i<m_line_table->selectionModel()->selectedRows().count();++i)
  {
    //disconect checkbox to avoid looping
    disconnect(((QComboBox *)m_line_table->cellWidget(
        m_line_table->selectionModel()->selectedRows().at(i).row(),4)),
               SIGNAL(currentIndexChanged(int)), this, SLOT (line_type_changed(int)));

    //change state
    ((QComboBox *)m_line_table->cellWidget(
        m_line_table->selectionModel()->selectedRows().at(i).row(),4))->
        setCurrentIndex(current_index);

    //re-connect after changing state
    connect(((QComboBox *)m_line_table->cellWidget(
        m_line_table->selectionModel()->selectedRows().at(i).row(),4)),
            SIGNAL(currentIndexChanged(int)), this, SLOT (line_type_changed(int)));

    //if one of the changed line is checked, then redraw, otherwise not.
    if(((QCheckBox *)m_line_table->cellWidget(
        m_line_table->selectionModel()->selectedRows().at(i).row(),0))->isChecked()){
      redraw = true;
    }
  }

  if(redraw)
    draw_action();
}

void GraphOption::clear_line_table_selection(){
  m_line_table->clearSelection();
}

void GraphOption::select_all_line_table(){
  m_line_table->selectAll();
}

void GraphOption::save_functions(){

  if(m_data_table->rowCount() > 0){

    //the popup dialog box
    QPointer<QDialog> popup_save_to_text = new QDialog(this);

    //the table where we check desired function
    m_choose_table = new QTableWidget(0,2,0);

    //labels of the choose_table's columns
    QStringList choose_table_labels;
    choose_table_labels.push_back(QString(""));
    choose_table_labels.push_back(QString("function name"));

    //seting the columns labels to the table
    m_choose_table->setHorizontalHeaderLabels(choose_table_labels);

    //set numbrer of rows
    m_choose_table->setRowCount(m_data_table->rowCount());

    //adding row for eatch data
    for(int i = 0; i < m_choose_table->rowCount(); ++i )
    {
      m_choose_table->setCellWidget(i,0,new QCheckBox());
      m_choose_table->setCellWidget(i,1,new QLabel(((QLabel *)m_data_table->cellWidget(i,0))->text()));
    }

    //adding Done and Save button
    QPointer<QPushButton> btn_save = new QPushButton("SAVE", popup_save_to_text);
    QPointer<QPushButton> btn_done = new QPushButton("DONE", popup_save_to_text);

    //connect them to their actions
    connect(btn_done, SIGNAL(released()),popup_save_to_text,SLOT(close()));
    //connect(btn_save, SIGNAL(released()),this,SLOT(save_functions_to_file()));
    connect(btn_save, SIGNAL(released()),this,SLOT(save_functions_to_file_no_buffering()));

    //the 2 layout of the popup for nice look & feel
    QPointer<QVBoxLayout> vertical_popup_layout = new QVBoxLayout();
    QPointer<QHBoxLayout> horisontal_popup_layout = new QHBoxLayout();
    popup_save_to_text->setLayout(vertical_popup_layout);
    vertical_popup_layout->addWidget(m_choose_table);
    vertical_popup_layout->addLayout(horisontal_popup_layout);
    horisontal_popup_layout->addWidget(btn_save);
    horisontal_popup_layout->addWidget(btn_done);

    popup_save_to_text->resize(260,220);
    popup_save_to_text->setModal(true);
    popup_save_to_text->show();
  }else{
    Core::NLog::globalLog()->addError("There are no data to save.");
  }
  return;
}


void GraphOption::save_functions_to_file(){

  //

  //generate txt table to save in file
  /***********************************************************/
  QString output;

  for(int i=0;i<m_data_table->rowCount();++i)
  {
    if(((QCheckBox *)m_choose_table->cellWidget(i,0))->isChecked()){
      output += "\"";
      output += ((QLabel *)m_data_table->cellWidget(i,0))->text();
      output += "\" ";
    }
  }

  output += "\n";

  for(int i=0;i<m_fcts->size();++i){
    for(int j=0;j<m_data_table->rowCount();++j){
      if(((QCheckBox *)m_choose_table->cellWidget(j,0))->isChecked()){
        output += "\"";
        output += QString::number((*m_fcts)[i][j]);
        output += "\" ";
      }
    }
    output += "\n";
  }
  /***********************************************************/

  //Save into the file
  /***********************************************************/
  //default file name
  QString file_name = "out.txt";

  //getting the file name and path for saving file
  file_name = QFileDialog::getSaveFileName(
      this, "Export File Name", QString(),
      "txt Documents (*.txt)");

  if ( !file_name.isEmpty() )
  {

    QFile file(file_name);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
      Core::NLog::globalLog()->addError("Unable to open file.");
      return;
    }

    QTextStream out(&file);
    out << output;
    file.close();

    Core::NLog::globalLog()->addMessage("Data saved ...");
    /***********************************************************/
  }
}

////////////////////////////////////////////////////////////////////////////////

void GraphOption::save_functions_to_file_no_buffering(){

  //Save into the file
  /***********************************************************/
  //default file name
  QString file_name = "out.txt";

  //getting the file name and path for saving file
  file_name = QFileDialog::getSaveFileName(
      this, "Export File Name", QString(),
      "txt Documents (*.txt)");

  if ( !file_name.isEmpty() )
  {

    QFile file(file_name);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
      Core::NLog::globalLog()->addError("Unable to open file.");
      return;
    }

    QTextStream out(&file);

    /***********************************************************/;

    for(int i=0;i<m_data_table->rowCount();++i)
    {
      if(((QCheckBox *)m_choose_table->cellWidget(i,0))->isChecked()){
        out <<  "\"";
        out <<  ((QLabel *)m_data_table->cellWidget(i,0))->text();
        out <<  "\" ";
      }
    }

    out <<  "\n";

    for(int i=0;i<m_fcts->size();++i){
      for(int j=0;j<m_data_table->rowCount();++j){
        if(((QCheckBox *)m_choose_table->cellWidget(j,0))->isChecked()){
          out <<  "\"";
          out <<  QString::number((*m_fcts)[i][j]);
          out <<  "\" ";
        }
      }
      out <<  "\n";
    }
    /***********************************************************/
    file.close();

    Core::NLog::globalLog()->addMessage("Data saved ...");
    /***********************************************************/
  }
}

} // Graphics
} // UI
} // CF
