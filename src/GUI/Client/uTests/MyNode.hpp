// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_uTests_MyNode_hpp
#define CF_GUI_Client_uTests_MyNode_hpp

#include "GUI/Client/CNode.hpp"

namespace CF {
namespace GUI {
namespace ClientTest {

  class MyNode : public CF::GUI::Client::CNode
  {
  public:

    typedef boost::shared_ptr<CNode> Ptr;

    MyNode(const QString & name);

    QIcon getIcon() const;

    QString getToolTip() const;

    static void defineConfigProperties ( CF::Common::PropertyList& options )
    {
      options.add_option< CF::Common::OptionT<int> >("theAnswer", "The answer to the ultimate "
                                  "question of Life, the Universe, and Everything", 42);
      options.add_option< CF::Common::OptionT<bool> >("someBool", "The bool value", true);
    }

  private: // helper functions

    static void regist_signals ( CF::Common::Component* self ) {}
  };

}
}
}

#endif // MYNODE_HPP
