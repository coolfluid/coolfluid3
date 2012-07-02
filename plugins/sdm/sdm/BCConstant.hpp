// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_BCConstant_hpp
#define cf3_sdm_BCConstant_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/StringConversion.hpp"
#include "common/OptionList.hpp"
#include "common/Option.hpp"
#include "common/Component.hpp"
#include "sdm/BCWeak.hpp"
#include "sdm/BCStrong.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {

////////////////////////////////////////////////////////////////////////////////

template <Uint NEQS, Uint NDIM>
class BCConstant : public BCWeak< PhysDataBase<NEQS,NDIM> >
{
public:
  static std::string type_name() { return "BCConstant<"+common::to_str(NEQS)+","+common::to_str(NDIM)+">"; }

  BCConstant(const std::string& name) :
    BCWeak< PhysDataBase<NEQS,NDIM> >(name)
  {
    common::Component::options().add("constants",std::vector<Real>(0.,NEQS))
        .link_to(&m_constants).mark_basic();
  }

  virtual ~BCConstant() {}

  virtual void compute_solution(const PhysDataBase<NEQS,NDIM>& inner_cell_data, const Eigen::Matrix<Real,NDIM,1>& unit_normal, Eigen::Matrix<Real,NEQS,1>& boundary_face_solution)
  {
    for (Uint v=0; v<NEQS; ++v)
    {
      boundary_face_solution[v] = m_constants[v];
    }
  }

private:
  std::vector<Real> m_constants;
};

////////////////////////////////////////////////////////////////////////////////

template <Uint NEQS, Uint NDIM>
class BCConstantStrong : public BCStrong< PhysDataBase<NEQS,NDIM> >
{
public:
  static std::string type_name() { return "BCConstantStrong<"+common::to_str(NEQS)+","+common::to_str(NDIM)+">"; }

  BCConstantStrong(const std::string& name) :
    BCStrong< PhysDataBase<NEQS,NDIM> >(name)
  {
    common::Component::options().add("constants",std::vector<Real>(0.,NEQS))
        .link_to(&m_constants).mark_basic();
  }

  virtual ~BCConstantStrong() {}

  virtual void compute_solution(const PhysDataBase<NEQS,NDIM>& inner_cell_data, const Eigen::Matrix<Real,NDIM,1>& unit_normal, Eigen::Matrix<Real,NEQS,1>& boundary_face_solution)
  {
    for (Uint v=0; v<NEQS; ++v)
    {
      boundary_face_solution[v] = m_constants[v];
    }
  }

private:
  std::vector<Real> m_constants;
};


} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_BCConstant_hpp
