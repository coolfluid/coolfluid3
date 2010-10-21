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

#include "Common/XmlHelpers.hpp"

class QDialogButtonBox;
class QFormLayout;
class QVBoxLayout;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
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

    bool show(CF::Common::XmlNode & sig, const QString & title);

  private slots:

    void btOkClicked();

    void btCancelClicked();

  private:

    QDialogButtonBox * m_buttons;

    OptionLayout * m_dataLayout;

    QVBoxLayout * m_mainLayout;

    bool m_okClicked;

    QMap<QString, CF::Common::XmlNode*> m_nodes;

  }; // class SignatureDialog

////////////////////////////////////////////////////////////////////////////////

} // namespace ClientUI
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UI_SignatureDialog_hpp
