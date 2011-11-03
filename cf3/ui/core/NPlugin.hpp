// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef cf3_ui_core_NPlugin_hpp
#define cf3_ui_core_NPlugin_hpp

////////////////////////////////////////////////////////////////////////////

#include "common/Signal.hpp"

#include "ui/core/CNode.hpp"

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace core {

////////////////////////////////////////////////////////////////////////////

class NPlugin : public CNode
{

public: // typedefs

  typedef boost::shared_ptr<NPlugin> Ptr;
  typedef boost::shared_ptr<const NPlugin> ConstPtr;

public:

  NPlugin(const std::string & name);

  virtual ~NPlugin();

  virtual QString tool_tip() const;

  common::SignalPtr add_signal( const std::string& name,
                                const std::string& descr = std::string(),
                                const std::string& readable_name = std::string() );

protected:

  virtual void disable_local_signals( QMap<QString, bool> &localSignals ) const {}

}; // NPlugin

////////////////////////////////////////////////////////////////////////////

} // Core
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_core_NPlugin_hpp
