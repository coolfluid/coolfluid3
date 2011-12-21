// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_Operations_hpp
#define cf3_SFDM_Operations_hpp

////////////////////////////////////////////////////////////////////////////////

#include "math/MatrixTypes.hpp"

#include "mesh/Field.hpp"
#include "mesh/Space.hpp"
#include "mesh/ShapeFunction.hpp"

#include "SFDM/ElementCaching.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {

////////////////////////////////////////////////////////////////////////////////

struct GeometryElement : ElementCache
{
  typedef CacheT<GeometryElement> cache_type;
  static std::string type_name() { return "GeometryElement"; }
  GeometryElement (const std::string& name=type_name()) : ElementCache(name) {}

  static void add_options(Cache& cache) {}

private:

  virtual void compute_fixed_data()
  {
    cf3_assert(entities);
    space = entities->geometry_space().handle<mesh::Space>();
    sf = space->shape_function().handle<mesh::ShapeFunction>();
    entities->allocate_coordinates(nodes);
  }

  virtual void compute_variable_data()
  {
    entities->put_coordinates(nodes,idx);
  }

public:
  // intrinsic state (not supposed to change)
  Handle< mesh::Space const         > space;
  Handle< mesh::ShapeFunction const > sf;

  // extrinsic state (changed for every computation)
  RealMatrix nodes;
};

////////////////////////////////////////////////////////////////////////////////

struct SFDElement : ElementCache
{
  typedef CacheT<SFDElement> cache_type;
  static std::string type_name() { return "SFDElement"; }
  SFDElement (const std::string& name=type_name()) : ElementCache(name) {}

  static void add_options(Cache& cache)
  {
    cache.options().add_option("space",std::string("sfd_space"));
  }

private:
  virtual void compute_fixed_data()
  {
    //cf3_assert(cache->options().check("space"));
    std::string space_id = cache->options().option("space").value<std::string>();
    space = entities->space(space_id).handle<mesh::Space>();
    sf = space->shape_function().handle<SFDM::ShapeFunction>();
  }

  virtual void compute_variable_data() {}

public:
  // intrinsic state (not supposed to change)
  Handle< mesh::Space const         > space;
  Handle< SFDM::ShapeFunction const > sf;
};

////////////////////////////////////////////////////////////////////////////////

struct FluxPointDivergence : ElementCache
{
  typedef CacheT<FluxPointDivergence> cache_type;
  static std::string type_name() { return "FluxPointDivergence"; }
  FluxPointDivergence (const std::string& name=type_name()) : ElementCache(name) {}

  static void add_options(Cache& cache)
  {
    cache.options().add_option("space",std::string("sfd_space"));
  }

private:
  virtual void compute_fixed_data()
  {
    std::string space_id = options().option("space").value<std::string>();
    space = entities->space(space_id).handle<mesh::Space>();
    sf = space->shape_function().handle<SFDM::ShapeFunction>();
    compute.build_coefficients(sf);
  }

  virtual void compute_variable_data() {}

public:
  // intrinsic state (not supposed to change)
  Handle< mesh::Space const         > space;
  Handle< SFDM::ShapeFunction const > sf;
  DivergenceReconstructFromFluxPoints compute;
};

////////////////////////////////////////////////////////////////////////////////

struct FluxPointReconstruct : ElementCache
{
  typedef CacheT<FluxPointReconstruct> cache_type;
  static std::string type_name() { return "FluxPointReconstruct"; }
  FluxPointReconstruct (const std::string& name=type_name()) : ElementCache(name) {}

  static void add_options(Cache& cache)
  {
    cache.options().add_option("space",std::string("sfd_space"));
  }

private:
  virtual void compute_fixed_data()
  {
    std::string space_id = options().option("space").value<std::string>();
    space = entities->space(space_id).handle<mesh::Space>();
    sf = space->shape_function().handle<SFDM::ShapeFunction>();
    compute.build_coefficients(sf);
  }

  virtual void compute_variable_data() {}

public:
  // intrinsic state (not supposed to change)
  Handle< mesh::Space const         > space;
  Handle< SFDM::ShapeFunction const > sf;
  std::string space_id;
  ReconstructFromFluxPoints compute;
};

////////////////////////////////////////////////////////////////////////////////

template <Uint NEQS,Uint NDIM>
struct PlaneJacobianNormal : ElementCache
{
  typedef CacheT<PlaneJacobianNormal<NEQS,NDIM> > cache_type;
  static std::string type_name() { return "PlaneJacobianNormal"; }
  PlaneJacobianNormal (const std::string& name=type_name()) : ElementCache(name) {}

  static void add_options(Cache& cache)
  {
    cache.options().add_option("space",std::string("sfd_space"));
  }

private:
  virtual void compute_fixed_data()
  {
    geo.configure(entities);
    std::string space_id = options().option("space").template value<std::string>();
    space = entities->space(space_id).handle<mesh::Space>();
    sf = space->shape_function().handle<SFDM::ShapeFunction>();

    plane_jacobian_normal.resize(sf->nb_flx_pts(),RealVector(entities->element_type().dimension()));
  }

  virtual void compute_variable_data()
  {
    geo.compute_element(idx); // computes geo.nodes, for use of plane_jacobian normals

    // compute plane-jacobian normals
    const RealMatrix& flx_pts = sf->flx_pts();
    for (Uint f=0; f<sf->nb_flx_pts(); ++f)
    {
      cf3_assert(sf->flx_pt_dirs(f).size() == 1);
      CoordRef dir = static_cast<CoordRef>(sf->flx_pt_dirs(f)[0]);
      /// @todo remove copy
      plane_jacobian_normal[f] = geo.entities->element_type().
          plane_jacobian_normal(flx_pts.row(f),geo.nodes,dir);
    }
  }
public:
  // intrinsic state (not supposed to change)
  Handle< mesh::Space const         > space;
  Handle< SFDM::ShapeFunction const > sf;
  GeometryElement geo;
  typedef Eigen::Matrix<Real, NDIM, 1> coord_t;

  // extrinsic state
  std::vector<coord_t>      plane_jacobian_normal;
};

////////////////////////////////////////////////////////////////////////////////

//template <Uint NEQS,Uint NDIM>
//struct Coordinates : ElementCache
//{
//  typedef CacheT<Coordinates<NEQS,NDIM> > cache_type;
//  static std::string type_name() { return "Coordinates"; }
//  Coordinates (const std::string& name=type_name()) : ElementCache(name) {}

//private:
//  virtual void compute_fixed_data()
//  {
//    geo.configure(entities);
//    std::string space_id = cache->options()->option("space").value<std::string>();
//    space = entities->space(space_id).handle<mesh::Space>();
//    sf = space->shape_function().handle<SFDM::ShapeFunction>();

//    reconstruct_to_flux_points.build_coefficients(geo.sf,sf);
//    coord_in_flx_pts.resize(sf->nb_flx_pts());
//  }

//  virtual void compute_variable_data()
//  {
//    geo.compute_element(idx); // computes geo.nodes, for use of plane_jacobian normals

//    // reconstruct the nodes
//    reconstruct_to_flux_points(geo.nodes,coord_in_flx_pts);
//  }
//public:
//  // intrinsic state (not supposed to change)
//  Handle< mesh::Space const         > space;
//  Handle< SFDM::ShapeFunction const > sf;
//  GeometryElementCache geo;

//  ReconstructToFluxPoints reconstruct_to_flux_points;
//  typedef Eigen::Matrix<Real, NDIM, 1> coord_t;

//  // extrinsic state
//  std::vector<coord_t>      coord_in_flx_pts;
//};

////////////////////////////////////////////////////////////////////////////////

template <Uint NVAR,Uint NDIM>
struct SFDField : ElementCache
{
  typedef CacheT< SFDField<NVAR,NDIM> > cache_type;
  static std::string type_name() { return "SFDField"; }
  SFDField (const std::string& name=type_name()) : ElementCache(name) {}

  static void add_options(Cache& cache)
  {
    cache.options().add_option("field",common::URI());
  }

private:
  virtual void compute_fixed_data()
  {
    field = cache->access_component(options().option("field").template value<common::URI>())->template handle<mesh::Field>();
    space = field->field_group().space(*entities).template handle<mesh::Space>();
    sf = space->shape_function().handle<SFDM::ShapeFunction>();
    reconstruct_to_flux_points.build_coefficients(sf);
    field_in_flx_pts.resize(sf->nb_flx_pts());
  }

  virtual void compute_variable_data()
  {
    mesh::Field::View field_in_sol_pts = field->view(space->indexes_for_element(idx));
    reconstruct_to_flux_points(field_in_sol_pts,field_in_flx_pts);
  }

public:
  // intrinsic state (not supposed to change)
  Handle< mesh::Space const         > space;
  Handle< SFDM::ShapeFunction const > sf;
  Handle< mesh::Field > field;
  ReconstructToFluxPoints reconstruct_to_flux_points;
  typedef Eigen::Matrix<Real, NVAR, 1> field_t;

  // extrinsic state
  std::vector<field_t>      field_in_flx_pts;
};

////////////////////////////////////////////////////////////////////////////////

//template <Uint NVAR,Uint NDIM>
//struct SFDGradField : ElementCache
//{
//  typedef CacheT< SFDGradField<NVAR,NDIM> > cache_type;
//  static std::string type_name() { return "SFDGradField"; }
//  SFDGradField (const std::string& name=type_name()) : ElementCache(name) {}

//  void set_field(Handle<Field> sfdfield)
//  {
//    field = sfdfield;
//  }

//private:
//  virtual void compute_fixed_data()
//  {
//    std::string space_id = cache->options()->option("space").value<std::string>();
//    space = entities->space(space_id).handle<mesh::Space>();
//    sf = space->shape_function().handle<SFDM::ShapeFunction>();
//    gradient_reconstruct_to_flux_points.build_coefficients(sf);
//    grad_field_in_flx_pts.resize(sf->nb_flx_pts());
//  }

//  virtual void compute_variable_data()
//  {
//    Field::View grad_field_in_sol_pts = field->view(space->indexes_for_element(idx));
//    gradient_reconstruct_to_flux_points(grad_field_in_sol_pts,grad_field_in_flx_pts);
//  }

//public:
//  // intrinsic state (not supposed to change)
//  Handle< mesh::Space const         > space;
//  Handle< SFDM::ShapeFunction const > sf;
//  Handle< Field > field;
//  GradientReconstructToFluxPoints gradient_reconstruct_to_flux_points;
//  typedef Eigen::Matrix<Real, NVAR, NDIM> grad_field_t;

//  // extrinsic state
//  std::vector< grad_field_t > grad_field_in_flx_pts;
//};

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_SFDM_Operations_hpp
