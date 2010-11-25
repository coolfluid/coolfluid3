// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CLoop_hpp
#define CF_Mesh_CLoop_hpp

#include "Actions/CAction.hpp"
#include "Actions/CLoopOperation.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  class CRegion;
}
namespace Actions {

  using namespace CF::Mesh;

/////////////////////////////////////////////////////////////////////////////////////

class Actions_API CLoop : public CAction
{
public: // typedefs

  /// provider
  typedef boost::shared_ptr< CLoop > Ptr;
  typedef boost::shared_ptr< CLoop const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CLoop ( const std::string& name );

  void trigger_Regions();

  /// Virtual destructor
  virtual ~CLoop() {}

  /// Get the class name
  static std::string type_name () { return "CLoop"; }

  /// Configuration Options
  virtual void define_config_properties ();

  // functions specific to the CLoop component

  CLoopOperation& create_action(const std::string action_provider);

  virtual const CLoopOperation& action(const std::string& name) const;

  virtual CLoopOperation& action(const std::string& name);

  virtual void execute() = 0;

private: // helper functions

  /// regists all the signals declared in this class
  virtual void define_signals () {}

protected:

  /// Regions to loop over
  std::vector<boost::shared_ptr<CRegion> > m_loop_regions;

};

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CLoop_hpp
