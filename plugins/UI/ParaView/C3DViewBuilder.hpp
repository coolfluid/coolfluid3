// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UI_ParaView_C3DVIEWBUILDER_HPP
#define CF_UI_ParaView_C3DVIEWBUILDER_HPP

// header
#include "Common/Component.hpp"
#include "UI/ParaView/C3DView.hpp"
#include "UI/ParaView/LibParaView.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace ParaView {

////////////////////////////////////////////////////////////////////////////////

  /// @brief C3DView class builder.
  /// @author Wertz Gil
class ParaView_API C3DViewBuilder :
    public Common::Component
{
public: // typedefs

  typedef boost::shared_ptr<C3DViewBuilder> Ptr;
  typedef boost::shared_ptr<C3DViewBuilder const> ConstPtr;

public:

  /// Constructor
  /// @param name Name of the node.
  C3DViewBuilder(const std::string & name);

  /// Get the class name
  static std::string type_name() { return "C3DViewBuilder"; }

  /// @name SIGNALS
  //@{

  void signal_create_3dview( Common::SignalArgs & args);

  void signature_create_3dview( Common::SignalArgs & args);

  //@} END SIGNALS

private: // data

  //process id
  //port
  //host
  //path

}; // C3DViewBuilder

////////////////////////////////////////////////////////////////////////////////

} // ParaView
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_UI_ParaView_C3DVIEWBUILDER_HPP
