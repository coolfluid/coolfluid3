// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_Graphics_SignatureDialog_hpp
#define cf3_ui_Graphics_SignatureDialog_hpp

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

namespace ui {
namespace graphics {

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

    void bt_ok_clicked();

    void bt_cancel_clicked();

  private:

    QDialogButtonBox * m_buttons;

    OptionLayout * m_data_layout;

    QVBoxLayout * m_main_layout;

    QMap<QString, common::XML::XmlNode> m_nodes;

    bool m_ok_clicked;

    bool m_is_blocking;

  }; // class SignatureDialog

////////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_Graphics_SignatureDialog_hpp
