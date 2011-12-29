// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_ConvectiveTerm_hpp
#define cf3_SFDM_ConvectiveTerm_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/PropertyList.hpp"

#include "math/MatrixTypes.hpp"
#include "math/VectorialFunction.hpp"

#include "SFDM/Tags.hpp"
#include "SFDM/Term.hpp"
#include "SFDM/ShapeFunction.hpp"
#include "SFDM/Operations.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {

//////////////////////////////////////////////////////////////////////////////

/// A proposed base class for simple convective terms.
/// Classes inheriting only need to implement functions to compute
/// - analytical flux for interior flux points
/// - numerical flux for face flux points
/// @author Willem Deconinck
template <Uint NEQS, Uint NDIM>
class ConvectiveTerm : public Term
{
public: // typedefs
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
public: // functions

  /// constructor
  ConvectiveTerm( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "ConvectiveTerm"; }

  virtual void execute();

private: // functions

  // Flux evaluations
  // ----------------
  virtual void compute_analytical_flux(const Eigen::Matrix<Real,NDIM,1>& unit_normal) = 0;
  virtual void compute_numerical_flux(const Eigen::Matrix<Real,NDIM,1>& unit_normal) = 0;

protected: // configuration

  virtual void initialize()
  {
    Term::initialize();
    elem                  = shared_caches().template get_cache< SFDElement >();
    divergence_in_solution_points  = shared_caches().template get_cache< FluxPointDivergence >();
    reconstruct_in_solution_points = shared_caches().template get_cache< FluxPointReconstruct >();
    flx_pt_solution              = shared_caches().template get_cache< FluxPointField<NEQS,NDIM> >(SFDM::Tags::solution());
    flx_pt_neighbour_solution    = shared_caches().template get_cache< FluxPointField<NEQS,NDIM> >(std::string("neighbour_")+SFDM::Tags::solution());
    flx_pt_plane_jacobian_normal = shared_caches().template get_cache< FluxPointPlaneJacobianNormal<NDIM> >();

    elem->options().configure_option("space",solution_field().space());
    divergence_in_solution_points->options().configure_option("space",solution_field().space());
    reconstruct_in_solution_points->options().configure_option("space",solution_field().space());
    flx_pt_solution->options().configure_option("field",solution_field().uri());
    flx_pt_neighbour_solution->options().configure_option("field",solution_field().uri());
    flx_pt_plane_jacobian_normal->options().configure_option("space",solution_field().space());
  }

  virtual void set_entities(const mesh::Entities& entities)
  {
    Term::set_entities(entities);

    elem->cache(m_entities);
    flx_pt_plane_jacobian_normal->cache(m_entities);
    divergence_in_solution_points->cache(m_entities);
    reconstruct_in_solution_points->cache(m_entities);
    flx_pt_solution->cache(m_entities);

    sol_pt_wave_speed.resize(NDIM,std::vector<Eigen::Matrix<Real,1,1> >(elem->get().sf->nb_sol_pts()));
    flx_pt_wave_speed.resize(elem->get().sf->nb_flx_pts());
    flx_pt_flux.resize(elem->get().sf->nb_flx_pts());
  }

  virtual void set_element(const Uint elem_idx)
  {
    Term::set_element(elem_idx);

    flx_pt_plane_jacobian_normal->get().compute_element(m_elem_idx);
    flx_pt_solution->get().compute_element(m_elem_idx);
  }

  virtual void unset_element()
  {
    Term::unset_element();
    flx_pt_plane_jacobian_normal->get().unlock();
    flx_pt_solution->get().unlock();
  }


protected: // fast-access-data (for convenience no "m_" prefix)

  // Data

  Uint sol_pt;
  Uint flx_pt;
  Uint neighbour_flx_pt;
  Handle<mesh::Entities const> neighbour_entities;
  Uint neighbour_elem_idx;
  Uint neighbour_face_nb;
  Handle<mesh::Entities const> face_entities;
  Uint face_idx;

  // In flux points:
  Handle< CacheT<SFDElement> > elem;
  Handle< CacheT<FluxPointDivergence> > divergence_in_solution_points;
  Handle< CacheT<FluxPointReconstruct> > reconstruct_in_solution_points;
  Handle< CacheT<FluxPointField<NEQS,NDIM> > > flx_pt_solution;
  Handle< CacheT<FluxPointField<NEQS,NDIM> > > flx_pt_neighbour_solution;
  Handle< CacheT<FluxPointPlaneJacobianNormal<NDIM> > > flx_pt_plane_jacobian_normal;

  std::vector< Eigen::Matrix<Real,NEQS,1> >   flx_pt_flux;
  std::vector< Eigen::Matrix<Real,1,1> >   flx_pt_wave_speed;
  std::vector< std::vector<Eigen::Matrix<Real,1,1> > > sol_pt_wave_speed;

}; // end ConvectiveTerm

////////////////////////////////////////////////////////////////////////////////

template <Uint NEQS, Uint NDIM>
ConvectiveTerm<NEQS,NDIM>::ConvectiveTerm( const std::string& name )
  : Term(name)
{
  properties()["brief"] = std::string("Convective Spectral Finite Difference term");
  properties()["description"] = std::string("Fields to be created: ...");
}

/////////////////////////////////////////////////////////////////////////////

template <Uint NEQS, Uint NDIM>
void ConvectiveTerm<NEQS,NDIM>::execute()
{
  boost_foreach(flx_pt, elem->get().sf->interior_flx_pts())
  {
    compute_analytical_flux(flx_pt_plane_jacobian_normal->get().plane_unit_normal[flx_pt]);
    flx_pt_flux[flx_pt] *= flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt];
    flx_pt_wave_speed[flx_pt] *= flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt];
  }
  for(Uint f=0; f<elem->get().sf->nb_faces(); ++f)
  {
    set_neighbour(m_entities,m_elem_idx,f,
                  neighbour_entities,neighbour_elem_idx,neighbour_face_nb,
                  face_entities,face_idx);
    if ( is_not_null(neighbour_entities) )
    {
      // Reconstruct neighbour solution
      flx_pt_neighbour_solution->cache(neighbour_entities,neighbour_elem_idx);
      Uint c=0;
      boost_foreach(flx_pt, elem->get().sf->face_flx_pts(f))
      {
        // Get connected neighbour_flx_pt
        neighbour_flx_pt = flx_pt_neighbour_solution->get().sf->face_flx_pts(neighbour_face_nb)[elem->get().sf->face_flx_pts(f).size()-1-c];
        ++c;
        compute_numerical_flux(elem->get().sf->flx_pt_sign(flx_pt) * flx_pt_plane_jacobian_normal->get().plane_unit_normal[flx_pt]);
        flx_pt_flux[flx_pt] *= elem->get().sf->flx_pt_sign(flx_pt) * flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt];
        flx_pt_wave_speed[flx_pt] *= flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt];
      }
      flx_pt_neighbour_solution->get().unlock();
    }
    else
    {
      boost_foreach(flx_pt, elem->get().sf->face_flx_pts(f))
      {
        compute_analytical_flux(flx_pt_plane_jacobian_normal->get().plane_unit_normal[flx_pt]);
        flx_pt_flux[flx_pt] *= flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt];
        flx_pt_wave_speed[flx_pt] *= flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt];
      }
    }
  }
  // compute divergence in solution points
  mesh::Field::View term = m_term_field->view(elem->get().space->indexes_for_element(m_elem_idx));
  divergence_in_solution_points->cache(m_entities).compute(flx_pt_flux,term);
  mesh::Field::View jacob_det = jacob_det_field().view(elem->get().space->indexes_for_element(m_elem_idx));
  for (sol_pt=0; sol_pt<elem->get().sf->nb_sol_pts(); ++sol_pt) {
    for (Uint v=0; v<NEQS; ++v) {
      term[sol_pt][v] /= jacob_det[sol_pt][0];
    }
  }

  mesh::Field::View residual = residual_field().view(elem->get().space->indexes_for_element(m_elem_idx));
  for (sol_pt=0; sol_pt<elem->get().sf->nb_sol_pts(); ++sol_pt) {
    for (Uint v=0; v<NEQS; ++v) {
      residual[sol_pt][v] -= term[sol_pt][v];
    }
  }

  // Compute cfl number.
  // dU'/dt = - A'*dU'/dski
  // dU/dt = - 1/|J| * A'*dU'/dksi  (1)
  // dU/dt = - A*dU/dx              (2)

  // (1) --> cfl = delta(t)*A/delta(x) = delta(t)*A/(J*delta(ksi))
  // (2) --> cfl = 1/|J| delta(t)*A'/delta(ksi)            (3)

  // ==> delta(t)/delta(ksi) * A'/|J|  =  delta(t)*A/(J*delta(ksi))
  // ==> A'/ |J| = A / J
  // ==> A' = |J|*J^-1 * A

  // (3) --> cfl = 1/|J| * delta(t) * A'/delta(ksi)
  //             = 1/|J| * delta(t) * |J|*J^-1 * A / delta(ksi)  (note that |J|*J^(-1) == plane_jacobian normal!)

  mesh::Field::View wave_speed = wave_speed_field().view(elem->get().space->indexes_for_element(m_elem_idx));
  mesh::Field::View term_wave = m_term_wave_speed_field->view(elem->get().space->indexes_for_element(m_elem_idx));
  for (Uint d=0; d<NDIM; ++d)
  {
    reconstruct_in_solution_points->cache(m_entities).compute(d,flx_pt_wave_speed,sol_pt_wave_speed[d]);
  }

  for (sol_pt=0; sol_pt<elem->get().sf->nb_sol_pts(); ++sol_pt)
  {
    term_wave[sol_pt][0] = 0;
    for (Uint d=0; d<NDIM; ++d)
    {
      term_wave[sol_pt][0] += sol_pt_wave_speed[d][sol_pt][0]/2.;
    }
    term_wave[sol_pt][0] /= jacob_det[sol_pt][0];
    wave_speed[sol_pt][0] = std::max(wave_speed[sol_pt][0],term_wave[sol_pt][0]);
  }
}

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_SFDM_ConvectiveTerm_hpp
