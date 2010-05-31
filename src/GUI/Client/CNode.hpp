#ifndef CF_GUI_Client_CNode_hpp
#define CF_GUI_Client_CNode_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QList>
#include <QObject>

#include "Common/Component.hpp"

class QIcon;
class QString;

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

  ////////////////////////////////////////////////////////////////////////////

  /// @brief Holds node actions

  struct NodeAction
  {
  public:

    /// @brief Action name
    QString m_action;

    /// @brief Qt slot to used to execute this action. Use the SLOT() macro
    /// to assign the value.
    const char * m_slot;

    /// @brief Sub-actions
    QList<NodeAction> m_subActions;

  }; // struct NodeAction

  ////////////////////////////////////////////////////////////////////////////

  /// @brief Base component adapted to fit the client needs.

  class CNode :
      public QObject,
      public CF::Common::Component
  {
    Q_OBJECT

  public:

    typedef boost::shared_ptr<CNode> Ptr;

    /// @brief Constructor.

    /// @param name Component name.
    /// @param componentType Corresponding component type name
    /// (on the simulator side)
    CNode(const QString & name, const QString & componentType);

    /// @brief Gives the corresponding component type name
    /// @return Returns the corresponding component type name
    QString getComponentType() const;

    /// @brief Gives a list of action the node can execute
    /// @return Returns a list of action the node can execute
    /// @note This method should be reimplemented by all subclasses.
    virtual QList<NodeAction> getNodeActions() const;

    /// @brief Gives the icon associated to this node
    /// @return Returns the icon associated to this node
    /// @note This method should be reimplemented by all subclasses.
    virtual QIcon getIcon() const;

    /// @brief Gives a string with the class name.
    /// This implementation always returns "CNode". Subclass implementations
    /// should returns their own class name.
    /// @return Returns the class name.
    /// @note This method should be reimplemented by all subclasses.
    virtual QString getClassName() const;

  private:

    /// @brief Component type name.
    QString m_componentType;

  }; // class CNode

  ////////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

//////////////////////////////////////////////////////////////////////////////

#endif // CNODE_HPP
