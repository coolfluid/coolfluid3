// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_ui_Graphics_Application_hpp
#define CF_ui_Graphics_Application_hpp

#include <QApplication>

/////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

/////////////////////////////////////////////////////////////////////////////

class Application : public QApplication
{
  Q_OBJECT

public:

    Application(int & argc, char** argv);

    virtual bool notify(QObject *, QEvent *);

}; // Application

/////////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3

/////////////////////////////////////////////////////////////////////////////

#endif // APPLICATION_HPP
