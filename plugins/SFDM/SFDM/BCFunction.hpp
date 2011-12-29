// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_BCFunction_hpp
#define cf3_SFDM_BCFunction_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "SFDM/BCStrong.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {

//////////////////////////////////////////////////////////////////////////////

class SFDM_API BCFunction : public BCStrong
{
public:
  static std::string type_name() { return "BCFunction"; }
  BCFunction(const std::string& name) : BCStrong(name)
  {
    options().add_option("functions", std::vector<std::string>())
        .description("math function applied as boundary condition (vars x,y,z)")
        .pretty_name("Functions")
        .attach_trigger ( boost::bind ( &BCFunction::config_function, this ) )
        .mark_basic();

    m_functions.variables("x,y,z");
    params.resize(3,0.);
  }

  void config_function()
  {
    m_functions.functions( options()["functions"].value<std::vector<std::string> >() );
    m_functions.parse();
  }

  virtual ~BCFunction() {}

  virtual void compute_solution()
  {
    for (Uint d=0; d<flx_pt_coordinates->get()[cell_flx_pt].size(); ++d)
      params[d] = flx_pt_coordinates->get()[cell_flx_pt][d];
    m_functions.evaluate(params,flx_pt_solution->get()[cell_flx_pt]);
  }

  virtual void initialize()
  {
    BCStrong::initialize();
    flx_pt_coordinates = shared_caches().get_cache<FluxPointCoordinatesDyn>();
    flx_pt_coordinates->options().configure_option("space",solution_field().space());
  }

  virtual void set_inner_cell()
  {
    BCStrong::set_inner_cell();
    flx_pt_coordinates->cache(cell_entities,cell_idx);
  }

  virtual void unset_inner_cell()
  {
    BCStrong::unset_inner_cell();
    flx_pt_coordinates->get().unlock();
  }

private:

  Handle< CacheT< FluxPointCoordinatesDyn > > flx_pt_coordinates;
  math::VectorialFunction  m_functions;
  std::vector<Real> params;
};

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_SFDM_BCFunction_hpp
