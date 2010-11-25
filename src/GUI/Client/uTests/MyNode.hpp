// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_uTests_MyNode_hpp
#define CF_GUI_Client_uTests_MyNode_hpp

#include "GUI/Client/Core/CNode.hpp"

namespace CF {
namespace GUI {
namespace ClientTest {

  class MyNode : public CF::GUI::ClientCore::CNode
  {
  public:

    typedef boost::shared_ptr<CNode> Ptr;

    MyNode(const QString & name);

    QIcon getIcon() const;

    QString toolTip() const;

    virtual void define_config_properties ()
    {
      m_properties.add_option< CF::Common::OptionT<int> >("theAnswer", "The answer to the ultimate "
                                  "question of Life, the Universe, and Everything", 42);
      m_properties.add_option< CF::Common::OptionT<bool> >("someBool", "The bool value", true);
    }

  };

}
}
}

#endif // MYNODE_HPP
