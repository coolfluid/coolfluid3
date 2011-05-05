// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UI_Core_Plugins_hpp
#define CF_UI_Core_Plugins_hpp

////////////////////////////////////////////////////////////////////////////

#include "UI/Core/NPlugin.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
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
  NPlugin::Ptr registerPlugin()
  {
    // a plugin cannot be registered twice
    cf_assert( m_components.find(LIB::library_name()) == m_components.end() );

    NPlugin::Ptr plugin = create_component_ptr<NPlugin>( LIB::library_name() );

    plugin->mark_basic();

    return plugin;
  }

  template<typename LIB>
  bool isRegisteredPlugin()
  {
    return m_components.find(LIB::library_name()) != m_components.end();
  }

  template<typename LIB>
  NPlugin::Ptr plugin()
  {
    return get_child_ptr_checked( LIB::library_name() )->as_ptr_checked<NPlugin>();
  }

  virtual QString toolTip() const;

  static Ptr globalPlugins();

protected:

  virtual void disableLocalSignals(QMap<QString, bool> &localSignals) const {}

}; // NPlugins

////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_UI_Core_Plugins_hpp
