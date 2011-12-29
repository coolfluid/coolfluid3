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
    link_fields();
    create_term_field();
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
  Handle<mesh::Entities const> face_entities;
  Uint face_idx;

  // In flux points:
  Handle< CacheT<SFDElement> > elem;
  Handle< CacheT<FluxPointDivergence> > divergence_in_solution_points;
  Handle< CacheT<FluxPointReconstruct> > reconstruct_in_solution_points;
  Handle< CacheT<FluxPointField<NEQS,NDIM> > > flx_pt_solution;
  Handle< CacheT<FluxPointField<NEQS,NDIM> > > flx_pt_neighbour_solution;
  Handle< CacheT<FluxPointPlaneJacobianNormal<NDIM> > > flx_pt_plane_jacobian_normal;

  std::vector< Eigen::Matrix<Real,NEQS,1> > flux;
  std::vector< Eigen::Matrix<Real,1,1> > wave_speed;
  Eigen::Matrix<Real,NDIM,1> delta_ksi;
  Eigen::Matrix<Real,NDIM,1> m_unit_normal;

  virtual void compute_wave_speed() {}

}; // end ConvectiveTerm

////////////////////////////////////////////////////////////////////////////////

template <Uint NEQS, Uint NDIM>
ConvectiveTerm<NEQS,NDIM>::ConvectiveTerm( const std::string& name )
  : Term(name)
{
  properties()["brief"] = std::string("Convective Spectral Finite Difference term");
  properties()["description"] = std::string("Fields to be created: ...");

  delta_ksi.setConstant(2.);
}

/////////////////////////////////////////////////////////////////////////////

template <Uint NEQS, Uint NDIM>
void ConvectiveTerm<NEQS,NDIM>::execute()
{
  wave_speed.resize(elem->get().sf->nb_flx_pts());
  flux.resize(elem->get().sf->nb_flx_pts());
  boost_foreach(flx_pt, elem->get().sf->interior_flx_pts())
  {
    //std::cout << "compute analytical flux in flx_pt["<<flx_pt<<"]"<<std::endl;
    m_unit_normal = flx_pt_plane_jacobian_normal->get()[flx_pt] / flx_pt_plane_jacobian_normal->get()[flx_pt].norm();
    compute_analytical_flux(m_unit_normal);
    flux[flx_pt] *= flx_pt_plane_jacobian_normal->get()[flx_pt].norm();
//    std::cout << "flux = " << flux[flx_pt] << std::endl;
  }
  for(Uint f=0; f<elem->get().sf->nb_faces(); ++f)
  {
    Uint neighbour_face_nb(100);
    set_neighbour(m_entities,m_elem_idx,f,
                  neighbour_entities,neighbour_elem_idx,neighbour_face_nb,
                  face_entities,face_idx);
    if ( is_not_null(neighbour_entities) )
    {
      //std::cout << "caching neighbour idx " << neighbour_elem_idx << std::endl;
      flx_pt_neighbour_solution->cache(neighbour_entities,neighbour_elem_idx);
      // 2) solve riemann problem on interior-faces  ----> Flux   ( linked with (1) )
      //     Save in face (yes/no)
      Uint c=0;
      boost_foreach(flx_pt, elem->get().sf->face_flx_pts(f))
      {
        neighbour_flx_pt = flx_pt_neighbour_solution->get().sf->face_flx_pts(neighbour_face_nb)[elem->get().sf->face_flx_pts(f).size()-1-c];
//        std::cout << "compute numerical flux in flx_pt["<<flx_pt<<"]  <-->  flx_pt["<<neighbour_flx_pt<<"]" << std::endl;
        //std::cout << "neighbour sol_in_flx_pt = " << neighbour_solution->field_in_flx_pts[f] << std::endl;
        m_unit_normal = elem->get().sf->flx_pt_sign(flx_pt) * flx_pt_plane_jacobian_normal->get()[flx_pt] / flx_pt_plane_jacobian_normal->get()[flx_pt].norm();

//        RealMatrix neighbour_nodes = neighbour_entities->get_coordinates(neighbour_elem_idx);
//        RealVector plane_jacob = neighbour_entities->element_type().plane_jacobian_normal(elem->get().sf->flx_pts().row(neighbour_flx_pt),neighbour_nodes,(CoordRef)elem->get().sf->flx_pt_dirs(neighbour_flx_pt)[0]);
//        cf3_assert(plane_jacob == plane_jacobian_normal->get()[flx_pt]);
//        RealMatrix nodes = m_entities->get_coordinates(m_elem_idx);
//        RealRowVector sf = m_entities->element_type().shape_function().value(elem->get().sf->flx_pts().row(flx_pt));
//        RealVector flx_pt_coord = sf*nodes;
//        RealRowVector neighbour_sf = neighbour_entities->element_type().shape_function().value(elem->get().sf->flx_pts().row(neighbour_flx_pt));
//        RealVector neighbour_flx_pt_coord = neighbour_sf*neighbour_nodes;
//        cf3_assert(flx_pt_coord == neighbour_flx_pt_coord);
        compute_numerical_flux(m_unit_normal);
        flux[flx_pt]*=elem->get().sf->flx_pt_sign(flx_pt);
        ++c;

        flux[flx_pt] *= flx_pt_plane_jacobian_normal->get()[flx_pt].norm();
//        std::cout << "flux = " << flux[flx_pt] << std::endl;

      }
      flx_pt_neighbour_solution->get().unlock();
    }
    else
    {
      boost_foreach(flx_pt, elem->get().sf->face_flx_pts(f))
      {
        m_unit_normal = flx_pt_plane_jacobian_normal->get()[flx_pt] / flx_pt_plane_jacobian_normal->get()[flx_pt].norm();
        compute_analytical_flux(m_unit_normal);
        flux[flx_pt] *= flx_pt_plane_jacobian_normal->get()[flx_pt].norm();
//        std::cout << "flux = " << flux[flx_pt] << std::endl;

      }
    }
  }
  // compute divergence in solution points
  mesh::Field::View term = m_term_field->view(elem->get().space->indexes_for_element(m_elem_idx));
  divergence_in_solution_points->cache(m_entities).compute(flux,term);
  mesh::Field::View jacob_det = jacob_det_field().view(elem->get().space->indexes_for_element(m_elem_idx));
  for (sol_pt=0; sol_pt<elem->get().sf->nb_sol_pts(); ++sol_pt) {
    for (Uint v=0; v<NEQS; ++v) {
      term[sol_pt][v] /= jacob_det[sol_pt][0];
    }
  }

//  reconstruct_in_solution_points->cache(m_entities).compute(XX,wave_speed,wave_speed_field().view(elem->get().space->indexes_for_element(m_elem_idx)));
  //std::cout << "div_flx = " << to_str(term) << std::endl; //elem->divergence_from_flux_points(elem->flx_in_flx_pts).transpose() << std::endl;

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

  for (sol_pt=0; sol_pt<elem->get().sf->nb_sol_pts(); ++sol_pt) {
    compute_wave_speed();
    mesh::Field::View term_wave = m_term_wave_speed_field->view(elem->get().space->indexes_for_element(m_elem_idx));
    mesh::Field::View delta = m_delta->view(elem->get().space->indexes_for_element(m_elem_idx));
    RealVector ws(NDIM);
    for (Uint d=0; d<NDIM; ++d)
    {
      ws[d]= std::abs(term_wave[sol_pt][d])  / delta[sol_pt][d];
    }
    wave_speed_field().view(elem->get().space->indexes_for_element(m_elem_idx))[sol_pt][0] = ws.maxCoeff();
  }

}

////////////////////////////////////////////////////////////////////////////////

class LinearAdvection1D : public ConvectiveTerm<1u,1u>
{
public:
  static std::string type_name() { return "LinearAdvection1D"; }
  LinearAdvection1D(const std::string& name) : ConvectiveTerm(name)
  {
    m_advection_speed.resize(1u);
    m_advection_speed[XX]= 1.;

    analytical_flux.functions(common::to_str(m_advection_speed[XX])+"*U");
    analytical_flux.variables("U");
    analytical_flux.parse();

    options().add_option("advection_speed",m_advection_speed).link_to(&m_advection_speed);
  }
  virtual ~LinearAdvection1D() {}

  virtual void compute_analytical_flux(const RealVector1& unit_normal)
  {
    Real A = unit_normal[XX]*m_advection_speed[XX];
    flux[flx_pt] = A*flx_pt_solution->get()[flx_pt];
    wave_speed[flx_pt][0] = std::abs(m_advection_speed[XX]);
  }

  virtual void compute_numerical_flux(const RealVector1& unit_normal)
  {
    RealVector1& left  = flx_pt_solution->get()[flx_pt];
    RealVector1& right = flx_pt_neighbour_solution->get()[neighbour_flx_pt];
    Real A = m_advection_speed[XX]*unit_normal[XX];
    flux[flx_pt] = 0.5 * A*(left + right) - 0.5 * std::abs(A)*(right - left);
    wave_speed[flx_pt][0] = std::abs(m_advection_speed[XX]);
  }

  virtual void compute_wave_speed()
  {
    mesh::Field::View ws = m_term_wave_speed_field->view(elem->get().space->indexes_for_element(m_elem_idx));
    ws[sol_pt][XX] = m_advection_speed[XX];
  }

private:
  math::VectorialFunction analytical_flux;

  std::vector<Real> m_advection_speed;
};

////////////////////////////////////////////////////////////////////////////////

class LinearAdvection2D : public ConvectiveTerm<1u,2u>
{
public:
  static std::string type_name() { return "LinearAdvection2D"; }
  LinearAdvection2D(const std::string& name) : ConvectiveTerm(name)
  {
    m_advection_speed.resize(2u);
    m_advection_speed[XX]= 1.;
    m_advection_speed[YY]= 1.;

    options().add_option("advection_speed",m_advection_speed).link_to(&m_advection_speed);
  }
  virtual ~LinearAdvection2D() {}

  virtual void compute_analytical_flux(const RealVector2& unit_normal)
  {
    Real A = (unit_normal[XX]*m_advection_speed[XX]+unit_normal[YY]*m_advection_speed[YY]);
    flux[flx_pt] = A*flx_pt_solution->get()[flx_pt];
  }

  virtual void compute_numerical_flux(const RealVector2& unit_normal)
  {
    RealVector1& left  = flx_pt_solution->get()[flx_pt];
    RealVector1& right = flx_pt_neighbour_solution->get()[neighbour_flx_pt];
    Real A = (m_advection_speed[XX]*unit_normal[XX]+m_advection_speed[YY]*unit_normal[YY]);
    flux[flx_pt] = 0.5 * A*(left + right) - 0.5 * std::abs(A)*(right - left);
  }

  virtual void compute_wave_speed()
  {
    mesh::Field::View ws = m_term_wave_speed_field->view(elem->get().space->indexes_for_element(m_elem_idx));
    ws[sol_pt][XX] = m_advection_speed[XX];
    ws[sol_pt][YY] = m_advection_speed[YY];
  }

private:
  std::vector<Real> m_advection_speed;
};

////////////////////////////////////////////////////////////////////////////////

class RotationAdvection2D : public ConvectiveTerm<1u,2u>
{
public:
  static std::string type_name() { return "RotationAdvection2D"; }
  RotationAdvection2D(const std::string& name) : ConvectiveTerm(name)
  {
  }

  virtual ~RotationAdvection2D() {}

  virtual void compute_analytical_flux(const RealVector2& unit_normal)
  {
    Real A = (unit_normal[XX]*flx_pt_coordinates->get()[flx_pt][YY]-unit_normal[YY]*flx_pt_coordinates->get()[flx_pt][XX]);
    flux[flx_pt] = A*flx_pt_solution->get()[flx_pt];
  }

  virtual void compute_numerical_flux(const RealVector2& unit_normal)
  {
    RealVector1& left  = flx_pt_solution->get()[flx_pt];
    RealVector1& right = flx_pt_neighbour_solution->get()[neighbour_flx_pt];
    Real A = (unit_normal[XX]*flx_pt_coordinates->get()[flx_pt][YY]-unit_normal[YY]*flx_pt_coordinates->get()[flx_pt][XX]);
    flux[flx_pt] = 0.5 * A*(left + right) - 0.5 * std::abs(A)*(right - left);
  }

  virtual void compute_wave_speed()
  {
    mesh::Field::View ws = m_term_wave_speed_field->view(elem->get().space->indexes_for_element(m_elem_idx));
    ws[sol_pt][XX] =  sol_pt_coordinates->get()[flx_pt][YY];
    ws[sol_pt][YY] = -sol_pt_coordinates->get()[flx_pt][XX];
  }

  virtual void initialize()
  {
    ConvectiveTerm::initialize();
    flx_pt_coordinates = shared_caches().get_cache< FluxPointCoordinates<2u> >();
    flx_pt_coordinates->options().configure_option("space",solution_field().space());
    sol_pt_coordinates = shared_caches().get_cache< SolutionPointCoordinates<2u> >();
    sol_pt_coordinates->options().configure_option("space",solution_field().space());
  }

  virtual void set_entities(const mesh::Entities& entities)
  {
    ConvectiveTerm::set_entities(entities);
    flx_pt_coordinates->cache(m_entities);
    sol_pt_coordinates->cache(m_entities);
  }

  virtual void set_element(const Uint elem_idx)
  {
    ConvectiveTerm::set_element(elem_idx);
    flx_pt_coordinates->get().compute_element(m_elem_idx);
    sol_pt_coordinates->get().compute_element(m_elem_idx);
  }

  virtual void unset_element()
  {
    ConvectiveTerm::unset_element();
    flx_pt_coordinates->get().unlock();
    sol_pt_coordinates->get().unlock();
  }

private:
  Handle< CacheT< FluxPointCoordinates<2u> > > flx_pt_coordinates;
  Handle< CacheT< SolutionPointCoordinates<2u> > > sol_pt_coordinates;
};

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_SFDM_ConvectiveTerm_hpp