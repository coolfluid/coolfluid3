// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_UI_GraphicalValue_hpp
#define CF_GUI_Client_UI_GraphicalValue_hpp

////////////////////////////////////////////////////////////////////////////

#include <QVariant>
#include <QWidget>

#include "Common/Option.hpp"

#include "GUI/Client/UI/LibClientUI.hpp"

class QHBoxLayout;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientUI {

  //////////////////////////////////////////////////////////////////////////

  class ClientUI_API GraphicalValue : public QWidget
  {
    Q_OBJECT
  public:

    static GraphicalValue * createFromOption(CF::Common::Option::ConstPtr option,
                                             QWidget * parent = nullptr);

    static GraphicalValue * createFromXml(const CF::Common::XmlNode & node,
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

  signals:

    void valueChanged();

  protected:

    QWidget * m_parent;

    QVariant m_originalValue;

    QHBoxLayout * m_layout;

    bool m_committing;

  private:

    static GraphicalValue * create(const QString & type, bool isArray, QWidget * parent);

  }; // class GraphicalValue

  //////////////////////////////////////////////////////////////////////////

} // namespace ClientUI
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UI_GraphicalValut_hpp
