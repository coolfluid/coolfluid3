#ifndef CF_GUI_Client_CNode_hpp
#define CF_GUI_Client_CNode_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QDomDocument>
#include <QList>
#include <QObject>

#include "Common/Component.hpp"

#include "GUI/Client/OptionType.hpp"

class QIcon;
class QDomNode;
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

  struct NodeParams
  {
    QString m_paramName;

    OptionType::Type m_paramType;

    QString m_paramValue;

    QString m_paramDescr;

    bool m_paramAdv;
  }; // struct NodeParams

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

    /// @brief Gives of child nodes
    /// @return Returns the number of child nodes this node has.
    int getNodeCount() const;

    CNode::Ptr getNode(CF::Uint index);

    /// @brief Gives a list of action the node can execute
    /// @return Returns a list of action the node can execute
    /// @note This method should be reimplemented by all subclasses.
    virtual QList<NodeAction> getNodeActions() const = 0;

    /// @brief Gives the icon associated to this node
    /// @return Returns the icon associated to this node
    /// @note This method should be reimplemented by all subclasses.
    virtual QIcon getIcon() const = 0;

    virtual QString getToolTip() const = 0;

    /// @brief Gives a string with the class name.
    /// This implementation always returns "CNode". Subclass implementations
    /// should returns their own class name.
    /// @return Returns the class name.
    /// @note This method should be reimplemented by all subclasses.
    virtual QString getClassName() const = 0;

    void setTextData(const QString & text);

    void setParams(const QDomNodeList & list);

    void getParams(QList<NodeParams> & params) const;

    /// @brief Creates an object tree from a given node

    /// @param node Node to convert
    /// @return Retuns a shared pointer to the created node.
    /// @throw XmlError If the tree could not be built.
    static CNode::Ptr createFromXml(const QDomElement & node);

  protected:

    QList<NodeParams> m_params;

    QString m_textData;

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
