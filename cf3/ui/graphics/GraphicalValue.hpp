// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_Graphics_GraphicalValue_hpp
#define cf3_ui_Graphics_GraphicalValue_hpp

////////////////////////////////////////////////////////////////////////////

#include <QVariant>
#include <QWidget>

#include "common/Option.hpp"

#include "ui/graphics/LibGraphics.hpp"

class QHBoxLayout;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

//////////////////////////////////////////////////////////////////////////

  class Graphics_API GraphicalValue : public QWidget
  {
    Q_OBJECT
  public:

    static GraphicalValue * create_from_option( boost::shared_ptr< common::Option > option,
                                              QWidget * parent = nullptr );

    GraphicalValue(QWidget * parent = 0);

    ~GraphicalValue();

    virtual bool set_value(const QVariant & value) = 0;

    virtual QVariant value() const = 0;

    QString value_string() const;

    QVariant original_value() const;

    QString original_value_string() const;

    bool is_modified() const;

    void commit();

    QString separator() const { return m_separator; }

  signals:

    void value_changed();

  protected:

    QVariant m_original_value;

    QHBoxLayout * m_layout;

    bool m_committing;

    QString m_separator;

  }; // class GraphicalValue

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_Graphics_GraphicalValut_hpp
