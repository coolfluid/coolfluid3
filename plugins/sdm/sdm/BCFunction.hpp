// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_BCFunction_hpp
#define cf3_sdm_BCFunction_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "math/VectorialFunction.hpp"

#include "sdm/BCWeak.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {

//////////////////////////////////////////////////////////////////////////////

template <Uint NEQS, Uint NDIM>
class BCFunction : public BCWeak< PhysDataBase<NEQS,NDIM> >
{
public:
  static std::string type_name() { return "BCFunction<"+common::to_str(NEQS)+","+common::to_str(NDIM)+">"; }
  BCFunction(const std::string& name) : BCWeak< PhysDataBase<NEQS,NDIM> >(name)
  {
    common::Component::options().add("functions", std::vector<std::string>())
        .description("math function applied as boundary condition (vars x,y,z)")
        .pretty_name("Functions")
        .attach_trigger ( boost::bind ( &BCFunction::config_function, this ) )
        .mark_basic();

    m_functions.variables("x,y,z");
    params.resize(3,0.);
  }

  void config_function()
  {
    m_functions.functions( common::Component::options()["functions"].template value<std::vector<std::string> >() );
    m_functions.parse();
  }

  virtual ~BCFunction() {}

  virtual void compute_solution(const PhysDataBase<NEQS,NDIM>& inner_cell_data, const Eigen::Matrix<Real,NDIM,1>& unit_normal, Eigen::Matrix<Real,NEQS,1>& boundary_face_solution)
  {
    for (Uint d=0; d<NDIM; ++d)
      params[d] = inner_cell_data.coord[d];
    m_functions.evaluate(params,boundary_face_solution);
  }

private:

  math::VectorialFunction  m_functions;
  std::vector<Real> params;
};

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_BCFunction_hpp
