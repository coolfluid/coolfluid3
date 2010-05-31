#ifndef CF_GUI_Client_NLink_hpp
#define CF_GUI_Client_NLink_hpp

//////////////////////////////////////////////////////////////////////////////

#include "GUI/Client/CNode.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

  ////////////////////////////////////////////////////////////////////////////

  class NLink :
      public CNode
  {
    Q_OBJECT

  public:

    typedef boost::shared_ptr<NLink> Ptr;

    /// @brief Constructor
    /// @param name Node name
    NLink(const QString & name);

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

  }; // class NLink

//////////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_NLink_hpp
