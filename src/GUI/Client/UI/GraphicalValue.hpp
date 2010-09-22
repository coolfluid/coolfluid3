// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_GraphicalValue_hpp
#define CF_GUI_Client_GraphicalValue_hpp

////////////////////////////////////////////////////////////////////////////

#include <QVariant>
#include <QWidget>

#include "GUI/Client/UI/LibClientUI.hpp"

class QHBoxLayout;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

  //////////////////////////////////////////////////////////////////////////

  class ClientUI_API GraphicalValue : public QWidget
  {
    Q_OBJECT
  public:

    GraphicalValue(QWidget * parent = 0);

    virtual bool setValue(const QVariant & value) = 0;

    virtual QVariant getValue() const = 0;

    QString getValueString() const;

    QVariant getOriginalValue() const;

    QString getOriginalString() const;

    bool isModified() const;

    void commit();

  signals:

    void valueChanged();

  protected:

    QVariant m_originalValue;

    QHBoxLayout * m_layout;

    bool m_committing;

  }; // class GraphicalValue

  //////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_GraphicalValut_hpp
