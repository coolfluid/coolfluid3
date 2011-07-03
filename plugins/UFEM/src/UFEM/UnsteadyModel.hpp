// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UFEM_UnsteadyModel_hpp
#define CF_UFEM_UnsteadyModel_hpp

#include <boost/scoped_ptr.hpp>

#include "LibUFEM.hpp"
#include "Model.hpp"

namespace CF {
  namespace Solver {
    namespace Actions { namespace Proto { template<class T > struct StoredReference; } }
  class CTime; }
namespace UFEM {

/// Base class for unsteady problems
class UFEM_API UnsteadyModel : public Model
{
public: // typedefs

  typedef boost::shared_ptr<UnsteadyModel> Ptr;
  typedef boost::shared_ptr<UnsteadyModel const> ConstPtr;

public: // functions
  
  /// Contructor
  /// @param name of the component
  UnsteadyModel ( const std::string& name );
  
  virtual ~UnsteadyModel();

  /// Get the class name
  static std::string type_name () { return "UnsteadyModel"; }
  
  virtual LinearProblem& problem();
  
  /// The component that keeps the time for our solution (created on model construction)
  Solver::CTime& time();
  
  /// Quick access to the inverse of the timestep
  Solver::Actions::Proto::StoredReference<Real> invdt() const;
  
private:
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;
};

} // UFEM
} // CF


#endif // CF_UFEM_UnsteadyModel_hpp
