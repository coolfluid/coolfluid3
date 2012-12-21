// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_core_CNode_hpp
#define cf3_ui_core_CNode_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QStringList>

#include "common/Component.hpp"
#include "common/Option.hpp"

#include "ui/core/LibCore.hpp"

class QMutex;
class QString;

template<typename T> class QList;
template<typename T, typename V> class QMap;

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace core {

  class NLink;
  class CNode;

  /////////////////////////////////////////////////////////////////////////

  /// @brief Handles signal emitting for @c CNode class.

  /// @c #CNode class cannot derive from @c QObject (thus, cannot emit or catch
  /// Qt signals) because one of its subclasses (@c #NTree) also derives from
  /// another Qt class. It means that this subclass would derive twice from
  /// @c QObject, which is illegal in C++.
  /// @author Quentin Gasper
  class Core_API CNodeNotifier :
      public QObject
  {
    Q_OBJECT

  public:

    /// Constructor

    /// @param parent The parent @c CNode. May be null.
    CNodeNotifier(CNode * parent = nullptr);

    /// Emits @c #child_count_changed() signal.
    void notify_child_count_changed();

    /// Emits @c #signal_signature() signal.
    void notify_signal_signature(common::SignalArgs &node);

  signals:

    /// Signal emitted when children have been added or removed.
    void child_count_changed();

    /// Signal emitted when a signal signature has been received.
    void signal_signature(cf3::common::SignalArgs & node);

  private:

    /// The parent @c CNode.
    CNode * m_parent;

  }; // class CNodeNotifier

  ////////////////////////////////////////////////////////////////////////////

  /// Handles information about actions (signals).
  /// @author Quentin Gasper
  struct Core_API ActionInfo
  {
    /// The action name.
    QString name;

    /// The action readable name. This name is intended to be displayed and
    /// should spaces instead of undescores.
    QString readable_name;

    /// The action description.
    QString description;

    /// Indicates whether the action is local or not.

    /// If @c true, the action is considered as local and has to be executed on the
    /// local component. If @c false, the action has to be executed on the remote
    /// component (on COOLFluiD side)
    bool is_local;

    /// Indicates wheter the action is enable or not.
    bool is_enabled;
  };

  ////////////////////////////////////////////////////////////////////////////

  /// Base component adapted to fit the client needs.

  /// Every component class created in the client should derive from this
  /// class.
  /// Each CNode has a special type, that helps a potential model to
  /// determine whether it should be showed or hidden, depending on
  /// the current modes (debug and/or advanced). @n
  /// The types are defined by the nested enum @c CNode::Type. Standard
  /// nodes are typically those comming from the server, they are deleted
  /// everytime the tree is updated. Local nodes are managed by the client
  /// itself to handle some local information, they only exist on the client-side.
  /// Debug nodes are local ones, except that they are only visible in
  /// debug mode. Local and debug modes exist are not deleted on tree update.

  /// @author Quentin Gasper
  class Core_API CNode :
      public common::Component
  {
  public:

    ////////////////////////////////////////////




    /// Defines the sub-node types
    enum Type
    {
      /// Standard node.
      STANDARD_NODE = 0,

      /// Local node.
      LOCAL_NODE = 1,

      /// Local and debug node.
      DEBUG_NODE = 2
    }; // enum Type

    ////////////////////////////////////////////

    /// Constructor.
    /// @param name Component name.
    /// @param component_type Corresponding component type name
    /// (on the simulator side)
    /// @param type Node type.
    CNode( const std::string & name, const QString & component_type, Type type );

    static std::string type_name () { return "CNode"; }

    /// Component::derived_type_name implementation
    std::string derived_type_name() const
    {
      return common::TypeInfo::instance().portable_types[ typeid(*this).name() ];
    }

    /// Gives the corresponding component type name
    /// @return Returns the corresponding component type name
    QString component_type() const;

    /// Gives a child a a certain index.
    /// @param index Index of the wanted child. Should be between 0 (included)
    /// and @c #count_children() (excluded).
    /// @return Returns the found child.
    Handle< CNode > child( Uint index );

    /// Gives the node tooltip.
    /// @return Returns the tooltip text.
    virtual QString tool_tip() const = 0;

    /// Indicates whether this node is a client component or not.
    /// A node is considered as a client one if its type is either
    /// @c CNode::LOCAL_TYPE or @c CNode::DEBUB_NODE.
    /// @return Returns @c true if this node is a client component.
    bool is_local_component() const
    {
      return m_type != STANDARD_NODE;
    }

    /// Gives the node type.
    /// @return Returns the type of this node.
    Type type() const
    {
      return m_type;
    }

    /// Indicates whether this component is the root or not.
    /// @return Returns @c true if this node is a NRoot component.
    bool is_root()
    {
      return m_is_root;
    }

    /// Sets node properties
    /// @param node Node containing the options
    void set_properties( const common::SignalArgs & node );

    /// Sets node signals
    /// Those are considered as non-local ones, meaning that asking the node
    /// to execute them will result to the sendng of a request to the remote
    /// component.
    /// @param node Node containing the signals
    void set_signals( const common::SignalArgs & node );

    /// Modifies options

    /// If at least one option has been modified, a @c configure signal is sent
    /// to the corresponding component on the server.
    /// @param options Map of options to modify. The key is the option name.
    /// The value is the new option value, in string format.
    /// @throw BadValue If an option could not be found or could not be
    /// converted to an option.
    void modify_options( const QMap<QString, QString> & options );

    /// Gives options
    /// @param options Reference to a list where options will be put. The list
    /// cleared before first use.
    void list_options( QList<boost::shared_ptr< common::Option > > & list );

    /// Gives properties
    /// @param props Reference to a map where properties will be put. The map
    /// is cleared before first use.
    void list_properties( QMap<QString, QString> & props );

    /// Gives actions.
    /// @param acttions Reference to a list where actions will be put. The list
    /// is cleared before first use.
    void list_signals( QList<ActionInfo> & actions );

    /// Creates an object tree from a given node

    /// @param node Node to convert
    /// @return Retuns a shared pointer to the created node.
    /// @throw XmlError If the tree could not be built.
    static boost::shared_ptr< CNode > create_from_xml( common::XML::XmlNode node );

    /// Casts this node to a constant component of type TYPE.
    /// @return Returns the cast pointer
    /// @throw CastingFailed if the casting failed.
    template<class TYPE>
    Handle<const TYPE> castTo() const
    {
      Handle<const TYPE> self = handle<TYPE>();
      if(is_null(self))
        throw CastingFailed(FromHere(), "Failed to cast node " + name() + " to type " + TYPE::type_name());

      return self;
    }

    /// Casts this node to a component of type TYPE.
    /// @return Returns the cast pointer
    /// @throw CastingFailed if the casting failed.
    template<class TYPE>
    Handle<TYPE> castTo()
    {
      Handle<TYPE> self = handle<TYPE>();
      if(is_null(self))
        throw common::CastingFailed(FromHere(), "Failed to cast node " + name() + " to type " + TYPE::type_name());

      return self;
    }

    /// Connects a slot a signal provided by the internal notifier.
    /// @param receiver The receiver object, The object on which the slot will be
    /// called.
    /// @param signal The signal.
    /// @param slot The slot to connect.
    /// @see CNodeNotifier
    void connect_notifier( QObject * reciever,
                           const char * signal,
                           const char * slot );

    /// Adds a sub-node.

    /// This method is a wrapper for @c Component::add_component(). It calls
    /// the parent method, but emits
    /// @c CNodeNotifier::notifyadvancedModeChanged() on success.@c
    /// It is recommended to add child nodes using this method in order to
    /// guarantee the view is correctly updated.
    /// @param node Node to add.
    /// @throw common::ValueExists Forwards to the upper level any
    /// @c common::ValueExists exception thrown by
    /// @c Component::add_component()
    void add_node( boost::shared_ptr< CNode > node );

    /// Removes a sub-node.

    /// This method is a wrapper for @c Component::remove_component(). It calls
    /// the parent method, but emits
    /// @c CNodeNotifier::notify_child_count_changed() on success.@c
    /// It is recommended to remove child nodes using this method in order to
    /// guarantee the view is correctly updated.
    /// @param node_name Node to remove.
    /// @throw common::ValueNotFound Forwards to the upper level any
    /// @c common::ValueNotFound exception thrown by
    /// @c Component::remove_component()
    void remove_node( const QString & node_name );

    /// Gives the internal notifier.
    /// @return Returns the internal notifier.
    CNodeNotifier * notifier() const;

    /// Lists all children paths in a string list.

    /// Strings have the same format as returned by @c CPath::uri().string().
    /// @param list The string list where paths will be stored. The list is not
    /// cleaned before first use.
    /// @param recursive If @c true, the listing is recursive. Otherwise,
    /// only direct children are listed.
    /// @param client_node If @c true, client nodes are included into the the
    /// result. Otherwise, they are ignored.
    void list_child_paths( QStringList & list,
                           bool recursive,
                           bool client_node = true ) const;

    void request_signal_signature(const QString & name);

    /// @name Signals
    //@{

    /// Method called when a @e tree_update event occurs on the server.
    /// This methods calls @c NTree::update_tree() method to resquet an update
    /// of the tree.
    /// @param node Signal data. This parameter is not used.
    void reply_update_tree( common::SignalArgs & node);

    /// Method called when receiving a reply to a previously sent
    /// "configure" signal.
    /// @param node An XML representation of the modified options.
    void reply_configure(common::SignalArgs & node);

    /// Method called when the server replies to a @c list_content request.
    /// @param node Signal data.
    void reply_list_content( common::SignalArgs & node );

    /// Method called when the server replies to a signal
    void reply_signal_signature( common::SignalArgs & node );

    //@} END Signals

    /// Retrieves the signature of a local signal.

    /// @param name The signal name.
    /// @param node node @c SignalFrame where the signature will be stored.
    void local_signature( const QString & name, common::SignalArgs& node );

    void finish_setup();

    virtual void about_to_be_removed() {}

  protected: // data

    /// This internal notifier.
    CNodeNotifier * m_notifier;

    /// Lists the names of the local signals.
    QStringList m_local_signals;

    QMutex * m_mutex;

    /// @c false until the node content has been retrieved from
    /// the server.
    bool m_content_listed;

    /// Idicates whether this node is already waiting for content
    /// from the server.
    /// This is used to avoid sending multiple requests to the server
    /// in case it is overloaded and takes some time to reply.
    bool m_listing_content;

    /// Indicates whether this component is a NRoot.
    /// If @c true, this component is a NRoot object.
    bool m_is_root;

  private: // data

    /// Component type name.
    QString m_component_type;

    CNode::Type m_type;

  protected:

    /// List of signals that can be remotely executed
    QList<ActionInfo> m_action_sigs;

    /// Disables the local signals that need to.
    /// @param local_signals Map of local signals. The map is pre-initialiazed
    /// before calling this function with all local signals and the value set
    /// to @c true.
    virtual void disable_local_signals( QMap<QString, bool> & local_signals) const = 0;

    virtual void setup_finished() {}

  private: // helper functions

    /// Creates an object tree from a given node

    /// This is a recursive method. The second parameter holds, for each link
    /// the tree, the @c CPath of the target. The methods proceeds in this way
    /// since the target might not exist yet when the link exists. Furthermore,
    /// the method does not guarantee that each target path exists (e.i. the
    /// target is missing in the XML). It is up to the calling code to make
    /// that check.
    /// @param node Node to convert
    /// @param link_targets Map where links
    /// @return Retuns a shared pointer to the created node.
    /// @throw XmlError If the tree could not be built.
    static boost::shared_ptr< CNode > create_from_xml_recursive(common::XML::XmlNode & node,
                         QMap<boost::shared_ptr<NLink>, common::URI> & link_targets);

    void fetch_content();

  }; // class CNode

  ////////////////////////////////////////////////////////////////////////////

} // Core
} // ui
} // cf3

//////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_core_CNode_hpp
