// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef COLORSELECTOR_HPP
#define COLORSELECTOR_HPP

#include <QLabel>
#include <QMouseEvent>

class ColorSelector : public QLabel
{
    Q_OBJECT

public:
    /// Constructor
    ColorSelector(QWidget * parent = 0);

    /// Return the selected color
    QColor get_color();

private:
    /// The selected color
    QColor m_color;

    /// Set the color choosed as the m_color
    void set_color();

protected:
    /// called when the QLabel is clicked
    void mousePressEvent ( QMouseEvent * event ) ;
};

#endif // COLORSELECTOR_HPP
