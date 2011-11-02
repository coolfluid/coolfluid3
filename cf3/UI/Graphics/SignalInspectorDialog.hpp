// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_Graphics_SignalInspectorDialog_hpp
#define cf3_ui_Graphics_SignalInspectorDialog_hpp

////////////////////////////////////////////////////////////////////////////

#include <QDialog>


class QDialogButtonBox;
class QTextEdit;
class QVBoxLayout;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {

namespace common
{
  namespace XML { class SignalFrame; }
}

namespace ui {

namespace graphics {

////////////////////////////////////////////////////////////////////////////////

class SignalInspectorDialog : public QDialog
{
  Q_OBJECT

public:

  SignalInspectorDialog(QWidget *parent = 0);

  ~SignalInspectorDialog();

  void show(const common::XML::SignalFrame & node);

private:

  QTextEdit * m_text_area;

  QDialogButtonBox * m_buttons;

  QVBoxLayout * m_main_layout;

}; // SignalInspectorDialog

////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_Graphics_SignalInspectorDialog_hpp
