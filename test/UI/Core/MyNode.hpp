// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_uTests_MyNode_hpp
#define CF_GUI_Client_uTests_MyNode_hpp

#include "UI/Core/CNode.hpp"

namespace CF {
namespace UI {
namespace CoreTest {

  class MyNode : public CF::UI::Core::CNode
  {
  public:

    typedef boost::shared_ptr<CNode> Ptr;

    MyNode(const std::string & name);

    QIcon getIcon() const;

    QString toolTip() const;

  protected:

    /// Disables the local signals that need to.
    /// @param localSignals Map of local signals. All values are set to true
    /// by default.
    virtual void disableLocalSignals(QMap<QString, bool> & localSignals) const {}

  }; // MyNode

}
}
}

#endif // MYNODE_HPP
