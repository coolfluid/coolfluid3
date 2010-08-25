#ifndef CF_GUI_Client_NGroup_hpp
#define CF_GUI_Client_NGroup_hpp

////////////////////////////////////////////////////////////////////////////

#include <QObject>

#include "GUI/Client/CNode.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

  //////////////////////////////////////////////////////////////////////////

  /// @brief Client corresponding component for @c CF::Common::CGroup.
  class NGroup :
      public QObject,
      public CNode
  {
    Q_OBJECT

  public:

    typedef boost::shared_ptr<NGroup> Ptr;
    typedef boost::shared_ptr<NGroup const> ConstPtr;

    /// @brief Constructor
    /// @param name Group name
    NGroup(const QString & name);

    /// @brief Gives the icon associated to this node
    /// @return Returns the icon associated to this node
    /// @note This method should be reimplemented by all subclasses.
    virtual QIcon getIcon() const;

    /// @brief Gives the text to put on a tool tip
    /// @return The name of the class.
    virtual QString getToolTip() const;

  private:

    /// regists all the signals declared in this class
    static void regist_signals ( Component* self ) {}

  }; // class NGroup

  //////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_NGroup_hpp
