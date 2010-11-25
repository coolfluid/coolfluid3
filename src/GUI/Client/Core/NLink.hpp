// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_Core_NLink_hpp
#define CF_GUI_Client_Core_NLink_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QObject>

#include "GUI/Client/Core/CNode.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {

namespace Common { class CPath; }

namespace GUI {
namespace ClientCore {

  ////////////////////////////////////////////////////////////////////////////

  /// @brief Client corresponding component for @c CF::Common::CLink.
  /// @author Quentin Gasper.

  class ClientCore_API NLink :
      public QObject,
      public CNode
  {
    Q_OBJECT

  public:

    typedef boost::shared_ptr<NLink> Ptr;
    typedef boost::shared_ptr<NLink const> ConstPtr;

    /// @brief Constructor
    /// @param name Link name
    /// @param targetPath Target path
    NLink(const QString & name);

    /// @brief Gives the text to put on a tool tip
    /// @return The name of the class.
    virtual QString toolTip() const;

    /// @brief Gives the target path
    /// @return Returns the target path.
    CF::Common::CPath targetPath() const;

    /// @brief Sets new target from its path.
    /// @param path New target path.
    /// @throw InvalidPath If the path does exist.
    void setTargetPath(const CF::Common::CPath & path);

    /// @brief Sets new target.
    /// If the node is null, nothing is done.
    /// @param path New target path.
    void setTargetNode(const CNode::Ptr & node);

    /// @brief Slot called when user wants to switch to the target
    void goToTarget(CF::Common::XmlNode & node);

  public slots:

    /// @brief Slot called when user wants to change the target path
    void changeTarget();

  private :

    /// @brief Target path
    CNode::Ptr m_target;

    /// @name Signals
    //@{

    /// @brief Method called when target path changes.
    /// @param node Signal parameters. Should contain a string value named
    /// "target_path" that with the new target path.
    void change_link(CF::Common::XmlNode & node);

    //@} END Signals
  }; // class NLink

//////////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_Core_NLink_hpp
