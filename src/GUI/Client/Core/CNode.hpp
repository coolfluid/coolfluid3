// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_Core_CNode_hpp
#define CF_GUI_Client_Core_CNode_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QMap>
#include <QObject>
#include <QStringList>
#include <QDebug>


#include "Common/Component.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionArray.hpp"
#include "Common/XML.hpp"
#include "Common/XmlHelpers.hpp"

#include "GUI/Client/Core/LibClientCore.hpp"

class QString;
class QAction;
class QMenu;
class QMutex;
class QPoint;

template<typename T> class QList;

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientCore {

  class NLink;
  class CNode;

  /////////////////////////////////////////////////////////////////////////

  /// @brief Handles signal emitting for @c #CNode class.

  /// @c #CNode class cannot derive from @c QObject (thus, cannot emit or catch
  /// Qt signals) because one of its subclasses (@c #NTree) also derives from
  /// another Qt class. It means that this subclass would derive twice from
  /// @c QObject, which is illegal in C++.
  /// @author Quentin Gasper
  class ClientCore_API CNodeNotifier :
      public QObject
  {
    Q_OBJECT

  public:

    /// @brief Constructor

    /// @param parent The parent @c CNode. May be null.
    CNodeNotifier(CNode * parent = nullptr);

    /// @brief Emits @c #childCountChanged() signal.
    void notifyChildCountChanged();

    /// @brief Emits @c #contentChanged() signal.
    void notifySignalSignature(Common::XmlNode & node);

  signals:

    /// @brief Signal emitted when children have been added or removed.
    void childCountChanged();

    /// @brief Signal emitted when a signal signature has been received.
    void signalSignature(Common::XmlNode & node);

  private:

    /// @brief The parent @c CNode.
    CNode * m_parent;

  }; // class CNodeNotifier

  ////////////////////////////////////////////////////////////////////////////

  /// @brief Handles information about actions (signals).
  /// @author Quentin Gasper
  struct ClientCore_API ActionInfo
  {
    /// @brief The action name.
    QString name;

    /// @brief The action readable name. This name is intended to be displayed and
    /// should spaces instead of undescores.
    QString readableName;

    /// @brief The action description.
    QString description;

    /// @brief Indicates whether the action is local or not.

    /// If @c true, the action is considered as local and has to be executed on the
    /// local component. If @c false, the action has to be executed on the remote
    /// component (on COOLFluiD side)
    bool isLocal;
  };

  ////////////////////////////////////////////////////////////////////////////

  /// @brief Base component adapted to fit the client needs.
  /// @author Quentin Gasper
  class ClientCore_API CNode :
      public Common::Component
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

      /// @brief Browser node
      BROWSER_NODE,

      /// @brief Link node
      LINK_NODE,

      /// @brief Log node
      LOG_NODE,

      /// @brief Tree node
      TREE_NODE,

      /// @brief Core node
      CORE_NODE,

      /// @brief Browser node
      JOURNAL_NODE,

      /// @brief Journal browser node
      JOURNAL_BROWSER_NODE,

      /// @brief Generic node
      GENERIC_NODE

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

    /// @brief Gives a child a a certain index.
    /// @param index Index of the wanted child. Should be between 0 (included)
    /// and @c #get_child_count() (excluded).
    /// @return Returns the found child.
    CNode::Ptr child(Uint index);

    /// @brief Gives the node tooltip.
    /// @return Returns the tooltip text.
    virtual QString toolTip() const = 0;

    /// @brief Indicates whether this node is a client component or not.
    /// @return Returns @c true if this node is a client component.
    bool isClientComponent() const
    {
      return (m_type == LOG_NODE) | (m_type == TREE_NODE) | (m_type == CORE_NODE) |
          (m_type == BROWSER_NODE) | (m_type == JOURNAL_BROWSER_NODE);
    }

    /// @brief Gives the node type
    /// @return Returns the node type
    CNode::Type type() const;

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
    void setOptions(Common::XmlNode & node);

    /// @brief Sets node properties
    /// @param node Note containing the options
    void setProperties(Common::XmlNode & node);

    /// @brief Sets node signals
    /// Those are considered as non-local ones, meaning that asking the node
    /// to execute them will result to the sendng of a request to the remote
    /// component.
    /// @param node Node containing the signals
    void setSignals(Common::XmlNode & node);

    /// @brief Modifies options

    /// If at least on option has been modified, a @c configure signal is sent
    /// to the corresponding component on the server.
    /// @param options Map of options to modify. The key is the option name.
    /// The value is the new option value, in string format.
    void modifyOptions(const QMap<QString, QString> & options);

    /// @brief Gives options
    /// @param options Reference to a list where options will be put. The list
    /// cleared before first use.
    void options(QList<Common::Option::ConstPtr> & list);

    /// @brief Gives properties
    /// @param props Reference to a map where properties will be put. The map
    /// cleared before first use.
    void properties(QMap<QString, QString> & props);

    void actions(QList<ActionInfo> & actions);

    /// @brief Creates an object tree from a given node

    /// @param node Node to convert
    /// @return Retuns a shared pointer to the created node.
    /// @throw XmlError If the tree could not be built.
    static CNode::Ptr createFromXml(Common::XmlNode & node);

    /// @brief Casts this node to a constant component of type TYPE.
    /// @return Returns the cast pointer
    /// @throw boost::bad_any_cast if the casting failed.
    template<class TYPE>
    boost::shared_ptr<const TYPE> castTo() const
    {
      return boost::dynamic_pointer_cast<TYPE>(shared_from_this());
    }

    /// @brief Casts this node to a component of type TYPE.
    /// @return Returns the cast pointer
    /// @throw boost::bad_any_cast if the casting failed.
    template<class TYPE>
    boost::shared_ptr<TYPE> castTo()
    {
      return boost::dynamic_pointer_cast<TYPE>(shared_from_this());
    }

    /// @brief Connects a slot a signal provided by the internal notifier.
    /// @param receiver The receiver object, The object on which the slot will be
    /// called.
    /// @param signal The signal.
    /// @param slot The slot to connect.
    /// @see CNodeNotifier
    void connectNotifier(QObject * reciever, const char * signal, const char * slot);

    /// @brief Adds a sub-node.

    /// This method is a wrapper for @c Component::add_component(). It calls
    /// the parent method, but emits
    /// @c CNodeNotifier::notifyChildCountChanged() on success.@c
    /// It is recommended to add child nodes using this method in order to
    /// guarantee the view is correctly updated.
    /// @param node Node to add.
    /// @throw Common::ValueExists Forwards to the upper level any
    /// @c Common::ValueExists exception thrown by
    /// @c Component::add_component()
    void addNode(CNode::Ptr node);

    /// @brief Removes a sub-node.

    /// This method is a wrapper for @c Component::remove_component(). It calls
    /// the parent method, but emits
    /// @c CNodeNotifier::notifyChildCountChanged() on success.@c
    /// It is recommended to remove child nodes using this method in order to
    /// guarantee the view is correctly updated.
    /// @param node Node to remove.
    /// @throw Common::ValueNotFound Forwards to the upper level any
    /// @c Common::ValueNotFound exception thrown by
    /// @c Component::remove_component()
    void removeNode(const QString & nodeName);

    /// @brief Gives the internal notifier.
    /// @return Returns the internal notifier.
    CNodeNotifier * notifier() const;

    /// @brief Lists all children paths in a string list.

    /// Strings have the same format as returned by @c CPath::full_path().string().
    /// @param list The string list where paths will be stored. The list is not
    /// cleaned before first use.
    /// @param recursive If @c true, the listing is recursive. Otherwise,
    /// only direct children are listed.
    /// @param clientNode If @c true, client nodes are included into the the
    /// result. Otherwise, they are ignored.
    void listChildPaths(QStringList & list, bool recursive, bool clientNode = true) const;

    /// @brief Creates an option from an XML node.

    /// This method handles @c OptionT, @c OptionURI and @c OptionArray types.
    /// @param node THe XML node from where the option has to be created.
    /// @return Returns the created option.
    /// @note This function should be removed once the new XML layer is operational.
    static Common::Option::Ptr makeOption(const Common::XmlNode & node);

    void requestSignalSignature(const QString & name);

    /// @name Signals
    //@{

    /// @brief Method called when a @e tree_update event occurs on the server.
    /// This methods calls @c NCore::update_tree() method to resquet an update
    /// of the tree.
    /// @param node Signal data. This parameter is not used.
    Common::Signal::return_t update_tree( Common::XmlNode & node);

    /// @brief Method called when receiving a reply to a previously sent
    /// "configure" signal.
    /// @param node An XML representation of the modified options.
    void configure_reply(Common::XmlNode & node);

    void list_content_reply( Common::XmlNode & node );

    void signal_signature_reply( Common::XmlNode & node );

    //@} END Signals

    void localSignature(const QString & name, Common::XmlNode& node );

  protected: // data

    /// @brief This node type.
    CNode::Type m_type;

    /// @brief This internal notifier.
    CNodeNotifier * m_notifier;

    /// @brief Lists the names of the local signals.
    QStringList m_localSignals;

    QMutex * m_mutex;

  private: // data

    /// @brief Component type name.
    QString m_componentType;

    /// @brief List of signals that can be remotely executed
    QList<ActionInfo> m_actionSigs;

    /// @c false until the node content has been retrieved from
    /// the server.
    bool m_contentListed;

    /// Idicates whether this node is already waiting for content
    /// from the server.
    /// This is used to avoid sending multiple requests to the server
    /// in case it is overloaded and takes some time to reply.
    bool m_listingContent;

  private: // helper functions

    /// @brief Creates an @c #OptionT option with a value of type TYPE.
    /// @param name Option name
    /// @param descr Option description
    /// @param node The value node. If it has a sibling node, this node is taken
    /// the restricted values list.
    /// @return Returns the created option.
    template<typename TYPE>
    static Common::Option::Ptr makeOptionT(const std::string & name,
                                               const std::string & descr,
                                               Common::XmlNode & node)
    {
      TYPE value;
      Common::to_value(node, value);
      Common::XmlNode * next = node.next_sibling();

      Common::Option::Ptr option(new Common::OptionT<TYPE>(name, descr, value));

      if(next != nullptr &&
         std::strcmp(next->name(), Common::XmlTag<TYPE>::array()) == 0)
      {

        Common::XmlNode * elem_node = next->first_node("e");

        for( ; elem_node != nullptr ; elem_node = elem_node->next_sibling("e"))
        {
          Common::to_value(*elem_node, value);
          option->restricted_list().push_back(TYPE(value));
        }
      }

      return option;
    }

    /// @brief Creates an @c #OptionArrayT option with values of type TYPE.
    /// @param name Option name
    /// @param descr Option description
    /// @param node The value node. If it has a sibling node, this node is taken
    /// the restricted values list.
    /// @return Returns the created option.
    template<typename TYPE>
    static Common::Option::Ptr makeOptionArrayT(const std::string & name,
                                                    const std::string & descr,
                                                    const Common::XmlNode & node)
    {
      std::vector<TYPE> value;
      //Common::to_value(node, value);
      Common::XmlNode * next = node.next_sibling();

      if(next != nullptr &&
         std::strcmp(next->name(), Common::XmlTag<TYPE>::array()) == 0)
      {

        Common::XmlNode * elem_node = next->first_node("e");

        for( ; elem_node != nullptr ; elem_node = elem_node->next_sibling("e"))
        {
          TYPE elem_value;
          Common::to_value(*elem_node, elem_value);
          value.push_back(TYPE(elem_value));
        }
      }

      Common::Option::Ptr option(new Common::OptionArrayT<TYPE>(name, descr, value));

      return option;
    }

    /// @brief Creates an object tree from a given node

    /// This is a recursive method. The second parameter holds, for each link
    /// the tree, the @c CPath of the target. The methods proceeds in this way
    /// since the target might not exist yet when the link exists. Furthermore,
    /// the method does not guarantee that each target path exists (e.i. the
    /// target is missing in the XML). It is up to the calling code to make
    /// that check.
    /// @param node Node to convert
    /// @param linkTargets Map where links
    /// @return Retuns a shared pointer to the created node.
    /// @throw XmlError If the tree could not be built.
    static CNode::Ptr createFromXmlRec(Common::XmlNode & node,
               QMap<boost::shared_ptr<NLink>, Common::URI> & linkTargets);

    /// @brief Converts a std::vector<boost::any> to a string list.

    /// Vector elements are supposed to be of type TYPE.
    /// @param vect The vector to convert.
    /// @return Returns the built list. May be empty, if the vector is empty.
    /// @throw boost::bad_any_cast If the casting failed.
    template<typename TYPE>
    QStringList vectToStringList(const std::vector<boost::any> & vect) const
    {
      QStringList returnList;
      std::vector<boost::any>::const_iterator it = vect.begin();

      for( ; it != vect.end() ; it++)
        returnList << Common::from_value( boost::any_cast<TYPE>(*it) ).c_str();

      return returnList;
    }

    void fetchContent();

  }; // class CNode

  ////////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_Core_CNode_hpp
