// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_core_NGeneric_hpp
#define cf3_ui_core_NGeneric_hpp

////////////////////////////////////////////////////////////////////////////

#include <QObject>

#include "UI/Core/CNode.hpp"

#include "UI/Core/LibCore.hpp"

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace core {

  //////////////////////////////////////////////////////////////////////////

  /// @brief Client generic component.
  /// @author Quentin Gasper.

  class Core_API NGeneric :
      public CNode
  {

  public:

    typedef boost::shared_ptr<NGeneric> Ptr;
    typedef boost::shared_ptr<NGeneric const> ConstPtr;

    /// @brief Constructor
    /// @param name Group name
    /// @param type The component type
    /// @param node_type The type of the node
    NGeneric( const std::string & name,
              const QString & type,
              CNode::Type node_type = CNode::STANDARD_NODE);

    /// @brief Gives the text to put on a tool tip
    /// @return The name of the class.
    virtual QString tool_tip() const;

  protected:

    /// Disables the local signals that need to.
    /// @param localSignals Map of local signals. All values are set to true
    /// by default.
    virtual void disable_local_signals(QMap<QString, bool> & local_signals) const {}

  }; // class NGeneric

  //////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_core_NGeneric_hpp
