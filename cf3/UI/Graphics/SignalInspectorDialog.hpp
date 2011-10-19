// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Graphics_SignalInspectorDialog_hpp
#define cf3_GUI_Graphics_SignalInspectorDialog_hpp

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

namespace UI {

namespace Graphics {

////////////////////////////////////////////////////////////////////////////////

class SignalInspectorDialog : public QDialog
{
  Q_OBJECT

public:

  SignalInspectorDialog(QWidget *parent = 0);

  ~SignalInspectorDialog();

  void show(const common::XML::SignalFrame & node);

private:

  QTextEdit * m_textArea;

  QDialogButtonBox * m_buttons;

  QVBoxLayout * m_mainLayout;

}; // SignalInspectorDialog

////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_GUI_Graphics_SignalInspectorDialog_hpp
