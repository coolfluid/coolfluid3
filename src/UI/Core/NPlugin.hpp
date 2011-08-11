// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef CF_UI_Core_NPlugin_hpp
#define CF_UI_Core_NPlugin_hpp

////////////////////////////////////////////////////////////////////////////

#include "Common/Signal.hpp"

#include "UI/Core/CNode.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Core {

////////////////////////////////////////////////////////////////////////////

class NPlugin : public CNode
{

public: // typedefs

  typedef boost::shared_ptr<NPlugin> Ptr;
  typedef boost::shared_ptr<const NPlugin> ConstPtr;

public:

  NPlugin(const std::string & name);

  virtual ~NPlugin();

  virtual QString toolTip() const;

  Common::SignalPtr addSignal( const std::string& name,
                               const std::string& descr = std::string(),
                               const std::string& readable_name = std::string() );

protected:

  virtual void disableLocalSignals( QMap<QString, bool> &localSignals ) const {}

}; // NPlugin

////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_UI_Core_NPlugin_hpp
