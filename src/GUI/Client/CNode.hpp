#ifndef CF_GUI_Client_CNode_hpp
#define CF_GUI_Client_CNode_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QDomDocument>
#include <QList>
#include <QHash>

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

  ////////////////////////////////////////////////////////////////////////////

  struct NodeOption
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
      public CF::Common::Component
  {
  public:

    ////////////////////////////////////////////

    typedef boost::shared_ptr<CNode> Ptr;
    typedef boost::shared_ptr<CNode const> ConstPtr;

    enum Type
    {
      ROOT_NODE,

      GROUP_NODE,

      LINK_NODE,

      MESH_NODE,

      METHOD_NODE,

      LOG_NODE,

      TREE_NODE,

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
    /// @note This method should be reimplemented by all subclasses.
    virtual QIcon getIcon() const = 0;

    virtual QString getToolTip() const = 0;

    virtual bool isClientComponent() const = 0;

    CNode::Type getType() const;

    inline bool checkType(CNode::Type type) const
    {
      return m_type == type;
    }

    void setTextData(const QString & text);

    void setOptions(const CF::Common::XmlNode & node);

    void modifyOptions(const QHash<QString, QString> options);

    virtual void getOptions(QList<NodeOption> & options) const = 0;

    /// @brief Creates an object tree from a given node

    /// @param node Node to convert
    /// @return Retuns a shared pointer to the created node.
    /// @throw XmlError If the tree could not be built.
    static CNode::Ptr createFromXml(CF::Common::XmlNode & node);

    static CNode::Ptr createFromXml(const CF::Common::XmlNode & node);

    QMenu * getContextMenu() const;

    void showContextMenu(const QPoint & pos) const;

    template<class TYPE>
    static boost::shared_ptr<TYPE> convertTo(CNode::Ptr node)
    {
      return boost::dynamic_pointer_cast<TYPE>(node);
    }

  protected:

    QString m_textData;

    QMenu * m_contextMenu;

    CNode::Type m_type;
		
		void buildOptionList(QList<NodeOption> & options) const;
		
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
