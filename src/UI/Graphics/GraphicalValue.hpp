// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Graphics_GraphicalValue_hpp
#define cf3_GUI_Graphics_GraphicalValue_hpp

////////////////////////////////////////////////////////////////////////////

#include <QVariant>
#include <QWidget>

#include "Common/Option.hpp"

#include "UI/Graphics/LibGraphics.hpp"

class QHBoxLayout;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Graphics {

//////////////////////////////////////////////////////////////////////////

  class Graphics_API GraphicalValue : public QWidget
  {
    Q_OBJECT
  public:

    static GraphicalValue * createFromOption(cf3::common::Option::ConstPtr option,
                                             QWidget * parent = nullptr);

    GraphicalValue(QWidget * parent = 0);

    ~GraphicalValue();

    virtual bool setValue(const QVariant & value) = 0;

    virtual QVariant value() const = 0;

    QString valueString() const;

    QVariant originalValue() const;

    QString originalValueString() const;

    bool isModified() const;

    void commit();

    QString separator() const { return m_separator; }

  signals:

    void valueChanged();

  protected:

    QVariant m_originalValue;

    QHBoxLayout * m_layout;

    bool m_committing;

    QString m_separator;

  }; // class GraphicalValue

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // CF3_GUI_Graphics_GraphicalValut_hpp
