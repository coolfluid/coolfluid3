// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Exception.hpp"

#include "UI/Core/NLog.hpp"
#include "UI/Graphics/Application.hpp"

/////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

/////////////////////////////////////////////////////////////////////////////

Application::Application(int & argc, char** argv) :
    QApplication(argc, argv)
{
}

/////////////////////////////////////////////////////////////////////////////

bool Application::notify(QObject * obj, QEvent * ev)
{
  try
  {
    return QApplication::notify( obj, ev );
  }
  catch(common::Exception & cfe)
  {
    core::NLog::global()->add_exception( cfe.what() );
  }
  catch(std::exception & stde)
  {
    core::NLog::global()->add_exception( stde.what() );
  }
  catch(...)
  {
    core::NLog::global()->add_error( "Unknown exception was caught by the event handler." );
  }

  return false;
}

/////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3
