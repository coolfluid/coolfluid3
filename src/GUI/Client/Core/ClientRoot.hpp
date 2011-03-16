// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_Core_ClientRoot_hpp
#define CF_GUI_Client_Core_ClientRoot_hpp

//////////////////////////////////////////////////////////////////////////////

#include "GUI/Client/Core/NRoot.hpp"
#include "GUI/Network/ComponentNames.hpp"

#include "GUI/Client/Core/LibClientCore.hpp"


//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientCore {

////////////////////////////////////////////////////////////////////////////

/// @brief Manages the client root node

/// This class ensures that the root node is available from anywhere, at
/// anytime.
/// @author Quentin Gasper.

class ClientCore_API ClientRoot
{
public:

  static ClientRoot & instance();

  /// @brief Gives the root UUID.
  /// @return Returns the root UUID.
  inline std::string getUUID() { return m_root->uuid(); }

  template<typename TYPE>
  typename TYPE::Ptr rootChild(const std::string & name) const
  {
    return m_root->root()->get_child_ptr(name)->as_ptr<TYPE>();
  }

  NRoot::ConstPtr root() const { return m_root; }

  NRoot::Ptr root() { return m_root; }

  void newSignal(Common::XML::XmlDoc::Ptr doc);

private:

  NRoot::Ptr m_root;

  ClientRoot();

}; // ClientRoot

////////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_Core_ClientRoot_hpp
