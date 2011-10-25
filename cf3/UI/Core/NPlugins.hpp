// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UI_Core_Plugins_hpp
#define cf3_UI_Core_Plugins_hpp

////////////////////////////////////////////////////////////////////////////

#include "UI/Core/NPlugin.hpp"

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Core {

////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////

class NPlugins : public CNode
{
public: // typedefs

  typedef boost::shared_ptr<NPlugins> Ptr;
  typedef boost::shared_ptr<const NPlugins> ConstPtr;

public:

  NPlugins(const std::string & name);

  virtual ~NPlugins();

  template<typename LIB>
  NPlugin::Ptr register_plugin()
  {
    // a plugin cannot be registered twice
    cf3_assert( m_components.find(LIB::library_name()) == m_components.end() );

    NPlugin::Ptr plugin = create_component_ptr<NPlugin>( LIB::library_name() );

    plugin->mark_basic();

    return plugin;
  }

  template<typename LIB>
  bool is_registered_plugin()
  {
    return m_components.find(LIB::library_name()) != m_components.end();
  }

  template<typename LIB>
  NPlugin::Ptr plugin()
  {
    return get_child_ptr_checked( LIB::library_name() )->as_ptr_checked<NPlugin>();
  }

  virtual QString tool_tip() const;

  static Ptr global();

protected:

  virtual void disable_local_signals(QMap<QString, bool> &localSignals) const {}

}; // NPlugins

////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_UI_Core_Plugins_hpp
