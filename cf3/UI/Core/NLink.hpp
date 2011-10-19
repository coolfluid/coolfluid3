// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Core_NLink_hpp
#define cf3_GUI_Core_NLink_hpp

//////////////////////////////////////////////////////////////////////////////

#include "UI/Core/CNode.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {

namespace common { class URI; }

namespace UI {
namespace Core {

////////////////////////////////////////////////////////////////////////////

  /// @brief Client corresponding component for @c cf3::common::CLink.
  /// @author Quentin Gasper.

  class Core_API NLink :
      public QObject,
      public CNode
  {

  public:

    typedef boost::shared_ptr<NLink> Ptr;
    typedef boost::shared_ptr<NLink const> ConstPtr;

    /// @brief Constructor
    /// @param name Link name
    /// @param targetPath Target path
    NLink(const std::string & name);

    /// @brief Gives the text to put on a tool tip
    /// @return The name of the class.
    virtual QString toolTip() const;

    /// @brief Gives the target path
    /// @return Returns the target path.
    cf3::common::URI targetPath() const;

    /// @brief Sets new target from its path.
    /// @param path New target path.
    /// @throw InvalidPath If the path does exist.
    void setTargetPath(const cf3::common::URI & path);

    /// @brief Sets new target.
    /// If the node is null, nothing is done.
    /// @param path New target path.
    void setTargetNode(const CNode::Ptr & node);

    /// @brief Slot called when user wants to switch to the target
    void goToTarget(common::SignalArgs & node);

  protected:

    /// Disables the local signals that need to.
    /// @param localSignals Map of local signals. All values are set to true
    /// by default.
    virtual void disableLocalSignals(QMap<QString, bool> & localSignals) const {}

  private :

    /// @brief Target path
    CNode::Ptr m_target;

    /// @name Signals
    //@{

    /// @brief Method called when target path changes.
    /// @param node Signal parameters. Should contain a string value named
    /// "target_path" with the new target path.
    void change_link(common::SignalArgs & node);

    //@} END Signals

  }; // class NLink

//////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // cf3

//////////////////////////////////////////////////////////////////////////////

#endif // cf3_GUI_Core_NLink_hpp
