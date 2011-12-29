// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_StrongBC_hpp
#define cf3_SFDM_StrongBC_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/PropertyList.hpp"

#include "math/MatrixTypes.hpp"
#include "math/VectorialFunction.hpp"

#include "SFDM/Tags.hpp"
#include "SFDM/BC.hpp"
#include "SFDM/ShapeFunction.hpp"
#include "SFDM/Operations.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {

//////////////////////////////////////////////////////////////////////////////

/// @author Willem Deconinck
class SFDM_API StrongBC : public BC
{
public: // typedefs
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
public: // functions

  /// constructor
  StrongBC( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "StrongBC"; }

  virtual void execute();

private: // functions

protected: // configuration

  virtual void initialize()
  {
    BC::initialize();
    reconstruct_in_solution_points = shared_caches().get_cache< FluxPointReconstruct >();
    flx_pt_solution              = shared_caches().get_cache< FluxPointFieldDyn >(SFDM::Tags::solution()+std::string("Dyn"));

    reconstruct_in_solution_points->options().configure_option("space",solution_field().space());
    flx_pt_solution->options().configure_option("field",solution_field().uri());
  }

  virtual void set_face_entities(const mesh::Entities& entities)
  {
    BC::set_face_entities(entities);
  }

  virtual void set_face_element(const Uint elem_idx)
  {
    BC::set_face_element(elem_idx);
  }

  virtual void unset_face_element()
  {
    BC::unset_face_element();
  }

  virtual void compute_inner_cell(const Handle<mesh::Entities const>& cell_entities, const Uint cell_idx)
  {
    flx_pt_solution->cache(cell_entities,cell_idx);
  }

  virtual void compute_solution() = 0;

protected: // fast-access-data (for convenience no "m_" prefix)

  // Data


  Handle<mesh::Entities const> cell_entities;
  Uint cell_idx;
  Uint cell_face_nb;
  Uint cell_flx_pt;

  // In flux points:
  Handle< CacheT<FluxPointFieldDyn> > flx_pt_solution;
  Handle< CacheT<FluxPointReconstruct> > reconstruct_in_solution_points;

}; // end StrongBC

////////////////////////////////////////////////////////////////////////////////

StrongBC::StrongBC( const std::string& name )
  : BC(name)
{
  properties()["brief"] = std::string("Convective Spectral Finite Difference BC");
  properties()["description"] = std::string("Fields to be created: ...");
}

/////////////////////////////////////////////////////////////////////////////

void StrongBC::execute()
{
  set_inner_cell(m_entities,m_elem_idx,cell_entities,cell_idx,cell_face_nb);
  compute_inner_cell(cell_entities,cell_idx);
  boost_foreach(cell_flx_pt, flx_pt_solution->get().sf->face_flx_pts(cell_face_nb))
  {
    compute_solution();
  }
  mesh::Field::View sol_pt_solution = solution_field().view(flx_pt_solution->get().space->indexes_for_element(cell_idx));
  reconstruct_in_solution_points->cache(cell_entities).
      compute(flx_pt_solution->get().sf->flx_pt_dirs(flx_pt_solution->get().sf->face_flx_pts(cell_face_nb)[0])[0],
                                        flx_pt_solution->get().field_in_flx_pts,sol_pt_solution);
  flx_pt_solution->get().unlock();
}

////////////////////////////////////////////////////////////////////////////////

class SFDM_API BCConstant : public StrongBC
{
public:
  static std::string type_name() { return "BCConstant"; }
  BCConstant(const std::string& name) : StrongBC(name)
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

} // SFDM
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_SFDM_StrongBC_hpp
