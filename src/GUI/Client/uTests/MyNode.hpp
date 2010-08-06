#ifndef MYNODE_HPP
#define MYNODE_HPP

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

    static void defineConfigOptions ( CF::Common::OptionList& options )
    {
      options.add< CF::Common::OptionT<int> >("theAnswer", "The answer to the ultimate "
                                  "question of Life, the Universe, and Everything", 42);
      options.add< CF::Common::OptionT<bool> >("someBool", "The bool value", true);
    }

  private: // helper functions

    static void regist_signals ( CF::Common::Component* self ) {}
  };

}
}
}

#endif // MYNODE_HPP
