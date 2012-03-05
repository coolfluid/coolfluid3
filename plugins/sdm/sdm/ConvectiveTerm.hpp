// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_ConvectiveTerm_hpp
#define cf3_sdm_ConvectiveTerm_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/PropertyList.hpp"

#include "math/MatrixTypes.hpp"

#include "mesh/Connectivity.hpp"

#include "sdm/Tags.hpp"
#include "sdm/Term.hpp"
#include "sdm/ShapeFunction.hpp"
#include "sdm/Operations.hpp"
#include "sdm/PhysDataBase.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {

//////////////////////////////////////////////////////////////////////////////

/// A proposed base class for simple convective terms.
/// Classes inheriting only need to implement functions to compute
/// - analytical flux for interior flux points
/// - numerical flux for face flux points
/// @author Willem Deconinck
template <typename PHYSDATA>
class ConvectiveTerm : public Term
{
public: // typedefs
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  enum { NDIM = PHYSDATA::NDIM }; ///< number of dimensions
  enum { NEQS = PHYSDATA::NEQS }; ///< number of independent variables or equations
  typedef PHYSDATA PhysData;
  typedef Eigen::Matrix<Real,NDIM,1> RealVectorNDIM;
  typedef Eigen::Matrix<Real,NEQS,1> RealVectorNEQS;

public: // functions

  /// constructor
  ConvectiveTerm( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "ConvectiveTerm"; }

  virtual void execute();

private: // functions

  // Pure virtual Flux evaluations
  // -----------------------------
  /// @brief Compute analytical flux
  /// @param [in]  data          Physical data necessary to compute the flux
  /// @param [in]  unit_normal   Unit normal to project flux on
  /// @param [out] flux          Computed flux, projected on unit_normal
  /// @param [out] wave_speed    wave-speed in unit_normal direction
  virtual void compute_analytical_flux(PHYSDATA& data, const RealVectorNDIM& unit_normal, RealVectorNEQS& flux, Real& wave_speed) = 0;

  /// @brief Compute numerical flux
  ///
  /// On faces, the physical data is discontinuous. A numerical flux needs to be computed. Typically an approximate Riemann solver
  /// @param [in]  left          Physical data left of the interface, necessary to compute the flux
  /// @param [in]  right         Physical data right of the interface, necessary to compute the flux
  /// @param [in]  unit_normal   Unit normal to project flux on
  /// @param [out] flux          Computed flux, projected on unit_normal
  /// @param [out] wave_speed    wave-speed in unit_normal direction
  virtual void compute_numerical_flux(PHYSDATA& left, PHYSDATA& right, const RealVectorNDIM& unit_normal, RealVectorNEQS& flux, Real& wave_speed) = 0;

protected: // configuration

  /// @brief Initialize this term
  virtual void initialize();

  /// @brief allocate data to be used in flux points
  virtual void allocate_flx_pt_data();

  /// @brief Initialize caches for a given Entities component
  virtual void set_entities(const mesh::Entities& entities);

  /// @brief Set caches for element with given index
  virtual void set_element(const Uint elem_idx);

  /// @brief Standard computation of solution and coordinates in flux point
  ///
  /// This function NEEDS to be overloaded for terms thar require more data to be set in phys_data
  virtual void compute_flx_pt_phys_data(const SFDElement& elem, const Uint flx_pt, PHYSDATA& phys_data );

  /// @brief Standard computation of solution and coordinates in a solution point
  ///
  /// This function has to be used for boundaries, where the neighbour-element is a face entities
  /// The physical data is then located inside the face itself.
  virtual void compute_sol_pt_phys_data(const SFDElement& elem, const Uint sol_pt, PHYSDATA& phys_data );

  /// @brief Given computed face, compute connectivity
  ///
  /// Sets left_face_pt_idx and right_face_pt_idx
  void set_connectivity();

  /// @brief Compute all physical data and connectivity in the face
  virtual void compute_face();

  /// @brief free the element caches
  virtual void unset_element();

protected: // fast-access-data (for convenience no "m_" prefix)

  boost::shared_ptr< PHYSDATA > flx_pt_data;                    ///< Physical data (for interior points)
  std::vector<boost::shared_ptr< PHYSDATA > > left_face_data;   ///< Physical data on left side of face
  std::vector<boost::shared_ptr< PHYSDATA > > right_face_data;  ///< Physical data on right side of face
  std::vector<Uint> left_face_pt_idx;                           ///< Connectivity of left face points
  std::vector<Uint> right_face_pt_idx;                          ///< Connectivity of right face points

  Uint sol_pt;                                           ///< Current solution point
  Uint flx_pt;                                           ///< Current flux point
  Handle<mesh::Entities const> neighbour_entities;       ///< This cell's neighbour at current face
  Uint neighbour_elem_idx;                               ///< This cell's neighbour cell index at current face
  Uint neighbour_face_nb;                                ///< This cell's neighbour face number of current face
  Handle<mesh::Entities const> face_entities;            ///< Current face
  Uint face_idx;                                         ///< Current face index

  Handle< CacheT<SFDElement> > elem;                     ///< This cell
  Handle< CacheT<SFDElement> > neighbour_elem;           ///< Current neighbour
  Handle< CacheT<FluxPointPlaneJacobianNormal<NDIM> > > flx_pt_plane_jacobian_normal; ///< Plane jacobian normal in flux points

  // In flux points:
  std::vector< RealVectorNEQS >   flx_pt_flux;                  ///< Storage of fluxes in flux points
  std::vector< RealVector1 >   flx_pt_wave_speed;               ///< Storage of wave speeds in flux points
  std::vector< std::vector<RealVector1> > sol_pt_wave_speed;   ///< Storage of wave speeds in solution points in every direction

}; // end ConvectiveTerm

////////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
ConvectiveTerm<PHYSDATA>::ConvectiveTerm( const std::string& name )
  : Term(name)
{
  properties()["brief"] = std::string("Convective Spectral Difference term");
  properties()["description"] = std::string("Computes on a per cell basis the residual- and"
                                            "wave-speed contribution of a convective term");
}

/////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
void ConvectiveTerm<PHYSDATA>::execute()
{

  /// 1) Calculate flux in interior flux points
  boost_foreach(flx_pt, elem->get().sf->interior_flx_pts())
  {
    compute_flx_pt_phys_data(elem->get(),flx_pt,*flx_pt_data);
    compute_analytical_flux(*flx_pt_data,flx_pt_plane_jacobian_normal->get().plane_unit_normal[flx_pt],
                            flx_pt_flux[flx_pt],flx_pt_wave_speed[flx_pt][0]);
    flx_pt_flux[flx_pt] *= flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt];
    flx_pt_wave_speed[flx_pt] *= flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt];
  }

  /// 2) Calculate flux in face flux points
  for(m_face_nb=0; m_face_nb<elem->get().sf->nb_faces(); ++m_face_nb)
  {
    /// 2.1) Compute physical data in face
    compute_face();

    /// 2.2) Compute flux
    /// * Case 1: face is marked as outer_face --> extrapolate solution from interior and compute analytical flux
    if (face_entities->has_tag(mesh::Tags::outer_faces()))
    {
      for (Uint face_pt=0; face_pt<elem->get().sf->face_flx_pts(m_face_nb).size(); ++face_pt)
      {
        flx_pt = left_face_pt_idx[face_pt];
        compute_analytical_flux(*left_face_data[face_pt],flx_pt_plane_jacobian_normal->get().plane_unit_normal[flx_pt],
                                flx_pt_flux[flx_pt],flx_pt_wave_speed[flx_pt][0]);
        flx_pt_flux[flx_pt] *= flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt];
        flx_pt_wave_speed[flx_pt] *= flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt];
      }
    }
    /// * Case 2: face is inner-face or boundary-face --> compute numerical flux
    else
    {
      for (Uint face_pt=0; face_pt<elem->get().sf->face_flx_pts(m_face_nb).size(); ++face_pt)
      {
        flx_pt = left_face_pt_idx[face_pt];
        compute_numerical_flux(*left_face_data[face_pt],*right_face_data[face_pt],flx_pt_plane_jacobian_normal->get().plane_unit_normal[flx_pt] * elem->get().sf->flx_pt_sign(flx_pt),
                               flx_pt_flux[flx_pt],flx_pt_wave_speed[flx_pt][0]);
        flx_pt_flux[flx_pt] *= elem->get().sf->flx_pt_sign(flx_pt) * flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt];
        flx_pt_wave_speed[flx_pt] *= flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt];
      }
    }
    neighbour_elem->get().unlock();
  }


  /// 3) Compute flux divergence in solution points and store in this term's field
  mesh::Field::View term = m_term_field->view(elem->get().space->connectivity()[m_elem_idx]);
  elem->get().reconstruct_divergence_from_flux_points_to_solution_space(flx_pt_flux,term);
  mesh::Field::View jacob_det = jacob_det_field().view(elem->get().space->connectivity()[m_elem_idx]);
  for (sol_pt=0; sol_pt<elem->get().sf->nb_sol_pts(); ++sol_pt) {
    for (Uint v=0; v<NEQS; ++v) {
      term[sol_pt][v] /= jacob_det[sol_pt][0];
    }
  }

  /// 4) Subtract this term from the residual field
  mesh::Field::View residual = residual_field().view(elem->get().space->connectivity()[m_elem_idx]);
  for (sol_pt=0; sol_pt<elem->get().sf->nb_sol_pts(); ++sol_pt) {
    for (Uint v=0; v<NEQS; ++v) {
      residual[sol_pt][v] -= term[sol_pt][v];
    }
  }

  /// 5) Reconstruct the wave speed from flux points to solution points
  ///    in this term's wave_speed field.
  ///    Then modify the total wave_speed to be more strict
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

  mesh::Field::View wave_speed = wave_speed_field().view(elem->get().space->connectivity()[m_elem_idx]);
  mesh::Field::View term_wave = m_term_wave_speed_field->view(elem->get().space->connectivity()[m_elem_idx]);
  for (Uint d=0; d<NDIM; ++d)
  {
    elem->get().reconstruct_from_flux_points_to_solution_space(d,flx_pt_wave_speed,sol_pt_wave_speed[d]);
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

template <typename PHYSDATA>
void ConvectiveTerm<PHYSDATA>::initialize()
{
  Term::initialize();
  elem                  = shared_caches().template get_cache< SFDElement >();
  neighbour_elem        = shared_caches().template get_cache< SFDElement >("neighbour_elem");
  flx_pt_plane_jacobian_normal = shared_caches().template get_cache< FluxPointPlaneJacobianNormal<NDIM> >();

  elem          ->options().configure_option("space",solution_field().dict().handle<mesh::Dictionary>());
  neighbour_elem->options().configure_option("space",solution_field().dict().handle<mesh::Dictionary>());
  flx_pt_plane_jacobian_normal->options().configure_option("space",solution_field().dict().handle<mesh::Dictionary>());
}

////////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
void ConvectiveTerm<PHYSDATA>::allocate_flx_pt_data()
{
  flx_pt_data = boost::shared_ptr< PHYSDATA >( new PHYSDATA );

  left_face_pt_idx.resize(elem->get().sf->face_flx_pts(0).size());
  right_face_pt_idx.resize(elem->get().sf->face_flx_pts(0).size());
  left_face_data .resize(elem->get().sf->face_flx_pts(0).size());
  right_face_data .resize(elem->get().sf->face_flx_pts(0).size());
  for (Uint face_pt=0; face_pt<left_face_data.size(); ++face_pt)
  {
    left_face_data[face_pt] = boost::shared_ptr< PHYSDATA >( new PHYSDATA );
    right_face_data[face_pt] = boost::shared_ptr< PHYSDATA >( new PHYSDATA );
  }
}

////////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
void ConvectiveTerm<PHYSDATA>::set_entities(const mesh::Entities& entities)
{
  Term::set_entities(entities);

  elem->cache(m_entities);
  flx_pt_plane_jacobian_normal->cache(m_entities);

  sol_pt_wave_speed.resize(NDIM,std::vector< RealVector1 >(elem->get().sf->nb_sol_pts()));
  flx_pt_wave_speed.resize(elem->get().sf->nb_flx_pts());
  flx_pt_flux.resize(elem->get().sf->nb_flx_pts());

  allocate_flx_pt_data();
}

////////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
void ConvectiveTerm<PHYSDATA>::set_element(const Uint elem_idx)
{
  Term::set_element(elem_idx);
  elem->set_cache(m_elem_idx);
  flx_pt_plane_jacobian_normal->set_cache(m_elem_idx);
}

////////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
void ConvectiveTerm<PHYSDATA>::compute_flx_pt_phys_data(const SFDElement& elem, const Uint flx_pt, PHYSDATA& phys_data )
{
  mesh::Field::View sol_pt_solution = solution_field().view(elem.space->connectivity()[elem.idx]);
  mesh::Field::View sol_pt_coords   = solution_field().dict().coordinates().view(elem.space->connectivity()[elem.idx]);
  elem.reconstruct_from_solution_space_to_flux_points[flx_pt](sol_pt_solution,phys_data.solution);
  elem.reconstruct_from_solution_space_to_flux_points[flx_pt](sol_pt_coords,phys_data.coord);
}

////////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
void ConvectiveTerm<PHYSDATA>::compute_sol_pt_phys_data(const SFDElement& elem, const Uint sol_pt, PHYSDATA& phys_data )
{
  mesh::Field::View sol_pt_solution = solution_field().view(elem.space->connectivity()[elem.idx]);
  mesh::Field::View sol_pt_coords   = solution_field().dict().coordinates().view(elem.space->connectivity()[elem.idx]);
  for (Uint var=0; var<NEQS; ++var)
    phys_data.solution[var] = sol_pt_solution[sol_pt][var];
  for (Uint dim=0; dim<NDIM; ++dim)
    phys_data.coord[dim] = sol_pt_coords[sol_pt][dim];
}

////////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
void ConvectiveTerm<PHYSDATA>::set_connectivity()
{
  left_face_pt_idx = elem->get().sf->face_flx_pts(m_face_nb);

  // If neighbour is a cell
  if ( is_not_null(neighbour_entities) )
  {
    neighbour_elem->cache(neighbour_entities,neighbour_elem_idx);
    Uint nb_neighbour_face_pts = neighbour_elem->get().sf->face_flx_pts(neighbour_face_nb).size();
    for(Uint face_pt=0; face_pt<nb_neighbour_face_pts; ++face_pt)
    {
      right_face_pt_idx[face_pt] = neighbour_elem->get().sf->face_flx_pts(neighbour_face_nb)[nb_neighbour_face_pts-1-face_pt];
    }
  }
  // If there is no neighbour, but a face
  else
  {
    neighbour_elem->cache(face_entities,face_idx);

    if (elem->get().sf->order() == 0)
    {
      right_face_pt_idx[0] = 0;
    }
    else
    {
      mesh::Field::View cell_coords   = solution_field().dict().coordinates().view(elem->get().space->connectivity()[elem->get().idx]);
      std::vector<RealVector> cell_face_coords(left_face_pt_idx.size(),RealVector(NDIM));
      for (Uint face_pt=0; face_pt<left_face_pt_idx.size(); ++face_pt)
      {
        elem->get().reconstruct_from_solution_space_to_flux_points[left_face_pt_idx[face_pt]](cell_coords,cell_face_coords[face_pt]);
      }
      RealMatrix face_coords = neighbour_elem->get().space->get_coordinates(neighbour_elem->get().idx);
      for (Uint right_face_pt=0; right_face_pt<right_face_pt_idx.size(); ++right_face_pt)
      {
        const RealVector& right_face_pt_coord = face_coords.row(right_face_pt);
        bool matched = false;
        for (Uint left_face_pt=0; left_face_pt<left_face_pt_idx.size(); ++left_face_pt)
        {
          bool m=true;
          for (Uint d=0; d<NDIM; ++d)
            m = m && ( std::abs(cell_face_coords[left_face_pt][d] - right_face_pt_coord[d]) < 100*math::Consts::eps() );
          if ( m )
          {
            matched=true;
            right_face_pt_idx[left_face_pt] = right_face_pt;
            break;
          }
        }
        cf3_assert_desc(elem->get().space->uri().string()+"["+common::to_str(elem->get().idx)+"]",matched);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
void ConvectiveTerm<PHYSDATA>::compute_face()
{
  static Uint face_side;

  /// 1) Find the neighbour element and face entity
  set_face(m_entities,m_elem_idx,m_face_nb,
           neighbour_entities,neighbour_elem_idx,neighbour_face_nb,
           face_entities,face_idx,face_side);

  /// 2) Set connectivity from face points on the left side to face points on the right side
  set_connectivity();

  /// 3) Compute physical data in left face points
  for (Uint face_pt=0; face_pt<elem->get().sf->face_flx_pts(m_face_nb).size(); ++face_pt)
    compute_flx_pt_phys_data(elem->get(),left_face_pt_idx[face_pt],*left_face_data[face_pt]);

  /// 4) Compute physical data in right face points

  ///  * Case there is a neighbour cell
  if ( is_not_null(neighbour_entities) )
  {
    for (Uint face_pt=0; face_pt<elem->get().sf->face_flx_pts(m_face_nb).size(); ++face_pt)
      compute_flx_pt_phys_data(neighbour_elem->get(),right_face_pt_idx[face_pt],*right_face_data[face_pt]);
  }
  /// * Case there is no neighbour, but physical data inside the face solution points
  else
  {
    neighbour_elem->cache(face_entities,face_idx);
    for (Uint face_pt=0; face_pt<right_face_pt_idx.size(); ++face_pt)
      compute_sol_pt_phys_data(neighbour_elem->get(),right_face_pt_idx[face_pt],*right_face_data[face_pt]);
  }
}

////////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
void ConvectiveTerm<PHYSDATA>::unset_element()
{
  Term::unset_element();
  elem->get().unlock();
  flx_pt_plane_jacobian_normal->get().unlock();
}

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_ConvectiveTerm_hpp
