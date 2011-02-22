// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef GRAPHOPTION_HPP
#define GRAPHOPTION_HPP

#include <QTableWidget>
#include <qwt/qwt_plot.h>

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientUI {

////////////////////////////////////////////////////////////////////////////////


class GraphOption : public QWidget
{
    Q_OBJECT
public:

    /// constructor
    GraphOption(std::vector< std::vector<double> > & fcts,
                std::vector<QString> & fct_label,
                QwtPlot * ptr_plot,
                QWidget *parent = 0);

    /// set the data to show in the option tab
    void set_data(std::vector< std::vector<double> > & fcts,
                  std::vector<QString> & fct_label);

    /// add the data to show in the option tab
    void add_data(std::vector<double> & fct, QString formule,
                  QString function_name);

private:
    /// QwtPlot pointer, refer to the plot where we draw cures
    QwtPlot * m_ptr_plot;

    /// the curves data
    std::vector< std::vector<double> > m_fcts;

    /// User tipe his function here
    QLineEdit * in_line_function;

    /// User tipe name funtion
    QLineEdit * in_line_function_name;

    /// cell and item table
    QTableWidget * m_tableau;

    /// function that find whitch data are the referance x
    int find_x();

private slots:

    /// draw curves according to the data and options
    void draw_action();

    /// function that user inserted QString to make a function
    void generate_function();

};

////////////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF

#endif // GRAPHOPTION_HPP
