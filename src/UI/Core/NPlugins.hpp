// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UI_Core_Plugins_hpp
#define CF_UI_Core_Plugins_hpp

////////////////////////////////////////////////////////////////////////////

#include "UI/Core/CNode.hpp"

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

  NPlugins(const QString & name);

  virtual ~NPlugins();

  void registerPlugin( /* something */ );

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
