// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CLoop_hpp
#define CF_Mesh_CLoop_hpp

#include "Actions/CAction.hpp"
#include "Actions/CElementOperation.hpp"

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
  typedef Common::ConcreteProvider < CAction , Common::NB_ARGS_1 > PROVIDER;

  /// provider
  typedef boost::shared_ptr< CLoop > Ptr;
  typedef boost::shared_ptr< CLoop const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CLoop ( const CName& name );

  void trigger_Regions();

  /// Virtual destructor
  virtual ~CLoop() {}

  /// Get the class name
  static std::string type_name () { return "CLoop"; }

  /// Configuration Options
  static void defineConfigProperties ( Common::PropertyList& options );

  // functions specific to the CLoop component

  CElementOperation& create_action(const std::string action_provider);

  virtual const CElementOperation& action(const CName& name) const;

  virtual CElementOperation& action(const CName& name);

  virtual void execute() = 0;

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

protected:

  /// Regions to loop over
  std::vector<boost::shared_ptr<CRegion> > m_loop_regions;

};

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CLoop_hpp
