#ifndef CF_GUI_Client_CBrowser_hpp
#define CF_GUI_Client_CBrowser_hpp

//////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

class QString;

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

  ////////////////////////////////////////////////////////////////////////////

  class CBrowser : public CF::Common::Component
  {
  public:

    typedef boost::shared_ptr<CBrowser> Ptr;
    typedef boost::shared_ptr<CBrowser const> ConstPtr;

    CBrowser();

    QString generateName();

  private:

      CF::Uint m_counter;

  }; // class CBrowser

  ////////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

#endif // CBROWSER_HPP
