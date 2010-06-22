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
    NGroup(const QString & name);

    NGroup(const QDomElement & element);

    /// @brief Gives a list of action the node can execute
    /// @return Returns a list of action the node can execute
    /// @note This method should be reimplemented by all subclasses.
    virtual QList<NodeAction> getNodeActions() const;

    /// @brief Gives the icon associated to this node
    /// @return Returns the icon associated to this node
    /// @note This method should be reimplemented by all subclasses.
    virtual QIcon getIcon() const;

    virtual QString getToolTip() const;

    /// @brief Gives a string with the class name.
    /// This implementation always returns "CNode". Subclass implementations
    /// should returns their own class name.
    /// @return Returns the class name.
    /// @note This method should be reimplemented by all subclasses.
    virtual QString getClassName() const;

    virtual void getOptions(QList<NodeOption> & params) const;

  }; // class NGroup

  //////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_NGroup_hpp
