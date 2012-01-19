// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_BCWeak_hpp
#define cf3_SFDM_BCWeak_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/PropertyList.hpp"

#include "math/MatrixTypes.hpp"
#include "math/VectorialFunction.hpp"

#include "SFDM/Tags.hpp"
#include "SFDM/BC.hpp"
#include "SFDM/ShapeFunction.hpp"
#include "SFDM/Operations.hpp"

#include <boost/bind.hpp>
#include <boost/function.hpp>

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {

//////////////////////////////////////////////////////////////////////////////

/// @todo this will be replaced eventually with physics properties
template <Uint NEQS, Uint NDIM>
struct BCPointData
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  enum { _ndim = NDIM }; ///< number of dimensions
  enum { _neqs = NEQS }; ///< number of independent variables or equations

  Eigen::Matrix<Real,NEQS,1> solution;
  Eigen::Matrix<Real,NDIM,1> coord;
};

/// @author Willem Deconinck
template <typename POINTDATA>
class SFDM_API BCWeak : public BC
{
public: // typedefs
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  enum { NDIM = POINTDATA::_ndim }; ///< number of dimensions
  enum { NEQS = POINTDATA::_neqs }; ///< number of independent variables or equations
public: // functions

  /// constructor
  BCWeak( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "BCWeak"; }

  virtual void execute();

private: // functions

protected: // configuration

  virtual void initialize()
  {
    BC::initialize();

    face_elem = shared_caches().template get_cache<SFDElement>("face_elem");
    face_elem->options().configure_option("space",solution_field().dict().handle<mesh::Dictionary>());
    inner_cell = shared_caches().template get_cache<SFDElement>();
    inner_cell->options().configure_option("space",solution_field().dict().handle<mesh::Dictionary>());
    face_pt_solution              = shared_caches().template get_cache< SolutionPointField<NEQS,NDIM> >(SFDM::Tags::solution());
    face_pt_solution->options().configure_option("field",solution_field().uri());
  }

  virtual void set_face_entities(const mesh::Entities& face_entities)
  {
    BC::set_face_entities(face_entities);
    face_elem->cache(m_face_entities);
    face_pt_solution->cache(m_face_entities);

    inner_cell_face_data.resize(face_elem->get().sf->nb_nodes());
    for(Uint face_pt=0; face_pt<inner_cell_face_data.size(); ++face_pt)
      inner_cell_face_data[face_pt] = boost::shared_ptr<POINTDATA>(new POINTDATA);

    boundary_face_data.resize(face_elem->get().sf->nb_nodes());
    for(Uint face_pt=0; face_pt<boundary_face_data.size(); ++face_pt)
      boundary_face_data[face_pt] = boost::shared_ptr<POINTDATA>(new POINTDATA);

  }

  virtual void set_face_element(const Uint face_elem_idx)
  {
    BC::set_face_element(face_elem_idx);
    face_elem->set_cache(m_face_elem_idx);
    face_pt_solution->set_cache(m_face_elem_idx);
  }

  virtual void unset_face_element()
  {
    BC::unset_face_element();
    face_elem->get().unlock();
    face_pt_solution->get().unlock();
  }

  virtual void set_inner_cell()
  {
    inner_cell->cache(cell_entities,cell_idx);
  }

  virtual void unset_inner_cell()
  {
    inner_cell->get().unlock();
  }

  virtual void compute_solution(const POINTDATA& inner_cell_data, Eigen::Matrix<Real,NEQS,1>& boundary_face_solution) = 0;

  void compute_face()
  {
    // connectivity
    inner_cell_face_pt_idx.resize(inner_cell->get().sf->face_flx_pts(cell_face_nb).size());
    boundary_face_pt_idx.resize(inner_cell->get().sf->face_flx_pts(cell_face_nb).size());
    inner_cell_face_pt_idx = inner_cell->get().sf->face_flx_pts(cell_face_nb);
    for (Uint face_pt=0; face_pt<inner_cell_face_pt_idx.size(); ++face_pt)
      boundary_face_pt_idx[face_pt] = face_pt;

    // inner cell data
    cf3_assert(inner_cell_face_data[0].get() != inner_cell_face_data[1].get());
    for (Uint face_pt=0; face_pt<inner_cell_face_pt_idx.size(); ++face_pt)
    {
      reconstruct_flx_pt_data(inner_cell->get(),inner_cell_face_pt_idx[face_pt],*inner_cell_face_data[face_pt]);
    }

    // boundary face data
    for (Uint face_pt=0; face_pt<boundary_face_data.size(); ++face_pt)
    {
//        common::TableConstRow<Uint>::type field_index = neighbour_elem->get().space->connectivity()[neighbour_elem->get().idx];
//        std::cout << "convective_term -- " << neighbour_elem->get().entities->uri() << "[" << neighbour_elem->get().idx << "]" << " : face_points = " << field_index[right_flx_pt_idx[face_pt]] << "  ---> " ;

      boundary_face_pt_idx[face_pt] = face_pt;


      mesh::Field::View boundary_face_pt_solution = solution_field().view(face_elem->get().space->connectivity()[face_elem->get().idx]);
      for (Uint v=0; v<NEQS; ++v)
        boundary_face_data[face_pt]->solution[v] = boundary_face_pt_solution[face_pt][v];
      boundary_face_data[face_pt]->coord = face_elem->get().space->get_coordinates(face_elem->get().idx).row(face_pt);
    }


    boost::shared_ptr<POINTDATA> tmp_data;
    Uint tmp_idx;
    for (Uint face_pt=0; face_pt<inner_cell->get().sf->face_flx_pts(cell_face_nb).size(); ++face_pt)
    {
      Uint bdry_face_pt=0;
      bool matched=false;
      for (; bdry_face_pt<inner_cell->get().sf->face_flx_pts(cell_face_nb).size(); ++bdry_face_pt)
      {
//        RealMatrix coords = face_elem->get().space->get_coordinates(face_elem->get().idx);
//        cf3_assert(bdry_face_pt<coords.rows());
//        const RealVector2& coord = coords.row(bdry_face_pt);
        bool m=true;
        for (Uint d=0; d<NDIM; ++d)
        {
          m = m && ( std::abs(boundary_face_data[bdry_face_pt]->coord[d] - inner_cell_face_data[face_pt]->coord[d]) < 20*math::Consts::eps() );
        }
        if ( m )
        {
          matched=true;
          break;
        }
      }
      if (!matched)
      {
        std::cout << face_elem->get().entities->uri() << "["<<face_elem->get().idx << "] boundary_face_data[0]->coord = " << boundary_face_data[0]->coord.transpose() << std::endl;
        std::cout << inner_cell->get().entities->uri() << "["<<inner_cell->get().idx << "] inner_cell_face_data[0]->coord = " << inner_cell_face_data[0]->coord.transpose() << std::endl;
      }
      cf3_assert(matched);

      tmp_data=boundary_face_data[face_pt];
      tmp_idx=boundary_face_pt_idx[face_pt];
      boundary_face_data[face_pt]=boundary_face_data[bdry_face_pt];
      boundary_face_pt_idx[face_pt]=boundary_face_pt_idx[bdry_face_pt];
      boundary_face_data[bdry_face_pt]=tmp_data;
      boundary_face_pt_idx[bdry_face_pt]=tmp_idx;
    }

//    for (Uint face_pt=0; face_pt<inner_cell->get().sf->face_flx_pts(cell_face_nb).size(); ++face_pt)
//    {
//      cf3_assert(inner_cell_face_data[face_pt]->coord == boundary_face_data[face_pt]->coord);
//    }


//    std::cout << "check reconstruction:" << std::endl;
//    for (Uint face_pt=0; face_pt<inner_cell_face_pt_idx.size(); ++face_pt)
//    {
//      std::cout << inner_cell_face_data[face_pt]->solution.transpose() << std::endl;
//    }

  }

  virtual void reconstruct_flx_pt_data(const SFDElement& elem, const Uint flx_pt, POINTDATA& point_data )
  {
    mesh::Field::View sol_pt_solution = solution_field().view(elem.space->connectivity()[elem.idx]);
    elem.reconstruct_solution_space_to_flux_points[flx_pt](sol_pt_solution,point_data.solution);
    elem.reconstruct_geometry_space_to_flux_points[flx_pt](elem.entities->geometry_space().get_coordinates(elem.idx),point_data.coord);
//    std::cout << "reconstruct \n" << elem.entities->get_coordinates(elem.idx) << "   to flx_pt " << flx_pt << "\n" << point_data.coord.transpose() << std::endl;
  }


protected: // fast-access-data (for convenience no "m_" prefix)

  // Data
std::vector<boost::shared_ptr< POINTDATA > > inner_cell_face_data;
std::vector<boost::shared_ptr< POINTDATA > > boundary_face_data;
std::vector<Uint> inner_cell_face_pt_idx;
std::vector<Uint> boundary_face_pt_idx;


  Handle<mesh::Entities const> cell_entities;
  Uint cell_idx;
  Uint cell_face_nb;
  Uint cell_flx_pt;

  // In face solution points ( = cell flux points )
  Handle< CacheT<SolutionPointField<NEQS,NDIM> > > face_pt_solution;

  Handle< CacheT<SFDElement> > inner_cell;
  Handle< CacheT<SFDElement> > face_elem;

}; // end BCWeak

////////////////////////////////////////////////////////////////////////////////

template <typename POINTDATA>
inline BCWeak<POINTDATA>::BCWeak( const std::string& name )
  : BC(name)
{
  properties()["brief"] = std::string("Spectral Finite Difference BC");
  properties()["description"] = std::string("");
}

/////////////////////////////////////////////////////////////////////////////

template <typename POINTDATA>
inline void BCWeak<POINTDATA>::execute()
{
  find_inner_cell(m_face_entities,m_face_elem_idx,cell_entities,cell_idx,cell_face_nb);
  set_inner_cell();
  compute_face();
  for(Uint face_pt=0; face_pt<boundary_face_pt_idx.size(); ++face_pt)
  { 
    Eigen::Matrix<Real,NEQS,1> face_sol;
    cell_flx_pt = inner_cell_face_pt_idx[face_pt];
    compute_solution(*inner_cell_face_data[face_pt],face_sol);
//    common::TableConstRow<Uint>::type field_index = face_elem->get().space->connectivity()[m_face_elem_idx];

//    std::cout << "boundary -- " << face_elem->get().entities->uri() << "[" << face_elem->get().idx << "]" << " : face_points = " << field_index[boundary_face_pt_idx[face_pt]] << "  ---> " ;
    for (Uint v=0; v<NEQS; ++v)
    {
      face_pt_solution->get()[boundary_face_pt_idx[face_pt]][v] = face_sol[v];
//      std::cout << solution_field()[field_index[boundary_face_pt_idx[face_pt]]][v] << " ";
//      std::cout << face_sol[v] << " ";
    }
//    std::cout << "     inner flx_pt idx = " << inner_cell_face_pt_idx[face_pt];
//    std::cout << std::endl;

  }
  unset_inner_cell();
}

////////////////////////////////////////////////////////////////////////////////

class SFDM_API BCNull : public BCWeak< BCPointData<4u,2u> >
{
public:
  static std::string type_name() { return "BCNull"; }
  BCNull(const std::string& name) : BCWeak< BCPointData<4u,2u> >(name)
  {
  }
  virtual ~BCNull() {}

  virtual void compute_solution(const BCPointData<4u,2u>& inner_cell_data, Eigen::Matrix<Real,NEQS,1>& boundary_face_pt_data)
  {
    boundary_face_pt_data = inner_cell_data.solution;
  }
};


} // SFDM
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_SFDM_BCWeak_hpp
