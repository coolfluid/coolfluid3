// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_BodyForceControl_hpp
#define cf3_UFEM_BodyForceControl_hpp

#include "solver/actions/Proto/ProtoAction.hpp"
#include "mesh/Region.hpp"


#include "LibUFEM.hpp"

namespace cf3 {
  namespace solver { class Time; }
namespace UFEM {

/// Boundary condition to hold the value of a field at a value given by another (or the same) field
class UFEM_API BodyForceControl : public solver::actions::Proto::ProtoAction
{
public:
  /// Contructor
  /// @param name of the component
  BodyForceControl ( const std::string& name );

  virtual ~BodyForceControl();

  /// Get the class name
  static std::string type_name () { return "BodyForceControl"; }

  virtual void execute();

private:
  std::vector<Real> m_uRef;
  RealVector uInteg;
  RealVector m_correction;
  Handle<solver::Time> m_time;
  Real m_dt;
  Real aCoef;
  Real bCoef;

  /// Wall regions to operate over
  std::vector< Handle<mesh::Region> > m_surface_regions;

};

} // UFEM
} // cf3


#endif // cf3_UFEM_BodyForceControl_hpp
