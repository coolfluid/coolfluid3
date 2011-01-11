// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_CLoop_hpp
#define CF_Solver_Actions_CLoop_hpp

#include "Common/CAction.hpp"

#include "Solver/Actions/LibActions.hpp"
#include "Solver/Actions/CLoopOperation.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Mesh
  {
    class CRegion;
  }

namespace Solver {
namespace Actions {


/////////////////////////////////////////////////////////////////////////////////////

class Solver_Actions_API CLoop : public Common::CAction
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

  // functions specific to the CLoop component

  CLoopOperation& create_action(const std::string action_provider);

  virtual const CLoopOperation& action(const std::string& name) const;

  virtual CLoopOperation& action(const std::string& name);

  virtual void execute() = 0;

protected:

  /// Regions to loop over
  std::vector<boost::shared_ptr<Mesh::CRegion> > m_loop_regions;

};

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_Actions_CLoop_hpp
