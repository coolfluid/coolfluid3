// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UFEM_SteadyModel_hpp
#define CF_UFEM_SteadyModel_hpp

#include <boost/scoped_ptr.hpp>

#include "LibUFEM.hpp"
#include "Model.hpp"

namespace CF {

namespace UFEM {

/// Special case of steady FEM problems: only one LSS solve is needed to get the solution of the problem
/// Useful for i.e. linear heat conduction
class UFEM_API SteadyModel : public Model
{
public: // typedefs

  typedef boost::shared_ptr<SteadyModel> Ptr;
  typedef boost::shared_ptr<SteadyModel const> ConstPtr;

public: // functions
  
  /// Contructor
  /// @param name of the component
  SteadyModel ( const std::string& name );
  
  virtual ~SteadyModel();

  /// Get the class name
  static std::string type_name () { return "SteadyModel"; }
  
  virtual LinearProblem& problem();
  
private:
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;
};

} // UFEM
} // CF


#endif // CF_UFEM_SteadyModel_hpp
