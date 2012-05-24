// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_BCExtrapolate_hpp
#define cf3_sdm_BCExtrapolate_hpp

////////////////////////////////////////////////////////////////////////////////

#include "sdm/BCWeak.hpp"
#include "sdm/PhysDataBase.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {

//////////////////////////////////////////////////////////////////////////////

/// Extrapolation boundary condition
/// type names:   cf3.sdm.BCExtrapolate<NEQS,NDIM>
/// @author Willem Deconinck
template <Uint NEQS, Uint NDIM>
class sdm_API BCExtrapolate : public BCWeak< PhysDataBase<NEQS,NDIM> >
{
public:
  static std::string type_name() { return "BCExtrapolate<"+common::to_str(NEQS)+","+common::to_str(NDIM)+">"; }
  BCExtrapolate(const std::string& name) : BCWeak< PhysDataBase<NEQS,NDIM> >(name)
  {
  }
  virtual ~BCExtrapolate() {}

  virtual void compute_solution(const PhysDataBase<NEQS,NDIM>& inner_cell_data, const Eigen::Matrix<Real,NDIM,1>& unit_normal, Eigen::Matrix<Real,NEQS,1>& boundary_face_pt_data)
  {
    boundary_face_pt_data = inner_cell_data.solution;
  }
};

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_BCExtrapolate_hpp
