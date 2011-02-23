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

public: //function
    /// Constructor.
    /// @param parent Parent of this widget.
    ColorSelector(QWidget * parent = 0);

    /// Return the selected color
    QColor get_color();

    /// Set the color.
    /// @param color The new color.
    void set_color(QColor color);

private: //function
    /// Set the color choosed as the m_color.
    void set_color();

protected: //function
    /// called when the QLabel is clicked.
    /// @param event The ;ouse event.
    void mousePressEvent ( QMouseEvent * event ) ;

private: //data
    /// The selected color.
    QColor m_color;

signals: //signals
    /// Signal emited when the color value has changed.
    /// @param color The new color value.
    void valueChanged(QColor color);

};

#endif // COLORSELECTOR_HPP
