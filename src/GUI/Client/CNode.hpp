#ifndef CF_GUI_Client_CNode_hpp
#define CF_GUI_Client_CNode_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QDomDocument>
#include <QHash>
#include <QList>
#include <QObject>

#include "Common/Component.hpp"
#include "Common/OptionT.hpp"
#include "Common/XML.hpp"

#include "GUI/Client/OptionType.hpp"

class QIcon;
class QDomNode;
class QString;
class QAction;
class QMenu;
class QPoint;

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

  /////////////////////////////////////////////////////////////////////////

  class CNodeNotifier
    : public QObject
  {
    Q_OBJECT

  public:

    CNodeNotifier(QObject * parent = CFNULL);

    void notifyChildCountChanged();

  signals:

    void childCountChanged();

  }; // class CNodeNotifier

  ////////////////////////////////////////////////////////////////////////////

  /// @brief Structure that handles node options

  struct NodeOption
  {

    /// @brief Option name
    QString m_paramName;

    /// @brief Option type
    OptionType::Type m_paramType;

    /// @brief Option value
    QString m_paramValue;

    /// @brief Option description
    QString m_paramDescr;

    /// @brief If @c true, the option is advanced. Otherwise, it is not.
    bool m_paramAdv;

  }; // struct NodeParams

  ////////////////////////////////////////////////////////////////////////////

  /// @brief Base component adapted to fit the client needs.

  class CNode :
      public CF::Common::Component
  {
  public:

    ////////////////////////////////////////////

    typedef boost::shared_ptr<CNode> Ptr;
    typedef boost::shared_ptr<CNode const> ConstPtr;

    /// @brief Available sub-node types
    enum Type
    {
      /// @brief Root node
      ROOT_NODE,

      /// @brief Group node
      GROUP_NODE,

      /// @brief Link node
      LINK_NODE,

      /// @brief Mesh node
      MESH_NODE,

      /// @brief Method node
      METHOD_NODE,

      /// @brief Log node
      LOG_NODE,

      /// @brief Tree node
      TREE_NODE,

      /// @brief Browser node
      BROWSER_NODE

    }; // enum Type

    ////////////////////////////////////////////

    /// @brief Constructor.
    /// @param name Component name.
    /// @param componentType Corresponding component type name
    /// (on the simulator side)
    /// @param type Node type.
    CNode(const QString & name, const QString & componentType, CNode::Type type);

    /// @brief Gives the corresponding component type name
    /// @return Returns the corresponding component type name
    QString getComponentType() const;

    CNode::Ptr getNode(CF::Uint index);

    /// @brief Gives the icon associated to this node
    /// @return Returns the icon associated to this node
    virtual QIcon getIcon() const = 0;

    /// @brief Gives the node tooltip.
    /// @return Returns the tooltip text.
    virtual QString getToolTip() const = 0;

    /// @brief Indicates whether this node is a client component or not.
    /// @return Returns @c true if this node is a client component.
    bool isClientComponent() const
    {
      return m_type == LOG_NODE | m_type == TREE_NODE | m_type == BROWSER_NODE;
    }

    /// @brief Gives the node type
    /// @return Returns the node type
    CNode::Type getType() const;

    /// @brief Checks whether this node is of the provided type.

    /// Doing @code node->checkType(type) @endcode is equivalent to
    /// @code node->getType() == type @endcode.
    /// @param type The type to compare to.
    /// @return Returns @c true is this node type is the same as @c type
    inline bool checkType(CNode::Type type) const
    {
      return m_type == type;
    }

    /// @brief Sets node options
    /// @param node Note containing the options
    void setOptions(const CF::Common::XmlNode & node);

    /// @brief Modifies options

    /// If at least on option has been modified, a @c configure signal is sent
    /// to the corresponding component on the server.
    /// @param options Map of options to modify. The key is the option name.
    /// The value is the new option value, in string format.
    void modifyOptions(const QHash<QString, QString> options);

    /// @brief Gives options
    /// @param options Reference to a list where options will be put. The list
    /// cleared before first use.
    void getOptions(QList<NodeOption> & options) const;

    /// @brief Creates an object tree from a given node

    /// @param node Node to convert
    /// @return Retuns a shared pointer to the created node.
    /// @throw XmlError If the tree could not be built.
    static CNode::Ptr createFromXml(CF::Common::XmlNode & node);

    QMenu * getContextMenu() const;

    void showContextMenu(const QPoint & pos) const;

    template<class TYPE>
    static boost::shared_ptr<TYPE> convertTo(CNode::Ptr node)
    {
      return boost::dynamic_pointer_cast<TYPE>(node);
    }

    void connectNotifier(QObject * reciever, const char * signal, const char * slot);

    /// @brief Adds a sub-node.

    /// This method is a wrapper for @c Component::add_component(). It calls
    /// the parent method, but emits calls
    /// @c CNodeNotifier::notifyChildCountChanged() on success.@c
    /// It is recommended to add child nodes using this method.
    /// @param node Node to add.
    /// @throw CF::Common::ValueExists Forwards to the upper level any
    /// @c CF::Common::ValueExists exception thrown by
    /// @c Component::add_component()
    void addNode(CNode::Ptr node);

  protected:

    QMenu * m_contextMenu;

    CNode::Type m_type;

    CNodeNotifier * m_notifier;

    void configure(CF::Common::XmlNode & node);

  private:

    /// @brief Component type name.
    QString m_componentType;

    /// regists all the signals declared in this class
    static void regist_signals ( Component* self ) {}

    template < typename TYPE >
    void addOption ( const std::string & name, const std::string & descr,
                     CF::Common::XmlNode & node )
    {
      TYPE value;
      CF::Common::xmlstr_to_value(node, value);
      m_option_list.add< CF::Common::OptionT<TYPE> >(name, descr, value);
    }

  }; // class CNode

  ////////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

//////////////////////////////////////////////////////////////////////////////

#endif // CNODE_HPP
