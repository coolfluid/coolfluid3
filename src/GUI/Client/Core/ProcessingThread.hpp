// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_Core_ProcessingThread_hpp
#define CF_GUI_Client_Core_ProcessingThread_hpp

////////////////////////////////////////////////////////////////////////////

#include <QThread>

#include "GUI/Client/Core/LibClientCore.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientCore {

////////////////////////////////////////////////////////////////////////////

class ClientCore_API ProcessingThread : public QThread
{
  Q_OBJECT

public:

  ProcessingThread(Common::XML::XmlDoc::Ptr m_node);

  void run();

  Common::XML::XmlNode & getNode() const;

private:

  Common::XML::XmlDoc::Ptr m_node;

}; // ProcessingThread

////////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_Core_ProcessingThread_hpp
