#ifndef CF_GUI_Client_NTable_hpp
#define CF_GUI_Client_NTable_hpp

////////////////////////////////////////////////////////////////////////////

#include <QObject>

#include "GUI/Client/CNode.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

  //////////////////////////////////////////////////////////////////////////

  /// @brief Client corresponding component for @c CF::Mesh::CTable.
  class NTable :
      public QObject,
      public CNode
  {
    Q_OBJECT

  public:

    typedef boost::shared_ptr<NTable> Ptr;
    typedef boost::shared_ptr<NTable const> ConstPtr;

    /// @brief Constructor
    /// @param name Group name
    NTable(const QString & name);

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

  }; // class NTable

  //////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_NTable_hpp
