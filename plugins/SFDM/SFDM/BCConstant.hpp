// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_BCConstant_hpp
#define cf3_SFDM_BCConstant_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/StringConversion.hpp"
#include "common/OptionList.hpp"
#include "common/Option.hpp"
#include "common/Component.hpp"
#include "SFDM/BCWeak.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {

////////////////////////////////////////////////////////////////////////////////

template <Uint NEQS, Uint NDIM>
class BCConstant : public BCWeak< BCPointData<NEQS,NDIM> >
{
public:
  static std::string type_name() { return "BCConstant<"+common::to_str(NEQS)+","+common::to_str(NDIM)+">"; }

  BCConstant(const std::string& name) :
    BCWeak< BCPointData<NEQS,NDIM> >(name)
  {
    common::Component::options().add_option("constants",std::vector<Real>())
        .link_to(&m_constants);
  }

  virtual ~BCConstant() {}

  virtual void compute_solution(const BCPointData<NEQS,NDIM>& inner_cell_data, Eigen::Matrix<Real,NEQS,1>& boundary_face_solution)
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

} // SFDM
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_SFDM_BCConstant_hpp
