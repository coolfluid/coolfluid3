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

#include "mesh/Connectivity.hpp"

#include "SFDM/Tags.hpp"
#include "SFDM/Term.hpp"
#include "SFDM/ShapeFunction.hpp"
#include "SFDM/Operations.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {

//////////////////////////////////////////////////////////////////////////////

/// @todo this will be replaced eventually with physics properties
template <Uint NEQS, Uint NDIM>
struct ConvectiveTermPointData
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  enum { _ndim = NDIM }; ///< number of dimensions
  enum { _neqs = NEQS }; ///< number of independent variables or equations

  Eigen::Matrix<Real,NEQS,1> solution;
  Eigen::Matrix<Real,NDIM,1> coord;
};

/// A proposed base class for simple convective terms.
/// Classes inheriting only need to implement functions to compute
/// - analytical flux for interior flux points
/// - numerical flux for face flux points
/// @author Willem Deconinck
template <typename PHYS>
class ConvectiveTerm : public Term
{
public: // typedefs
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  enum { NDIM = PHYS::_ndim }; ///< number of dimensions
  enum { NEQS = PHYS::_neqs }; ///< number of independent variables or equations

public: // functions

  /// constructor
  ConvectiveTerm( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "ConvectiveTerm"; }

  virtual void execute();

private: // functions

  // Flux evaluations
  // ----------------
  virtual void compute_analytical_flux(PHYS& data, const Eigen::Matrix<Real,NDIM,1>& unit_normal, Eigen::Matrix<Real,NEQS,1>& flux, Real& wave_speed) = 0;
  virtual void compute_numerical_flux(PHYS& left, PHYS& right, const Eigen::Matrix<Real,NDIM,1>& unit_normal, Eigen::Matrix<Real,NEQS,1>& flux, Real& wave_speed) = 0;

protected: // configuration

  virtual void initialize()
  {
    Term::initialize();
    elem                  = shared_caches().template get_cache< SFDElement >();
    neighbour_elem        = shared_caches().template get_cache< SFDElement >("neighbour_elem");

    divergence_in_solution_points  = shared_caches().template get_cache< FluxPointDivergence >();
    reconstruct_in_solution_points = shared_caches().template get_cache< FluxPointReconstruct >();
    flx_pt_plane_jacobian_normal = shared_caches().template get_cache< FluxPointPlaneJacobianNormal<NDIM> >();

    elem->options().configure_option("space",solution_field().space());
    neighbour_elem->options().configure_option("space",solution_field().space());
    divergence_in_solution_points->options().configure_option("space",solution_field().space());
    reconstruct_in_solution_points->options().configure_option("space",solution_field().space());
    flx_pt_plane_jacobian_normal->options().configure_option("space",solution_field().space());

  }

  virtual void allocate_flx_pt_data()
  {
    flx_pt_data = boost::shared_ptr< ConvectiveTermPointData<NEQS,NDIM> >( new ConvectiveTermPointData<NEQS,NDIM> );

    left_flx_pt_idx.resize(elem->get().sf->face_flx_pts(0).size());
    right_flx_pt_idx.resize(elem->get().sf->face_flx_pts(0).size());
    left_face_data .resize(elem->get().sf->face_flx_pts(0).size());
    right_face_data .resize(elem->get().sf->face_flx_pts(0).size());
    for (Uint face_pt=0; face_pt<left_face_data.size(); ++face_pt)
    {
      left_face_data[face_pt] = boost::shared_ptr< ConvectiveTermPointData<NEQS,NDIM> >( new ConvectiveTermPointData<NEQS,NDIM> );
      right_face_data[face_pt] = boost::shared_ptr< ConvectiveTermPointData<NEQS,NDIM> >( new ConvectiveTermPointData<NEQS,NDIM> );
    }
  }

  virtual void set_entities(const mesh::Entities& entities)
  {
    Term::set_entities(entities);

    elem->cache(m_entities);
    flx_pt_plane_jacobian_normal->cache(m_entities);
    divergence_in_solution_points->cache(m_entities);
    reconstruct_in_solution_points->cache(m_entities);

    sol_pt_wave_speed.resize(NDIM,std::vector<Eigen::Matrix<Real,1,1> >(elem->get().sf->nb_sol_pts()));
    flx_pt_wave_speed.resize(elem->get().sf->nb_flx_pts());
    flx_pt_flux.resize(elem->get().sf->nb_flx_pts());

    allocate_flx_pt_data();

  }

  virtual void set_element(const Uint elem_idx)
  {
    Term::set_element(elem_idx);
    elem->set_cache(m_elem_idx);
    flx_pt_plane_jacobian_normal->set_cache(m_elem_idx);
  }

  virtual void reconstruct_flx_pt_data(const SFDElement& elem, const Uint flx_pt, PHYS& phys_data )
  {
    mesh::Field::View sol_pt_solution = solution_field().view(elem.space->indexes_for_element(elem.idx));
    elem.reconstruct_solution_space_to_flux_points[flx_pt](sol_pt_solution,phys_data.solution);
    elem.reconstruct_geometry_space_to_flux_points[flx_pt](elem.entities->get_coordinates(elem.idx),phys_data.coord);
  }

  virtual void compute_face()
  {

    Uint face_side;
    set_face(m_entities,m_elem_idx,m_face_nb,
             neighbour_entities,neighbour_elem_idx,neighbour_face_nb,
             face_entities,face_idx,face_side);


    left_flx_pt_idx = elem->get().sf->face_flx_pts(m_face_nb);


    for (Uint face_pt=0; face_pt<elem->get().sf->face_flx_pts(m_face_nb).size(); ++face_pt)
      reconstruct_flx_pt_data(elem->get(),left_flx_pt_idx[face_pt],*left_face_data[face_pt]);


    if ( is_not_null(neighbour_entities) )
    {
      neighbour_elem->cache(neighbour_entities,neighbour_elem_idx);

      // if order(neighbour_elem) != order(elem)
      //    {
      //      interpolate neighbour-face to right_face_data
      //    }

      Uint nb_neighbour_face_pts = neighbour_elem->get().sf->face_flx_pts(neighbour_face_nb).size();
      for(Uint face_pt=0; face_pt<nb_neighbour_face_pts; ++face_pt)
      {
        right_flx_pt_idx[face_pt] = neighbour_elem->get().sf->face_flx_pts(neighbour_face_nb)[nb_neighbour_face_pts-1-face_pt];

      }
      for (Uint face_pt=0; face_pt<elem->get().sf->face_flx_pts(m_face_nb).size(); ++face_pt)
        reconstruct_flx_pt_data(neighbour_elem->get(),right_flx_pt_idx[face_pt],*right_face_data[face_pt]);
    }
    else
    {
      neighbour_elem->cache(face_entities,face_idx);

      if (face_entities->has_tag(mesh::Tags::outer_faces())) // no data can be set
      {
        for (Uint face_pt=0; face_pt<elem->get().sf->face_flx_pts(m_face_nb).size(); ++face_pt)
        {
          right_face_data[face_pt]->solution = left_face_data[face_pt]->solution;
          right_face_data[face_pt]->coord = left_face_data[face_pt]->coord;
          right_flx_pt_idx[face_pt]=left_flx_pt_idx[face_pt];
        }
      }
      else
      {
        for (Uint face_pt=0; face_pt<right_flx_pt_idx.size(); ++face_pt)
        {
  //        common::TableConstRow<Uint>::type field_index = neighbour_elem->get().space->indexes_for_element(neighbour_elem->get().idx);
  //        std::cout << field_index.shape()[0] << "x" << field_index.shape()[1] << std::endl;
  //        std::cout << right_flx_pt_idx.size() << std::endl;
  //        std::cout << "convective_term -- " << neighbour_elem->get().entities->uri() << "[" << neighbour_elem->get().idx << "]" << " : face_points = " << field_index[face_pt] << "  ---> " ;

          right_flx_pt_idx[face_pt] = face_pt;

          mesh::Field::View boundary_face_pt_solution = solution_field().view(neighbour_elem->get().space->indexes_for_element(neighbour_elem->get().idx));
          for (Uint v=0; v<NEQS; ++v)
            right_face_data[face_pt]->solution[v] = boundary_face_pt_solution[face_pt][v];
          right_face_data[face_pt]->coord = neighbour_elem->get().space->get_coordinates(neighbour_elem->get().idx).row(face_pt);
        }


        boost::shared_ptr<PHYS> tmp_data;
        Uint tmp_idx;
        for (Uint face_pt=0; face_pt<elem->get().sf->face_flx_pts(m_face_nb).size(); ++face_pt)
        {
          Uint right_face_pt=0;
          bool matched=false;

          for (; right_face_pt<elem->get().sf->face_flx_pts(m_face_nb).size(); ++right_face_pt)
          {
            bool m=true;
  //          std::cout << "right = " << right_face_data[right_face_pt]->coord.transpose() << std::endl;
            for (Uint d=0; d<NDIM; ++d)
            {
              m = m && ( std::abs(left_face_data[face_pt]->coord[d] - right_face_data[right_face_pt]->coord[d]) < 20*math::Consts::eps() );
            }
            if ( m )
            {
              matched=true;
              break;
            }
          }
  //        if (!matched)
  //        {
  //          std::cout << "coord " << left_face_data[face_pt]->coord.transpose() << std::endl;
            cf3_assert_desc(elem->get().space->uri().string()+"["+common::to_str(elem->get().idx)+"]",matched);
  //        }


          tmp_data=right_face_data[face_pt];
          tmp_idx=right_flx_pt_idx[face_pt];
          right_face_data[face_pt]=right_face_data[right_face_pt];
          right_flx_pt_idx[face_pt]=right_flx_pt_idx[right_face_pt];
          right_face_data[right_face_pt]=tmp_data;
          right_flx_pt_idx[right_face_pt]=tmp_idx;

        }


      }


    }
//    boost::shared_ptr<PHYS> tmp_data;
//    Uint tmp_idx;
//    for (Uint face_pt=0; face_pt<elem->get().sf->face_flx_pts(m_face_nb).size(); ++face_pt)
//    {
//      Uint right_face_pt=0;
//      bool matched=false;
//      for (; right_face_pt<elem->get().sf->face_flx_pts(m_face_nb).size(); ++right_face_pt)
//      {

//        if (left_face_data[face_pt]->coord == right_face_data[right_face_pt]->coord)
//        {
//          matched=true;
//          break;
//        }
//      }
//      cf3_assert(matched);

//      tmp_data=right_face_data[face_pt];
//      tmp_idx=right_flx_pt_idx[face_pt];
//      right_face_data[face_pt]=right_face_data[right_face_pt];
//      right_flx_pt_idx[face_pt]=right_flx_pt_idx[right_face_pt];
//      right_face_data[right_face_pt]=tmp_data;
//      right_flx_pt_idx[right_face_pt]=tmp_idx;

//    }
//    for (Uint face_pt=0; face_pt<elem->get().sf->face_flx_pts(m_face_nb).size(); ++face_pt)
//    {
//      if(left_face_data[face_pt]->coord != right_face_data[face_pt]->coord)
//      {
//        std::cout << m_entities->uri() << "["<<m_elem_idx<<"]  <--> " << neighbour_elem->get().entities->uri() << "["<<neighbour_elem->get().idx<<"]"<<std::endl;
//        cf3_assert(false);
//      }
//    }


  }

  virtual void unset_element()
  {
    Term::unset_element();
    elem->get().unlock();
    flx_pt_plane_jacobian_normal->get().unlock();
  }


protected: // fast-access-data (for convenience no "m_" prefix)

  boost::shared_ptr< PHYS > flx_pt_data;
  std::vector<boost::shared_ptr< PHYS > > left_face_data;
  std::vector<boost::shared_ptr< PHYS > > right_face_data;
  std::vector<Uint> left_flx_pt_idx;
  std::vector<Uint> right_flx_pt_idx;

  // Data
  Uint sol_pt;
  Uint flx_pt;
  Handle<mesh::Entities const> neighbour_entities;
  Uint neighbour_elem_idx;
  Uint neighbour_face_nb;
  Handle<mesh::Entities const> face_entities;
  Uint face_idx;

  // In flux points:
  Handle< CacheT<SFDElement> > elem;
  Handle< CacheT<SFDElement> > neighbour_elem;
  Handle< CacheT<FluxPointDivergence> > divergence_in_solution_points;
  Handle< CacheT<FluxPointReconstruct> > reconstruct_in_solution_points;
  Handle< CacheT<FluxPointPlaneJacobianNormal<NDIM> > > flx_pt_plane_jacobian_normal;

  std::vector< Eigen::Matrix<Real,NEQS,1> >   flx_pt_flux;
  std::vector< Eigen::Matrix<Real,1,1> >   flx_pt_wave_speed;
  std::vector< std::vector<Eigen::Matrix<Real,1,1> > > sol_pt_wave_speed;

}; // end ConvectiveTerm

////////////////////////////////////////////////////////////////////////////////

template <typename PHYS>
ConvectiveTerm<PHYS>::ConvectiveTerm( const std::string& name )
  : Term(name)
{
  properties()["brief"] = std::string("Convective Spectral Finite Difference term");
  properties()["description"] = std::string("Fields to be created: ...");
}

/////////////////////////////////////////////////////////////////////////////

template <typename PHYS>
void ConvectiveTerm<PHYS>::execute()
{

  boost_foreach(flx_pt, elem->get().sf->interior_flx_pts())
  {
    reconstruct_flx_pt_data(elem->get(),flx_pt,*flx_pt_data);
    compute_analytical_flux(*flx_pt_data,flx_pt_plane_jacobian_normal->get().plane_unit_normal[flx_pt],
                            flx_pt_flux[flx_pt],flx_pt_wave_speed[flx_pt][0]);
    flx_pt_flux[flx_pt] *= flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt];
    flx_pt_wave_speed[flx_pt] *= flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt];
  }

  for(m_face_nb=0; m_face_nb<elem->get().sf->nb_faces(); ++m_face_nb)
  {
    compute_face();
//    if ( is_not_null(neighbour_entities) )
    {
      for (Uint face_pt=0; face_pt<elem->get().sf->face_flx_pts(m_face_nb).size(); ++face_pt)
      {
//        cf3_assert(face_pt<left_flx_pt_idx.size());
//        cf3_assert(face_pt<right_flx_pt_idx.size());
//        cf3_assert(is_not_null(left_face_data[left_flx_pt_idx[face_pt]]));
//        cf3_assert(is_not_null(right_face_data[right_flx_pt_idx[face_pt]]));
        flx_pt = left_flx_pt_idx[face_pt];
        compute_numerical_flux(*left_face_data[face_pt],*right_face_data[face_pt],flx_pt_plane_jacobian_normal->get().plane_unit_normal[flx_pt] * elem->get().sf->flx_pt_sign(flx_pt),
                               flx_pt_flux[flx_pt],flx_pt_wave_speed[flx_pt][0]);
        flx_pt_flux[flx_pt] *= elem->get().sf->flx_pt_sign(flx_pt) * flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt];
        flx_pt_wave_speed[flx_pt] *= flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt];
      }
    }
//    else
//    {
//      for (Uint face_pt=0; face_pt<elem->get().sf->face_flx_pts(m_face_nb).size(); ++face_pt)
//      {
//        flx_pt = left_flx_pt_idx[face_pt];

//        compute_analytical_flux(*left_face_data[face_pt],flx_pt_plane_jacobian_normal->get().plane_unit_normal[flx_pt],
//                                flx_pt_flux[flx_pt],flx_pt_wave_speed[flx_pt][0]);
//        flx_pt_flux[flx_pt] *= flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt];
//        flx_pt_wave_speed[flx_pt] *= flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt];
//      }
//    }
    neighbour_elem->get().unlock();
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
