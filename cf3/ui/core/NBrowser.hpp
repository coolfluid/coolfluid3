// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_core_NBrowser_hpp
#define cf3_ui_core_NBrowser_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QObject>

#include "ui/core/CNode.hpp"

#include "ui/core/LibCore.hpp"

class QString;

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace core {

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
    QString generate_name();

    /// @brief Gives the text to put on a tool tip
    /// @return The name of the class.
    virtual QString tool_tip() const;

    static Ptr global();

  protected:

    /// Disables the local signals that need to.
    /// @param localSignals Map of local signals. All values are set to true
    /// by default.
    virtual void disable_local_signals(QMap<QString, bool> & localSignals) const {}

  private:

    /// @brief Browser counter.
    cf3::Uint m_counter;

  }; // class NBrowser

  ////////////////////////////////////////////////////////////////////////////

} // Core
} // ui
} // cf3

#endif // NBrowser_HPP
