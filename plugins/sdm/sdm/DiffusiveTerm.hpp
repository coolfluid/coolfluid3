// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_DiffusiveTerm_hpp
#define cf3_sdm_DiffusiveTerm_hpp

//#ifdef GNUPLOT_FOUND
// Uncomment to output gnuplot plot for the flux. only for 1D simulations!!!
//  #define SDM_OUTPUT_FLUX_GNUPLOT
//#endif
////////////////////////////////////////////////////////////////////////////////

#include "common/PropertyList.hpp"
#include "common/OptionT.hpp"

#include "math/MatrixTypes.hpp"

#include "mesh/Connectivity.hpp"

#include "sdm/Tags.hpp"
#include "sdm/Term.hpp"
#include "sdm/ShapeFunction.hpp"
#include "sdm/Operations.hpp"
#include "sdm/PhysDataBase.hpp"

#ifdef SDM_OUTPUT_FLUX_GNUPLOT
#include "Tools/Gnuplot/Gnuplot.hpp"
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {

//////////////////////////////////////////////////////////////////////////////

/// A proposed base class for simple Diffusive terms.
/// Classes inheriting only need to implement functions to compute
/// - flux
/// @author Willem Deconinck
template <typename PHYSDATA>
class DiffusiveTerm : public Term
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
  DiffusiveTerm( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "DiffusiveTerm"; }

  virtual void execute();

private: // functions

  // Pure virtual Flux evaluations
  // -----------------------------
  /// @brief Compute flux
  /// @param [in]  data          Physical data necessary to compute the flux
  /// @param [in]  unit_normal   Unit normal to project flux on
  /// @param [out] flux          Computed flux, projected on unit_normal
  /// @param [out] wave_speed    wave-speed in unit_normal direction
  virtual void compute_flux(PHYSDATA& data, const RealVectorNDIM& unit_normal, RealVectorNEQS& flux, Real& wave_speed) = 0;

protected: // configuration

  /// @brief Initialize this term
  virtual void initialize();

  /// @brief allocate data to be used in flux points
  virtual void allocate_flx_pt_data();

  /// @brief Initialize caches for a given Entities component
  virtual void set_entities(const mesh::Entities& entities);

  /// @brief Set caches for element with given index
  virtual void set_element(const Uint elem_idx);

  template <typename matrix_t>
  void compute_sol_pt_gradients(const SFDElement& elem, const matrix_t& sol_pt_values, std::vector< Eigen::Matrix<Real,NDIM,NEQS> >& sol_pt_gradient);

  template <typename matrix_t>
  void compute_flx_pt_gradient(const SFDElement& elem, const matrix_t& flx_pt_values, const Uint flx_pt, Eigen::Matrix<Real,NDIM,NEQS>& flx_pt_gradient);

  /// @brief Standard computation of solution and coordinates in flux point
  ///
  /// This function NEEDS to be overloaded for terms thar require more data to be set in phys_data
  virtual void compute_flx_pt_phys_data(const SFDElement& elem, const Uint flx_pt, const std::vector< Eigen::Matrix<Real,NDIM,NEQS> >& sol_pt_gradient, PHYSDATA& phys_data );

  /// @brief Standard computation of solution and coordinates in a solution point
  ///
  /// This function has to be used for boundaries, where the neighbour-element is a face entities
  /// The physical data is then located inside the face itself.
  virtual void compute_sol_pt_phys_data(const SFDElement& elem, const Uint sol_pt, const std::vector< Eigen::Matrix<Real,NDIM,NEQS> >& sol_pt_gradient, PHYSDATA& phys_data );

  /// @brief Compute the average physical data, given a left physical data and a right physical data
  /// @param [in]  left_phys_data   left physical data
  /// @param [in]  right_phys_data  right physical data
  /// @param [out] avg_phys_data    averaged physical data
  virtual void compute_average_phys_data(const PHYSDATA& left_phys_data, const PHYSDATA& right_phys_data, PHYSDATA& avg_phys_data);

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

  std::vector< Eigen::Matrix<Real,NDIM,NEQS> > m_sol_pt_gradient;
  std::vector< Eigen::Matrix<Real,NDIM,NEQS> > m_neighbour_sol_pt_gradient;

#ifdef SDM_OUTPUT_FLUX_GNUPLOT
  Gnuplot m_gp;
  Uint m_gp_counter;
  Uint m_dump_count;
#endif

  Real m_alpha;

private:

  RealMatrix m_J;
  std::vector< Eigen::Matrix<Real,NDIM,NEQS> > LambdaL;
  std::vector< Eigen::Matrix<Real,NDIM,NEQS> > LambdaR;
  std::vector< RealVectorNEQS > flx_pt_dQL;
  std::vector< RealVectorNEQS > flx_pt_dQR;


}; // end DiffusiveTerm

////////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
DiffusiveTerm<PHYSDATA>::DiffusiveTerm( const std::string& name )
  : Term(name)
  #ifdef SDM_OUTPUT_FLUX_GNUPLOT
  , m_gp(std::string(GNUPLOT_COMMAND))
  #endif
{
  properties()["brief"] = std::string("Diffusive Spectral Difference term");
  properties()["description"] = std::string("Computes on a per cell basis the residual- and"
                                            "wave-speed contribution of a Diffusive term");


#ifdef SDM_OUTPUT_FLUX_GNUPLOT
  m_gp << "set terminal png\n";
  m_gp << "set output 'diffusive_flux.png'\n";
  //  gp << "set yrange [-1.2:1.2]\n";
//  m_gp << "set xrange [0:0.3]\n";
  m_gp << "set grid\n";
  m_gp << "set xlabel 'x'\n";
  m_gp << "set ylabel 'U'\n";
  m_gp << "set title 'flux'\n";
  m_gp_counter=0;
  m_dump_count=1;
#endif

  m_alpha=0.;
  options().add("alpha",m_alpha)
      .description("Coefficient for lifting-operator in the second approach of Bassi-Rebay")
      .link_to(&m_alpha);
}

/////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
void DiffusiveTerm<PHYSDATA>::execute()
{
//  std::cout << m_entities->uri()<<"["<<m_elem_idx<<"]"<<std::endl;

  /// 1) Calculate flux in interior flux points
  boost_foreach(flx_pt, elem->get().sf->interior_flx_pts())
  {
    compute_flx_pt_phys_data(elem->get(),flx_pt,m_sol_pt_gradient,*flx_pt_data);
    compute_flux(*flx_pt_data,flx_pt_plane_jacobian_normal->get().plane_unit_normal[flx_pt],
                  flx_pt_flux[flx_pt],flx_pt_wave_speed[flx_pt][0]);
    flx_pt_flux[flx_pt] *= flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt];
    flx_pt_wave_speed[flx_pt] *= 2*flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt];
    // the factor 2 comes from formula " CFL = 2*nu*dt/dx^2  "
  }

  /// 2) Calculate flux in face flux points
  for(m_face_nb=0; m_face_nb<elem->get().sf->nb_faces(); ++m_face_nb)
  {
    /// 2.1) Compute physical data in face
    compute_face();

    {
      for (Uint face_pt=0; face_pt<elem->get().sf->face_flx_pts(m_face_nb).size(); ++face_pt)
      {
        flx_pt = left_face_pt_idx[face_pt];
        compute_flux(*left_face_data[face_pt],flx_pt_plane_jacobian_normal->get().plane_unit_normal[flx_pt],
                      flx_pt_flux[flx_pt],flx_pt_wave_speed[flx_pt][0]);
        flx_pt_flux[flx_pt] *= flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt];
        flx_pt_wave_speed[flx_pt] *= flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt]*flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt];
      }
    }
    neighbour_elem->get().unlock();
  }

  /// 3) Compute flux divergence in solution points and store in this term's field
  mesh::Field::View term = m_term_field->view(elem->get().space->connectivity()[m_elem_idx]);
  elem->get().reconstruct_divergence_from_flux_points_to_solution_space(flx_pt_flux,term);

//  std::cout << "flx_pt_flux = ";
//  boost_foreach(const RealVectorNEQS& flx,flx_pt_flux)
//      std::cout << flx <<"   ";
//  std::cout << std::endl;
//  elem->get().reconstruct_from_flux_points_to_solution_space(0,flx_pt_flux,term);

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
      residual[sol_pt][v] += term[sol_pt][v];
    }
  }

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
      term_wave[sol_pt][0] += sol_pt_wave_speed[d][sol_pt][0]/(2.*2.);
    }
    term_wave[sol_pt][0] /= (jacob_det[sol_pt][0]*jacob_det[sol_pt][0]);
    wave_speed[sol_pt][0] = std::max(wave_speed[sol_pt][0],term_wave[sol_pt][0]);
  }

#ifdef SDM_OUTPUT_FLUX_GNUPLOT
  if (m_gp_counter == m_dump_count)
  {
//    mesh::Field::View sol_pt_coords    = solution_field().dict().coordinates().view(elem->get().space->connectivity()[elem->get().idx]);
    RealMatrix flx_pt_coords; flx_pt_coords.resize(elem->get().sf->nb_flx_pts(),NDIM);
    elem->get().reconstruct_from_geometry_space_to_flux_points(elem->get().entities->geometry_space().get_coordinates(elem->get().idx),flx_pt_coords);
//    elem->get().reconstruct_from_solution_space_to_flux_points(sol_pt_coords,flx_pt_coords);
    std::vector< std::pair<Real,Real> > flux(20);
    for (Uint i=0; i<flux.size(); ++i)
    {
      RealVector loc; loc.resize(1); loc << -1.+ 2.*i/(flux.size()-1);
      RealRowVector sf_val; sf_val.resize(elem->get().sf->nb_flx_pts());
      elem->get().sf->compute_flux_value(XX,loc,sf_val);
      flux[i].first = 0.;
      flux[i].second = 0.;
      for (Uint flx_pt=0; flx_pt<elem->get().sf->nb_flx_pts(); ++flx_pt)
      {
        flux[i].first += sf_val[flx_pt] * flx_pt_coords(flx_pt,XX);
        flux[i].second += sf_val[flx_pt] * flx_pt_flux[flx_pt][0];
      }
    }
    m_gp.send( flux );
  }
#endif
}

////////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
void DiffusiveTerm<PHYSDATA>::initialize()
{
  Term::initialize();
  elem                  = shared_caches().template get_cache< SFDElement >();
  neighbour_elem        = shared_caches().template get_cache< SFDElement >("neighbour_elem");
  flx_pt_plane_jacobian_normal = shared_caches().template get_cache< FluxPointPlaneJacobianNormal<NDIM> >();

  elem          ->options().set("space",solution_field().dict().template handle<mesh::Dictionary>());
  neighbour_elem->options().set("space",solution_field().dict().template handle<mesh::Dictionary>());
  flx_pt_plane_jacobian_normal->options().set("space",solution_field().dict().template handle<mesh::Dictionary>());

  m_J.resize(NDIM,NDIM);
}

////////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
void DiffusiveTerm<PHYSDATA>::allocate_flx_pt_data()
{
  flx_pt_data = boost::shared_ptr< PHYSDATA >( new PHYSDATA );

  left_face_pt_idx .resize(elem->get().sf->face_flx_pts(0).size());
  right_face_pt_idx.resize(elem->get().sf->face_flx_pts(0).size());
  left_face_data .resize(elem->get().sf->face_flx_pts(0).size());
  right_face_data.resize(elem->get().sf->face_flx_pts(0).size());
  for (Uint face_pt=0; face_pt<left_face_data.size(); ++face_pt)
  {
    left_face_data[face_pt]  = boost::shared_ptr< PHYSDATA >( new PHYSDATA );
    right_face_data[face_pt] = boost::shared_ptr< PHYSDATA >( new PHYSDATA );
    cf3_assert(left_face_data[face_pt] != right_face_data[face_pt]);
  }
}

////////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
void DiffusiveTerm<PHYSDATA>::set_entities(const mesh::Entities& entities)
{
  Term::set_entities(entities);

  elem->cache(m_entities);
  flx_pt_plane_jacobian_normal->cache(m_entities);

  sol_pt_wave_speed.resize(NDIM,std::vector< RealVector1 >(elem->get().sf->nb_sol_pts()));
  flx_pt_wave_speed.resize(elem->get().sf->nb_flx_pts());
  flx_pt_flux.resize(elem->get().sf->nb_flx_pts());

  allocate_flx_pt_data();

#ifdef SDM_OUTPUT_FLUX_GNUPLOT
  m_gp_counter++;
  if (m_gp_counter==m_dump_count)
  {
    std::cout << "\n\n\n\n\n\n\n\nYES YES YES AND AGAIN YES\n\n\n\n\n\n\n\n" << std::endl;
    m_gp << "plot ";
    for (Uint e=0; e<entities.size()-1; ++e)
    m_gp << "'-' with lines title 'cell"<<e<<"', ";
    m_gp << "'-' with lines title 'cell"<<entities.size()-1<<"'\n";
  }
#endif
}

////////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
void DiffusiveTerm<PHYSDATA>::set_element(const Uint elem_idx)
{
  Term::set_element(elem_idx);
  elem->set_cache(m_elem_idx);
  flx_pt_plane_jacobian_normal->set_cache(m_elem_idx);

  compute_sol_pt_gradients(elem->get(),
                           solution_field().view(elem->get().space->connectivity()[elem_idx]),
                           m_sol_pt_gradient);
}

////////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
void DiffusiveTerm<PHYSDATA>::compute_flx_pt_phys_data(const SFDElement& elem, const Uint flx_pt, const std::vector< Eigen::Matrix<Real,NDIM,NEQS> >& sol_pt_gradient, PHYSDATA& phys_data )
{
  const Uint nb_sol_pts = elem.sf->nb_sol_pts();
//  std::cout << "    " << elem.entities->uri() << "["<<elem.idx<<"]   flx_pt[" <<flx_pt<<"]"<<std::endl;
  mesh::Field::View sol_pt_solution  = solution_field().view(elem.space->connectivity()[elem.idx]);
  mesh::Field::View sol_pt_coords    = solution_field().dict().coordinates().view(elem.space->connectivity()[elem.idx]);
  mesh::Field::View sol_pt_jacob_det = jacob_det_field().view(elem.space->connectivity()[elem.idx]);
//  std::cout << "    reconstruct solution" << std::endl;
  elem.reconstruct_from_solution_space_to_flux_points[flx_pt](sol_pt_solution,phys_data.solution);
//  std::cout << "    reconstruct coordinates" << std::endl;
  elem.reconstruct_from_solution_space_to_flux_points[flx_pt](sol_pt_coords,phys_data.coord);

  // INTERPOLATE GRADIENT TO THIS FLUX POINT
  RealMatrix sol_pt_derivative; sol_pt_derivative.resize(nb_sol_pts,NEQS);
  for (Uint d=0; d<NDIM; ++d)
  {
    for (Uint sol_pt=0; sol_pt<nb_sol_pts; ++sol_pt)
    {
      sol_pt_derivative.row(sol_pt) = sol_pt_gradient[sol_pt].row(d);
    }
    elem.reconstruct_from_solution_space_to_flux_points[flx_pt](sol_pt_derivative,phys_data.solution_gradient.row(d));
  }
}

////////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
template <typename matrix_t>
void DiffusiveTerm<PHYSDATA>::compute_sol_pt_gradients(const SFDElement& elem, const matrix_t& sol_pt_values, std::vector< Eigen::Matrix<Real,NDIM,NEQS> >& sol_pt_gradient)
{
  const Uint nb_sol_pts = elem.sf->nb_sol_pts();
  sol_pt_gradient.resize(nb_sol_pts);

  // COMPUTE GRADIENTS IN SOLUTION POINTS
  RealMatrix geom_coord_matrix = elem.entities->geometry_space().get_coordinates(elem.idx);
  RealMatrix sf_grad; sf_grad.resize(NDIM,nb_sol_pts);

  for (Uint sol_pt=0; sol_pt<nb_sol_pts; ++sol_pt)
  {
    sol_pt_gradient[sol_pt].setZero();

    // compute gradient of solution in local coordinates (dQ/dxi, dQ/deta, dQ/dzeta)
    elem.sf->compute_gradient(elem.sf->sol_pts().row(sol_pt), sf_grad);
    for (Uint pt=0; pt<nb_sol_pts; ++pt)
    {
      for (Uint v=0; v<NEQS; ++v)
      {
        for (Uint d=0; d<NDIM; ++d)
        {
          sol_pt_gradient[sol_pt](d,v) += sol_pt_values[pt][v] * sf_grad(d,pt);
        }
      }
    }
    // transform to physical space: (dQ/dx, dQ/dy, dQ/dz)
    elem.entities->element_type().compute_jacobian(elem.sf->sol_pts().row(sol_pt),geom_coord_matrix,m_J);
    sol_pt_gradient[sol_pt] = m_J.inverse() * sol_pt_gradient[sol_pt];
  }
}

////////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
template <typename matrix_t>
void DiffusiveTerm<PHYSDATA>::compute_flx_pt_gradient(const SFDElement& elem, const matrix_t& flx_pt_values, const Uint flx_pt, Eigen::Matrix<Real,NDIM,NEQS>& flx_pt_gradient)
{
  const Uint nb_flx_pts = elem.sf->nb_flx_pts();

  RealMatrix geom_coord_matrix = elem.entities->geometry_space().get_coordinates(elem.idx);
  RealVector sf_deriv; sf_deriv.resize(nb_flx_pts);

  flx_pt_gradient.setZero();

  // compute gradient of solution in local coordinates (dQ/dxi, dQ/deta, dQ/dzeta)
  for (Uint d=0; d<NDIM; ++d)
  {
    elem.sf->compute_flux_derivative(elem.sf->flx_pt_dirs(flx_pt)[0],elem.sf->flx_pts().row(flx_pt), sf_deriv);
    for (Uint v=0; v<NEQS; ++v)
    {
      for (Uint pt=0; pt<nb_flx_pts; ++pt)
        flx_pt_gradient(d,v) += flx_pt_values[pt][v] * sf_deriv[pt];
    }
  }

  // transform to physical space: (dQ/dx, dQ/dy, dQ/dz)
  elem.entities->element_type().compute_jacobian(elem.sf->flx_pts().row(flx_pt),geom_coord_matrix,m_J);
  flx_pt_gradient = m_J.inverse() * flx_pt_gradient;
}

////////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
void DiffusiveTerm<PHYSDATA>::compute_sol_pt_phys_data(const SFDElement& elem, const Uint sol_pt, const std::vector< Eigen::Matrix<Real,NDIM,NEQS> >& sol_pt_gradient, PHYSDATA& phys_data)
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
void DiffusiveTerm<PHYSDATA>::compute_average_phys_data(const PHYSDATA& left_phys_data, const PHYSDATA& right_phys_data, PHYSDATA& avg_phys_data)
{
  /// This simple procedure corresponds to Bassi-Rebay second approach, with coefficient alpha=0, meaning no lifting operator is necessary
  /// This is not consistent for P0 shape-functions, as there the average is zero, and only the lifting operator can play a role
  avg_phys_data.solution = 0.5*(left_phys_data.solution + right_phys_data.solution);
  avg_phys_data.solution_gradient = 0.5*(left_phys_data.solution_gradient + right_phys_data.solution_gradient);
}

////////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
void DiffusiveTerm<PHYSDATA>::set_connectivity()
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
void DiffusiveTerm<PHYSDATA>::compute_face()
{
  Uint face_side;

  /// 1) Find the neighbour element and face entity
  set_face(m_entities,m_elem_idx,m_face_nb,
           neighbour_entities,neighbour_elem_idx,neighbour_face_nb,
           face_entities,face_idx,face_side);

  /// 2) Set connectivity from face points on the left side to face points on the right side
  set_connectivity();

  const Uint nb_face_pts = elem->get().sf->face_flx_pts(m_face_nb).size();

  /// 3) Compute physical data in left face points
  for (Uint face_pt=0; face_pt<nb_face_pts; ++face_pt)
    compute_flx_pt_phys_data(elem->get(),left_face_pt_idx[face_pt],m_sol_pt_gradient,*left_face_data[face_pt]);

  /// 4) Compute physical data in right face points

  ///  * Case there is a neighbour cell
  if ( is_not_null(neighbour_entities) )
  {
    compute_sol_pt_gradients(neighbour_elem->get(),
                             solution_field().view(neighbour_elem->get().space->connectivity()[neighbour_elem->get().idx]),
                             m_neighbour_sol_pt_gradient);
    for (Uint face_pt=0; face_pt<nb_face_pts; ++face_pt)
      compute_flx_pt_phys_data(neighbour_elem->get(),right_face_pt_idx[face_pt],m_neighbour_sol_pt_gradient,*right_face_data[face_pt]);
  }
  /// * Case there is no neighbour, but physical data inside the face solution points
  else
  {
    neighbour_elem->cache(face_entities,face_idx);
    for (Uint face_pt=0; face_pt<right_face_pt_idx.size(); ++face_pt)
      compute_sol_pt_phys_data(neighbour_elem->get(),right_face_pt_idx[face_pt],m_sol_pt_gradient,*right_face_data[face_pt]);
    for (Uint face_pt=0; face_pt<right_face_pt_idx.size(); ++face_pt)
    {
      right_face_data[face_pt]->solution_gradient = left_face_data[face_pt]->solution_gradient;
    }
    //std::cout << "right_face_data.solution = " << right_face_data[0]->solution << std::endl;
  }

  /// 5) Lifting operator
  LambdaL.resize(nb_face_pts);
  LambdaR.resize(nb_face_pts);
  for (Uint face_pt=0; face_pt<nb_face_pts; ++face_pt)
  {
    LambdaL[face_pt].setZero();
    LambdaR[face_pt].setZero();
  }

  flx_pt_dQL.resize(elem->get().sf->nb_flx_pts());
  for (Uint i=0; i<elem->get().sf->nb_flx_pts(); ++i)
    flx_pt_dQL[i].setZero();

  for (Uint face_pt=0; face_pt<nb_face_pts; ++face_pt)
  {
    flx_pt_dQL[left_face_pt_idx[face_pt]] =
        (right_face_data[face_pt]->solution - left_face_data[face_pt]->solution)
        ;//* elem->get().sf->flx_pt_sign(left_face_pt_idx[face_pt]);
  }
  for (Uint face_pt=0; face_pt<nb_face_pts; ++face_pt)
  {
    compute_flx_pt_gradient(elem->get(),flx_pt_dQL,left_face_pt_idx[face_pt],LambdaL[face_pt]);
  }
  if ( is_not_null(neighbour_entities) )
  {
    flx_pt_dQR.resize(neighbour_elem->get().sf->nb_flx_pts());
    for (Uint i=0; i<neighbour_elem->get().sf->nb_flx_pts(); ++i)
      flx_pt_dQR[i].setZero();

    for (Uint face_pt=0; face_pt<nb_face_pts; ++face_pt)
    {
      flx_pt_dQR[right_face_pt_idx[face_pt]] =
          - (right_face_data[face_pt]->solution - left_face_data[face_pt]->solution)
          ;//* elem->get().sf->flx_pt_sign(left_face_pt_idx[face_pt]);
    }
    for (Uint face_pt=0; face_pt<nb_face_pts; ++face_pt)
    {
      compute_flx_pt_gradient(neighbour_elem->get(),flx_pt_dQR,right_face_pt_idx[face_pt],LambdaR[face_pt]);
    }
  }
  else
  {
    LambdaR = LambdaL;
  }
  for (Uint face_pt=0; face_pt<nb_face_pts; ++face_pt)
  {
    compute_average_phys_data(*left_face_data[face_pt],*right_face_data[face_pt], *left_face_data[face_pt]);
//    std::cout << "left_face_data[face_pt]->solution_gradient = " << left_face_data[face_pt]->solution_gradient << std::endl;
    left_face_data[face_pt]->solution_gradient += m_alpha * 0.5*(LambdaL[face_pt]+LambdaR[face_pt]);
//    std::cout << "m_alpha * 0.5*(LambdaL[face_pt]+LambdaR[face_pt]) = \n" << m_alpha * 0.5*(LambdaL[face_pt]+LambdaR[face_pt]) << std::endl;
  }


}

//template <typename PHYSDATA>
//void DiffusiveTerm<PHYSDATA>::compute_face()
//{
//  static Uint face_side;

//  cf3_assert(left_face_data[0]!=right_face_data[0]);

//  /// 1) Find the neighbour element and face entity
//  set_face(m_entities,m_elem_idx,m_face_nb,
//           neighbour_entities,neighbour_elem_idx,neighbour_face_nb,
//           face_entities,face_idx,face_side);

//  /// 2) Set connectivity from face points on the left side to face points on the right side
//  set_connectivity();

//  const Uint nb_face_pts = elem->get().sf->face_flx_pts(m_face_nb).size();
//  /// 3) Compute physical data in left face points
//  for (Uint face_pt=0; face_pt<nb_face_pts; ++face_pt)
//    compute_flx_pt_phys_data(elem->get(),left_face_pt_idx[face_pt],m_sol_pt_gradient,*left_face_data[face_pt]);

//  /// 4) Compute physical data in right face points
//  std::vector< Eigen::Matrix<Real,NDIM,NEQS> > LambdaL(nb_face_pts);
//  std::vector< Eigen::Matrix<Real,NDIM,NEQS> > LambdaR(nb_face_pts);
//  for (Uint face_pt=0; face_pt<nb_face_pts; ++face_pt)
//  {
//    LambdaL[face_pt].setZero();
//    LambdaR[face_pt].setZero();
//  }


//  ///  * Case there is a neighbour cell
//  if ( is_not_null(neighbour_entities) )
//  {
//    compute_sol_pt_gradients(neighbour_elem->get(),m_neighbour_sol_pt_gradient);
//    for (Uint face_pt=0; face_pt<nb_face_pts; ++face_pt)
//      compute_flx_pt_phys_data(neighbour_elem->get(),right_face_pt_idx[face_pt],m_neighbour_sol_pt_gradient,*right_face_data[face_pt]);
//  }
//  /// * Case there is no neighbour, but physical data inside the face solution points
//  else
//  {
//    neighbour_elem->cache(face_entities,face_idx);
//    for (Uint face_pt=0; face_pt<right_face_pt_idx.size(); ++face_pt)
//      compute_sol_pt_phys_data(neighbour_elem->get(),right_face_pt_idx[face_pt],m_sol_pt_gradient,*right_face_data[face_pt]);
//    for (Uint face_pt=0; face_pt<right_face_pt_idx.size(); ++face_pt)
//    {
//      right_face_data[face_pt]->solution_gradient = left_face_data[face_pt]->solution_gradient;
//    }
//    //std::cout << "right_face_data.solution = " << right_face_data[0]->solution << std::endl;

//  }





//  // COMPUTE LEFT LIFTING OPERATOR
//  {
//    RealMatrix geom_coord_matrix = elem->get().entities->geometry_space().get_coordinates(elem->get().idx);
//    RealMatrix J; J.resize(NDIM,NDIM);

//    std::vector< RealVectorNEQS > dQL(elem->get().sf->nb_flx_pts());
//    boost_foreach(RealVectorNEQS& dql, dQL)
//        dql.setZero();

//    RealRowVector interpol_coeffs(elem->get().sf->nb_sol_pts());
//    RealVector flux_derivative_coeffs(dQL.size());


////    RealMatrix geom_coord_matrix = elem->get().entities->geometry_space().get_coordinates(elem->get().idx);
//    for (Uint face_pt=0; face_pt<nb_face_pts; ++face_pt)
//    {
//      dQL[left_face_pt_idx[face_pt]] = flx_pt_plane_jacobian_normal->get().plane_jacobian_normal[left_face_pt_idx[face_pt]] * (right_face_data[face_pt]->solution - left_face_data[face_pt]->solution);
////      std::cout << "(right_face_data[face_pt]->solution - left_face_data[face_pt]->solution) = " << (right_face_data[face_pt]->solution - left_face_data[face_pt]->solution).transpose() << std::endl;
////      std::cout << "dQL[left_face_pt_idx[face_pt]] = " << dQL[left_face_pt_idx[face_pt]].transpose() << std::endl;
//    }
//    std::vector< Eigen::Matrix<Real,NDIM,NEQS> > sol_pt_LambdaL(elem->get().sf->nb_sol_pts());
////    neighbour_elem->get().reconstruct_divergence_from_flux_points_to_solution_space(dQR,sol_pt_LambdaR);

//    mesh::Field::View jacob_det = jacob_det_field().view(elem->get().space->connectivity()[elem->get().idx]);
//    for (Uint sol_pt=0; sol_pt<sol_pt_LambdaL.size(); ++sol_pt)
//    {
//      sol_pt_LambdaL[sol_pt].setZero();
//      for (Uint d=0; d<NDIM; ++d)
//      {
//        flux_derivative_coeffs.setZero();
//        elem->get().sf->compute_flux_derivative(d, elem->get().sf->sol_pts().row(sol_pt), flux_derivative_coeffs);
//        for (Uint face_pt=0; face_pt<nb_face_pts; ++face_pt)
//        {
//          sol_pt_LambdaL[sol_pt].row(d) += flux_derivative_coeffs[left_face_pt_idx[face_pt]] * dQL[left_face_pt_idx[face_pt]].transpose();
//        }
//      }
//      sol_pt_LambdaL[sol_pt] /= jacob_det[sol_pt][0];
//    }

//    // left side
//    {
//      mesh::Field::View sol_pt_jacob_det = jacob_det_field().view(elem->get().space->connectivity()[elem->get().idx]);
//      for (Uint sol_pt=0; sol_pt<elem->get().sf->nb_sol_pts(); ++sol_pt)
//      {
//        sol_pt_LambdaL[sol_pt]/=sol_pt_jacob_det[sol_pt][0];
//      }
//      for (Uint face_pt=0; face_pt<nb_face_pts; ++face_pt)
//      {
//        elem->get().sf->compute_value(elem->get().sf->flx_pts().row(left_face_pt_idx[face_pt]), interpol_coeffs);
//        for (Uint sol_pt=0; sol_pt<elem->get().sf->nb_sol_pts(); ++sol_pt)
//        {
//          LambdaL[face_pt] += interpol_coeffs[sol_pt] * sol_pt_LambdaL[sol_pt];
//        }
//      }
////      elem->get().reconstruct_from_solution_space_to_flux_points[left_face_pt_idx[face_pt]](sol_pt_LambdaL,LambdaL[face_pt]);
//    }
//  }


//  // COMPUTE RIGHT LIFTING OPERATOR
//  if (is_null(neighbour_entities)) // if boundary face
//  {
//    LambdaR = LambdaL;
//    std::cout << "LambdaL = " << LambdaL[0].transpose() << std::endl;
//  }
//  else // if interior face
//  {
//    RealMatrix geom_coord_matrix = neighbour_elem->get().entities->geometry_space().get_coordinates(neighbour_elem->get().idx);
//    RealMatrix J; J.resize(NDIM,NDIM);

//    std::vector<RealVectorNEQS> dQR(neighbour_elem->get().sf->nb_flx_pts());
//    boost_foreach(RealVectorNEQS& dqr, dQR)
//        dqr.setZero();
//    RealVector flux_derivative_coeffs(dQR.size());
//    RealRowVector interpol_coeffs(neighbour_elem->get().sf->nb_sol_pts());

//    for (Uint face_pt=0; face_pt<nb_face_pts; ++face_pt)
//    {
//      dQR[right_face_pt_idx[face_pt]] = -(right_face_data[face_pt]->solution - left_face_data[face_pt]->solution)
//                                        * flx_pt_plane_jacobian_normal->get().plane_jacobian_normal[left_face_pt_idx[face_pt]].cwiseAbs().transpose()
//                                        * flx_pt_plane_jacobian_normal->get().plane_unit_normal[left_face_pt_idx[face_pt]];
////      std::cout << "dQL = " << dQL[left_face_pt_idx[face_pt]].transpose() << std::endl;
////      std::cout << "dQR = " << dQR[right_face_pt_idx[face_pt]].transpose() << std::endl;
//    }

//    std::vector< Eigen::Matrix<Real,NDIM,NEQS> > sol_pt_LambdaR(neighbour_elem->get().sf->nb_sol_pts());
////    neighbour_elem->get().reconstruct_divergence_from_flux_points_to_solution_space(dQR,sol_pt_LambdaR);

//    for (Uint sol_pt=0; sol_pt<sol_pt_LambdaR.size(); ++sol_pt)
//    {
//      sol_pt_LambdaR[sol_pt].setZero();
//      for (Uint d=0; d<NDIM; ++d)
//      {
//        flux_derivative_coeffs.setZero();
//        neighbour_elem->get().sf->compute_flux_derivative(d, neighbour_elem->get().sf->sol_pts().row(sol_pt), flux_derivative_coeffs);
//        for (Uint face_pt=0; face_pt<nb_face_pts; ++face_pt)
//        {
//          sol_pt_LambdaR[sol_pt].row(d) += flux_derivative_coeffs[right_face_pt_idx[face_pt]] * dQR[right_face_pt_idx[face_pt]];
//        }
//      }
//      neighbour_elem->get().entities->element_type().compute_jacobian(neighbour_elem->get().sf->sol_pts().row(sol_pt),geom_coord_matrix,J);
//      sol_pt_LambdaR[sol_pt] = J.inverse()* sol_pt_LambdaR[sol_pt];
//    }

//    // right side
//    {
//      mesh::Field::View sol_pt_jacob_det = jacob_det_field().view(neighbour_elem->get().space->connectivity()[neighbour_elem->get().idx]);
//      for (Uint sol_pt=0; sol_pt<neighbour_elem->get().sf->nb_sol_pts(); ++sol_pt)
//      {
//        sol_pt_LambdaR[sol_pt]/=sol_pt_jacob_det[sol_pt][0];
//      }
//      for (Uint face_pt=0; face_pt<nb_face_pts; ++face_pt)
//      {

//        elem->get().sf->compute_value(neighbour_elem->get().sf->flx_pts().row(right_face_pt_idx[face_pt]), interpol_coeffs);
//        for (Uint sol_pt=0; sol_pt<neighbour_elem->get().sf->nb_sol_pts(); ++sol_pt)
//        {
//          LambdaR[face_pt] += interpol_coeffs[sol_pt] * sol_pt_LambdaR[sol_pt];
//        }
//      }
//    }
//  }
//  for (Uint face_pt=0; face_pt<nb_face_pts; ++face_pt)
//  {
//    compute_average_phys_data(*left_face_data[face_pt],*right_face_data[face_pt], *left_face_data[face_pt]);
////    std::cout << "left_face_data[face_pt]->solution_gradient = " << left_face_data[face_pt]->solution_gradient << std::endl;
//    left_face_data[face_pt]->solution_gradient += m_alpha * 0.5*(LambdaL[face_pt]+LambdaR[face_pt]);
////    std::cout << "m_alpha * 0.5*(LambdaL[face_pt]+LambdaR[face_pt]) = \n" << m_alpha * 0.5*(LambdaL[face_pt]+LambdaR[face_pt]) << std::endl;
//  }
//}

////////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
void DiffusiveTerm<PHYSDATA>::unset_element()
{
  Term::unset_element();
  elem->get().unlock();
  flx_pt_plane_jacobian_normal->get().unlock();
}

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_DiffusiveTerm_hpp
