#ifndef CF_GUI_Client_NBrowser_hpp
#define CF_GUI_Client_NBrowser_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QList>

#include "GUI/Client/CNode.hpp"

class QString;
class QIcon;

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

  ////////////////////////////////////////////////////////////////////////////

  class NBrowser : public CNode
  {
  public:

    typedef boost::shared_ptr<NBrowser> Ptr;
    typedef boost::shared_ptr<NBrowser const> ConstPtr;

    NBrowser();

    QString generateName();

    /// @brief Gives the icon associated to this node
    /// @return Returns the icon associated to this node
    /// @note This method should be reimplemented by all subclasses.
    virtual QIcon getIcon() const;

    virtual QString getToolTip() const;

    virtual void getOptions(QList<NodeOption> & params) const;

  private:

      CF::Uint m_counter;

      /// regists all the signals declared in this class
      static void regist_signals ( Component* self ) {}

  }; // class NBrowser

  ////////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

#endif // NBrowser_HPP
