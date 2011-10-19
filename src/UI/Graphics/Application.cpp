// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Exception.hpp"

#include "UI/Core/NLog.hpp"
#include "UI/Graphics/Application.hpp"

/////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Graphics {

/////////////////////////////////////////////////////////////////////////////

Application::Application(int & argc, char* argv[]) :
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
  catch(CF::Common::Exception & cfe)
  {
    CF::UI::Core::NLog::globalLog()->addException( cfe.what() );
  }
  catch(std::exception & stde)
  {
    CF::UI::Core::NLog::globalLog()->addException( stde.what() );
  }
  catch(...)
  {
    CF::UI::Core::NLog::globalLog()->addError( "Unknown exception was caught by the event handler." );
  }

  return false;
}

/////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // CF
