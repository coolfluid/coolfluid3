// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_Core_NBrowser_hpp
#define CF_GUI_Client_Core_NBrowser_hpp

//////////////////////////////////////////////////////////////////////////////

#include "GUI/Client/Core/CNode.hpp"

#include "GUI/Client/Core/LibClientCore.hpp"

class QString;

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientCore {

  ////////////////////////////////////////////////////////////////////////////

  /// @brief Component that manages remote browsers.
  /// This class subclasses CNode class.
  /// @author Quentin Gasper.

  class ClientCore_API NBrowser : public CNode
  {
  public:

    typedef boost::shared_ptr<NBrowser> Ptr;
    typedef boost::shared_ptr<NBrowser const> ConstPtr;

    /// @brief Constructor
    NBrowser();

    /// @brief Generates a name for a browser
    /// The name has the format "Browser_i" where "i" is the value of an
    /// internal counter, incremented each a name is generated.
    /// @return Returns the  generated name.
    QString generateName();

    /// @brief Gives the text to put on a tool tip
    /// @return The name of the class.
    virtual QString toolTip() const;

  private:

    /// @brief Browser counter.
    CF::Uint m_counter;

    /// regists all the signals declared in this class
    virtual void define_signals () {}

  }; // class NBrowser

  ////////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF

#endif // NBrowser_HPP
