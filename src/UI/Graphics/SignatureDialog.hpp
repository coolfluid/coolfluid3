// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Graphics_SignatureDialog_hpp
#define cf3_GUI_Graphics_SignatureDialog_hpp

////////////////////////////////////////////////////////////////////////////////

#include <QDialog>
#include <QMap>

class QDialogButtonBox;
class QFormLayout;
class QVBoxLayout;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {

namespace common {
  namespace XML { class XmlNode; }
}

namespace UI {
namespace Graphics {

  class OptionLayout;

////////////////////////////////////////////////////////////////////////////////

  class SignatureDialog : public QDialog
  {
    Q_OBJECT

  public:

    SignatureDialog(QWidget *parent = 0);

    ~SignatureDialog();

    bool show(common::XML::XmlNode & sig, const QString & title, bool block = false);

  private slots:

    void btOkClicked();

    void btCancelClicked();

  private:

    QDialogButtonBox * m_buttons;

    OptionLayout * m_dataLayout;

    QVBoxLayout * m_mainLayout;

    QMap<QString, common::XML::XmlNode> m_nodes;

    bool m_okClicked;

    bool m_isBlocking;

  }; // class SignatureDialog

////////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // CF3_GUI_Graphics_SignatureDialog_hpp
