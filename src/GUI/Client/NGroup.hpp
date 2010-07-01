#ifndef CF_GUI_Client_NGroup_hpp
#define CF_GUI_Client_NGroup_hpp

////////////////////////////////////////////////////////////////////////////

#include "GUI/Client/CNode.hpp"

class QDomElement;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

  //////////////////////////////////////////////////////////////////////////

  class NGroup :
      public CNode
  {
    Q_OBJECT

  public:

    typedef boost::shared_ptr<NGroup> Ptr;

    NGroup(const QString & name);

    /// @brief Gives the icon associated to this node
    /// @return Returns the icon associated to this node
    /// @note This method should be reimplemented by all subclasses.
    virtual QIcon getIcon() const;

    virtual QString getToolTip() const;

    virtual void getOptions(QList<NodeOption> & params) const;

  }; // class NGroup

  //////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_NGroup_hpp
