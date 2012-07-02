// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_core_Plugins_hpp
#define cf3_ui_core_Plugins_hpp

////////////////////////////////////////////////////////////////////////////

#include "ui/core/NPlugin.hpp"

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace core {

////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////

class NPlugins : public CNode
{
public: // typedefs




public:

  NPlugins(const std::string & name);

  virtual ~NPlugins();

  template<typename LIB>
  Handle< NPlugin > register_plugin()
  {
    // a plugin cannot be registered twice
    cf3_assert( is_null(get_child(LIB::library_name())) );

    Handle<NPlugin> plugin = create_component<NPlugin>( LIB::library_name() );

    plugin->mark_basic();

    return plugin;
  }

  template<typename LIB>
  bool is_registered_plugin()
  {
    return is_not_null(get_child(LIB::library_name()));
  }

  template<typename LIB>
  Handle< NPlugin > plugin()
  {
    return get_child_checked( LIB::library_name() )->template handle<NPlugin>();
  }

  virtual QString tool_tip() const;

  static Handle<NPlugins> global();

protected:

  virtual void disable_local_signals(QMap<QString, bool> &localSignals) const {}

}; // NPlugins

////////////////////////////////////////////////////////////////////////////

} // Core
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_core_Plugins_hpp
