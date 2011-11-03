// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UI_QwtTab_ColorSelector_hpp
#define cf3_UI_QwtTab_ColorSelector_hpp

// Qt header
#include <QLabel>
#include <QMouseEvent>
#include <QColorDialog>

// header
#include "ui/QwtTab/LibQwtTab.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace QwtTab {

////////////////////////////////////////////////////////////////////////////////


/// @brief This is QLabel that display a color picker when clicked and can give this color back.
/// @author Wertz Gil
class QwtTab_API ColorSelector : public QLabel
{
    Q_OBJECT

public: //function
    /// Constructor.
    /// @param parent Parent of this widget.
    ColorSelector(int raw,QWidget * parent = 0);

    /// Return the selected color
    QColor get_color();

    /// Set the color.
    /// @param color The new color.
    void set_color(QColor color);

    /// Set the row number of this colorSelector.
    /// @param row The row number.
    void set_row(int row);

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
    int m_raw;

signals: //signals
    /// Signal emited when the color value has changed.
    /// @param color The new color value.
    void valueChanged(QColor color,int raw);

};

////////////////////////////////////////////////////////////////////////////////

} // QwtTab
} // UI
} // cf3

#endif // cf3_UI_QwtTab_ColorSelector_hpp
