// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_Core_NGeneric_hpp
#define CF_GUI_Client_Core_NGeneric_hpp

////////////////////////////////////////////////////////////////////////////

#include <QObject>

#include "GUI/Client/Core/CNode.hpp"

#include "GUI/Client/Core/LibClientCore.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientCore {

  //////////////////////////////////////////////////////////////////////////

  /// @brief Client generic component.
  /// @author Quentin Gasper.

  class ClientCore_API NGeneric :
      public CNode
  {

  public:

    typedef boost::shared_ptr<NGeneric> Ptr;
    typedef boost::shared_ptr<NGeneric const> ConstPtr;

    /// @brief Constructor
    /// @param name Group name
    /// @param The component type
    NGeneric(const QString & name, const QString & type);

    /// @brief Gives the text to put on a tool tip
    /// @return The name of the class.
    virtual QString toolTip() const;

  protected:

    /// Disables the local signals that need to.
    /// @param localSignals Map of local signals. All values are set to true
    /// by default.
    virtual void disableLocalSignals(QMap<QString, bool> & localSignals) const {}

  }; // class NGeneric

  //////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_Core_NGeneric_hpp
