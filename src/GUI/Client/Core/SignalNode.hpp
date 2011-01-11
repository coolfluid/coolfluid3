// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_Core_SignalNode_hpp
#define CF_GUI_Client_Core_SignalNode_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/XML.hpp"

class QString;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientCore {

////////////////////////////////////////////////////////////////////////////////

  class SignalNode
  {

  public:

    SignalNode(const Common::XmlNode * node);

    QString target() const;

    QString sender() const;

    QString receiver() const;

    QString type() const;

    QString direction() const;

    QString time() const;

    const Common::XmlNode * node() const;

  private:

    const Common::XmlNode * m_node;

  }; // class SignalNode

////////////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_Core_SignalNode_hpp
