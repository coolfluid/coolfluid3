// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_UI_SignalInspectorDialog_hpp
#define CF_GUI_Client_UI_SignalInspectorDialog_hpp

////////////////////////////////////////////////////////////////////////////

#include <QDialog>

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientUI {

////////////////////////////////////////////////////////////////////////////////

class SignalInspectorDialog : public QDialog
{
  Q_OBJECT

public:

  SignalInspectorDialog(QWidget *parent = 0);

public slots:

};

////////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UI_SignalInspectorDialog_hpp
