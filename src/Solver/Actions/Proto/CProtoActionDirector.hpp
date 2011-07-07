// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_CProtoActionDirector_hpp
#define CF_Solver_Actions_CProtoActionDirector_hpp

#include <boost/scoped_ptr.hpp>

#include "Common/CActionDirector.hpp"

#include "CProtoAction.hpp"

namespace CF {
  namespace Common { template<typename T> class OptionComponent; }
  namespace Mesh { class CMesh; class CRegion; }
namespace Solver {
  class CPhysicalModel;
namespace Actions {
namespace Proto {
 
/// Provides functionality to execute multiple proto actions in sequence
/// Standard options are physical_model and region, which are passed on to the stored
/// proto actions. Convenience methods to easily create actions are also made available.
class CProtoActionDirector : public Common::CActionDirector
{
public:
  typedef boost::shared_ptr<CProtoActionDirector> Ptr;
  typedef boost::shared_ptr<CProtoActionDirector const> ConstPtr;
  
  CProtoActionDirector(const std::string& name);
  /// Destructor needs to be defined in the .cpp for scoped_ptr to work (class Implementation must be complete at destruction time)
  virtual ~CProtoActionDirector();
  
  static std::string type_name() { return "CProtoActionDirector"; }
  
  /// Create an action and store it as a child. It is NOT added to the list
  /// of actions to execute automatically (use append or the << operator for that)
  CAction& add_action(const std::string& name, const boost::shared_ptr<Expression>& expression);

protected:
  /// Reference to the physical model. A user friendly error is thrown if it's not available
  CPhysicalModel& physical_model();
  
private:
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;
};

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Actions_CProtoActionDirector_hpp
