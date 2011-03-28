// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Core_NBrowser_hpp
#define CF_GUI_Core_NBrowser_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QObject>

#include "UI/Core/CNode.hpp"

#include "UI/Core/LibCore.hpp"

class QString;

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Core {

  ////////////////////////////////////////////////////////////////////////////

  /// @brief Component that manages remote browsers.
  /// This class subclasses CNode class.
  /// @author Quentin Gasper.

  class Core_API NBrowser : public CNode, public QObject
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

    static Ptr globalBrowser();

  protected:

    /// Disables the local signals that need to.
    /// @param localSignals Map of local signals. All values are set to true
    /// by default.
    virtual void disableLocalSignals(QMap<QString, bool> & localSignals) const {}

  private:

    /// @brief Browser counter.
    CF::Uint m_counter;

  }; // class NBrowser

  ////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // CF

#endif // NBrowser_HPP
