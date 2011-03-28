// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_UI_TabBuilder_hpp
#define CF_GUI_Client_UI_TabBuilder_hpp

///////////////////////////////////////////////////////////////////////////

#include <QTabWidget>

///////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Graphics {

///////////////////////////////////////////////////////////////////////////

class TabBuilder : public QTabWidget
{

public:

  static TabBuilder * instance();

private:

  TabBuilder(QWidget * parent = 0);

  ~TabBuilder();

}; // TabManager

///////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // CF

///////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UI_TabBuilder_hpp
