// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_lineuler_FixMeanFlowBoundaryLayer_hpp
#define cf3_sdm_lineuler_FixMeanFlowBoundaryLayer_hpp

////////////////////////////////////////////////////////////////////////////////

#include "sdm/BCStrong.hpp"

#include "sdm/lineuler/LibLinEuler.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace lineuler {

////////////////////////////////////////////////////////////////////////////////

class sdm_lineuler_API SetMeanVelocityZero : public BCStrong< PhysDataBase<4u,2u> >
{
public:
  static std::string type_name() { return "SetMeanVelocityZero"; }
  SetMeanVelocityZero(const std::string& name) : BCStrong< PhysData >(name)
  {
  }

  virtual ~SetMeanVelocityZero() {}

private:

  virtual void compute_solution(const PhysData &inner_cell_data, const RealVectorNDIM &boundary_face_normal, RealVectorNEQS &boundary_face_solution)
  {
    boundary_face_solution[0] = inner_cell_data.solution[0];
    boundary_face_solution[1] = 0.;
    boundary_face_solution[2] = 0.;
    boundary_face_solution[3] = inner_cell_data.solution[3];
  }
};

////////////////////////////////////////////////////////////////////////////////

} // lineuler
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_lineuler_FixMeanFlowBoundaryLayer_hpp
