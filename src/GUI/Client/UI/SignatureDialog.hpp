// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_UI_SignatureDialog_hpp
#define CF_GUI_Client_UI_SignatureDialog_hpp

////////////////////////////////////////////////////////////////////////////////

#include <QDialog>
#include <QMap>

class QDialogButtonBox;
class QFormLayout;
class QVBoxLayout;

////////////////////////////////////////////////////////////////////////////////

namespace CF {

namespace Common {
  namespace XML { class XmlNode; }
}

namespace GUI {
namespace ClientUI {

  class OptionLayout;

////////////////////////////////////////////////////////////////////////////////

  class SignatureDialog : public QDialog
  {
    Q_OBJECT

  public:

    SignatureDialog(QWidget *parent = 0);

    ~SignatureDialog();

    bool show(Common::XML::XmlNode & sig, const QString & title, bool block = false);

  private slots:

    void btOkClicked();

    void btCancelClicked();

  private:

    QDialogButtonBox * m_buttons;

    OptionLayout * m_dataLayout;

    QVBoxLayout * m_mainLayout;

    QMap<QString, Common::XML::XmlNode> m_nodes;

    bool m_okClicked;

  }; // class SignatureDialog

////////////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UI_SignatureDialog_hpp
