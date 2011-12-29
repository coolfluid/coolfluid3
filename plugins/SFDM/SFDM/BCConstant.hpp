// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_BCConstant_hpp
#define cf3_SFDM_BCConstant_hpp

////////////////////////////////////////////////////////////////////////////////

#include "SFDM/BCStrong.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {

////////////////////////////////////////////////////////////////////////////////

class SFDM_API BCConstant : public BCStrong
{
public:
  static std::string type_name() { return "BCConstant"; }
  BCConstant(const std::string& name) : BCStrong(name)
  {
    options().add_option("constants",std::vector<Real>())
        .link_to(&m_constants);
  }
  virtual ~BCConstant() {}

  virtual void compute_solution()
  {
    for (Uint v=0; v<flx_pt_solution->get()[cell_flx_pt].size(); ++v)
    {
      flx_pt_solution->get()[cell_flx_pt][v] = m_constants[v];
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
