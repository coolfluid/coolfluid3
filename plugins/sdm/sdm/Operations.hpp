// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_Operations_hpp
#define cf3_sdm_Operations_hpp

////////////////////////////////////////////////////////////////////////////////

#include "math/MatrixTypes.hpp"

#include "mesh/Field.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/ShapeFunction.hpp"

#include "sdm/ElementCaching.hpp"
#include "sdm/Reconstructions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {

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
    entities->geometry_space().allocate_coordinates(nodes);
  }

  virtual void compute_variable_data()
  {
    entities->geometry_space().put_coordinates(nodes,idx);
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
    cache.options().add("space",Handle<mesh::Dictionary>()).description("path to Dictionary");
  }

private:
  virtual void compute_fixed_data()
  {
    //cf3_assert(cache->options().check("space"));
    cf3_assert(entities);
    space = options().value< Handle<mesh::Dictionary> >("space")->space(entities);
    cf3_assert(space);
    sf = space->shape_function().handle<sdm::ShapeFunction>();
    cf3_assert(sf);
    reconstruct_from_geometry_space_to_flux_points.build_coefficients(entities->element_type().shape_function().handle<mesh::ShapeFunction>(),sf);
    reconstruct_from_solution_space_to_flux_points.build_coefficients(sf,sf);
    reconstruct_from_flux_points_to_solution_space.build_coefficients(sf);
    reconstruct_divergence_from_flux_points_to_solution_space.build_coefficients(sf);
  }

  virtual void compute_variable_data() {}

public:
  // intrinsic state (not supposed to change)
  Handle< mesh::Space const         > space;
  Handle< sdm::ShapeFunction const > sf;

  ReconstructToFluxPoints              reconstruct_from_solution_space_to_flux_points;
  ReconstructToFluxPoints              reconstruct_from_geometry_space_to_flux_points;
  ReconstructFromFluxPoints            reconstruct_from_flux_points_to_solution_space;
  DivergenceReconstructFromFluxPoints  reconstruct_divergence_from_flux_points_to_solution_space;

};

////////////////////////////////////////////////////////////////////////////////

template <Uint NDIM>
struct FluxPointPlaneJacobianNormal : ElementCache
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  typedef CacheT<FluxPointPlaneJacobianNormal<NDIM> > cache_type;
  typedef Eigen::Matrix<Real, NDIM, 1> coord_t;

  static std::string type_name() { return "FluxPointPlaneJacobianNormal"; }
  FluxPointPlaneJacobianNormal (const std::string& name=type_name()) : ElementCache(name) {}

  static void add_options(Cache& cache)
  {
    cache.options().add("space",Handle<mesh::Dictionary>()).description("path to Dictionary");
  }

private:
  virtual void compute_fixed_data()
  {
    geo.configure(entities);
    space = options().option("space").template value< Handle<mesh::Dictionary> >()->space(entities);
    sf = space->shape_function().handle<sdm::ShapeFunction>();

    plane_jacobian_normal.resize(sf->nb_flx_pts());
    plane_jacobian.resize(sf->nb_flx_pts());
    plane_unit_normal.resize(sf->nb_flx_pts());
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
      plane_jacobian[f] = plane_jacobian_normal[f].norm();
      plane_unit_normal[f] = plane_jacobian_normal[f]/plane_jacobian[f];
    }
  }

public:

  coord_t& operator[] (const Uint flx_pt)             { return plane_jacobian_normal[flx_pt]; }
  const coord_t& operator[] (const Uint flx_pt) const { return plane_jacobian_normal[flx_pt]; }

public:
  // intrinsic state (not supposed to change)
  Handle< mesh::Space const         > space;
  Handle< sdm::ShapeFunction const > sf;
  GeometryElement geo;

  // extrinsic state
  std::vector<coord_t, Eigen::aligned_allocator<coord_t> >      plane_jacobian_normal;
  std::vector<Real>         plane_jacobian;
  std::vector<coord_t, Eigen::aligned_allocator<coord_t> >      plane_unit_normal;
};

////////////////////////////////////////////////////////////////////////////////

template <Uint NDIM>
struct FluxPointCoordinates : ElementCache
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  typedef Eigen::Matrix<Real, NDIM, 1> coord_t;

  typedef CacheT<FluxPointCoordinates<NDIM> > cache_type;
  static std::string type_name() { return "FluxPointCoordinates"; }
  FluxPointCoordinates (const std::string& name=type_name()) : ElementCache(name) {}

  static void add_options(Cache& cache)
  {
    cache.options().add("space",Handle<mesh::Dictionary>()).description("path to Dictionary");
  }

private:
  virtual void compute_fixed_data()
  {
    geo.configure(entities);
    space = options().option("space").template value< Handle<mesh::Dictionary> >()->space(entities);
    sf = space->shape_function().handle<sdm::ShapeFunction>();

    reconstruct_to_flux_points.build_coefficients(geo.sf,sf);
    coord_in_flx_pts.resize(sf->nb_flx_pts());
  }

  virtual void compute_variable_data()
  {
    geo.compute_element(idx); // computes geo.nodes

    // reconstruct the nodes
    reconstruct_to_flux_points(geo.nodes,coord_in_flx_pts);
  }

public:

  coord_t& operator[] (const Uint flx_pt)             { return coord_in_flx_pts[flx_pt]; }
  const coord_t& operator[] (const Uint flx_pt) const { return coord_in_flx_pts[flx_pt]; }

public:
  // intrinsic state (not supposed to change)
  Handle< mesh::Space const         > space;
  Handle< sdm::ShapeFunction const > sf;
  GeometryElement geo;

  ReconstructToFluxPoints reconstruct_to_flux_points;

  // extrinsic state
  std::vector<coord_t>      coord_in_flx_pts;
};

struct FluxPointCoordinatesDyn : ElementCache
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  typedef RealVector coord_t;

  typedef CacheT<FluxPointCoordinatesDyn > cache_type;
  static std::string type_name() { return "FluxPointCoordinatesDyn"; }
  FluxPointCoordinatesDyn (const std::string& name=type_name()) : ElementCache(name) {}

  static void add_options(Cache& cache)
  {
    cache.options().add("space",Handle<mesh::Dictionary>()).description("path to Dictionary");
  }

private:
  virtual void compute_fixed_data()
  {
    geo.configure(entities);
    space = options().value< Handle<mesh::Dictionary> >("space")->space(entities);
    sf = space->shape_function().handle<sdm::ShapeFunction>();

    reconstruct_to_flux_points.build_coefficients(geo.sf,sf);
    coord_in_flx_pts.resize(sf->nb_flx_pts(),RealVector(entities->element_type().dimension()));
  }

  virtual void compute_variable_data()
  {
    geo.compute_element(idx); // computes geo.nodes

    // reconstruct the nodes
    reconstruct_to_flux_points(geo.nodes,coord_in_flx_pts);
  }

public:

  coord_t& operator[] (const Uint flx_pt)             { return coord_in_flx_pts[flx_pt]; }
  const coord_t& operator[] (const Uint flx_pt) const { return coord_in_flx_pts[flx_pt]; }

public:
  // intrinsic state (not supposed to change)
  Handle< mesh::Space const         > space;
  Handle< sdm::ShapeFunction const > sf;
  GeometryElement geo;

  ReconstructToFluxPoints reconstruct_to_flux_points;

  // extrinsic state
  std::vector<coord_t>      coord_in_flx_pts;
};
////////////////////////////////////////////////////////////////////////////////

template <Uint NDIM>
struct SolutionPointCoordinates : ElementCache
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  typedef Eigen::Matrix<Real, NDIM, 1> coord_t;

  typedef CacheT<SolutionPointCoordinates<NDIM> > cache_type;
  static std::string type_name() { return "SolutionPointCoordinates"; }
  SolutionPointCoordinates (const std::string& name=type_name()) : ElementCache(name) {}

  static void add_options(Cache& cache)
  {
    cache.options().add("space",Handle<mesh::Dictionary>()).description("path to Dictionary");
  }

private:
  virtual void compute_fixed_data()
  {
    geo.configure(entities);
    space = options().option("space").template value< Handle<mesh::Dictionary> >()->space(entities);
    sf = space->shape_function().handle<sdm::ShapeFunction>();

    reconstruct_to_solution_points.build_coefficients(geo.sf,sf);
    coord_in_sol_pts.resize(sf->nb_sol_pts());
  }

  virtual void compute_variable_data()
  {
    geo.compute_element(idx); // computes geo.nodes

    // reconstruct the nodes
    reconstruct_to_solution_points(geo.nodes,coord_in_sol_pts);
  }

public:

  coord_t& operator[] (const Uint sol_pt)             { return coord_in_sol_pts[sol_pt]; }
  const coord_t& operator[] (const Uint sol_pt) const { return coord_in_sol_pts[sol_pt]; }

public:
  // intrinsic state (not supposed to change)
  Handle< mesh::Space const         > space;
  Handle< sdm::ShapeFunction const > sf;
  GeometryElement geo;

  mesh::Reconstruct reconstruct_to_solution_points;

  // extrinsic state
  std::vector<coord_t>      coord_in_sol_pts;
};

////////////////////////////////////////////////////////////////////////////////

struct SolutionPointFieldDyn : ElementCache
{
  typedef CacheT< SolutionPointFieldDyn > cache_type;
  static std::string type_name() { return "SolutionPointFieldDyn"; }
  SolutionPointFieldDyn (const std::string& name=type_name()) : ElementCache(name), field_in_sol_pts(nullptr) {}

  ~SolutionPointFieldDyn()
  {
    if (is_not_null(field_in_sol_pts))
      delete(field_in_sol_pts);
  }

  static void add_options(Cache& cache)
  {
    cache.options().add("field",common::URI());
  }

private:
  virtual void compute_fixed_data()
  {
    field = cache->access_component(options().value<common::URI>("field"))->handle<mesh::Field>();
    space = field->dict().space(*entities).handle<mesh::Space>();
    sf = space->shape_function().handle<sdm::ShapeFunction>();
  }

  virtual void compute_variable_data()
  {
    if (is_not_null(field_in_sol_pts))
      delete(field_in_sol_pts);

    field_in_sol_pts = new mesh::Field::View(field->view(space->connectivity()[idx]));
  }

public:
  typedef boost::detail::multi_array::sub_array<Real, 1> field_t;
  typedef boost::detail::multi_array::const_sub_array<Real, 1> const_field_t;

  field_t operator[] (const Uint sol_pt)             { cf3_assert(is_not_null(field_in_sol_pts)); return (*field_in_sol_pts)[sol_pt]; }
  const field_t operator[] (const Uint sol_pt) const { cf3_assert(is_not_null(field_in_sol_pts)); return (*field_in_sol_pts)[sol_pt]; }

public:
  // intrinsic state (not supposed to change)
  Handle< mesh::Space const         > space;
  Handle< sdm::ShapeFunction const > sf;
  Handle< mesh::Field > field;

  // extrinsic state
  mesh::Field::View*     field_in_sol_pts;
};


template <Uint NVAR,Uint NDIM>
struct SolutionPointField : ElementCache
{
  typedef CacheT< SolutionPointField<NVAR,NDIM> > cache_type;
  static std::string type_name() { return "SolutionPointField"; }
  SolutionPointField (const std::string& name=type_name()) : ElementCache(name), field_in_sol_pts(nullptr) {}

  ~SolutionPointField()
  {
    if (is_not_null(field_in_sol_pts))
      delete(field_in_sol_pts);
  }

  static void add_options(Cache& cache)
  {
    cache.options().add("field",common::URI());
  }

private:
  virtual void compute_fixed_data()
  {
    field = cache->access_component(options().option("field").template value<common::URI>())->template handle<mesh::Field>();
    space = field->dict().space(*entities).template handle<mesh::Space>();
    sf = space->shape_function().handle<sdm::ShapeFunction>();
  }

  virtual void compute_variable_data()
  {
    if (is_not_null(field_in_sol_pts))
      delete(field_in_sol_pts);

    field_in_sol_pts = new mesh::Field::View(field->view(space->connectivity()[idx]));
  }

public:
  typedef boost::detail::multi_array::sub_array<Real, 1> field_t;
  typedef boost::detail::multi_array::const_sub_array<Real, 1> const_field_t;

  field_t operator[] (const Uint sol_pt)             { cf3_assert(is_not_null(field_in_sol_pts)); return (*field_in_sol_pts)[sol_pt]; }
  const field_t operator[] (const Uint sol_pt) const { cf3_assert(is_not_null(field_in_sol_pts)); return (*field_in_sol_pts)[sol_pt]; }

public:
  // intrinsic state (not supposed to change)
  Handle< mesh::Space const         > space;
  Handle< sdm::ShapeFunction const > sf;
  Handle< mesh::Field > field;

  // extrinsic state
  mesh::Field::View*     field_in_sol_pts;
};

////////////////////////////////////////////////////////////////////////////////

struct FluxPointFieldDyn : ElementCache
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  typedef CacheT< FluxPointFieldDyn > cache_type;
  static std::string type_name() { return "FluxPointFieldDyn"; }
  FluxPointFieldDyn (const std::string& name=type_name()) : ElementCache(name) {}

  static void add_options(Cache& cache)
  {
    cache.options().add("field",common::URI());
  }

private:
  virtual void compute_fixed_data()
  {
    field = cache->access_component(options().value<common::URI>("field"))->handle<mesh::Field>();
    space = field->dict().space(*entities).handle<mesh::Space>();
    sf = space->shape_function().handle<sdm::ShapeFunction>();
    reconstruct_to_flux_points.build_coefficients(sf);
    field_in_flx_pts.resize(sf->nb_flx_pts(),RealVector(field->row_size()));
  }

  virtual void compute_variable_data()
  {
    mesh::Field::View field_in_sol_pts = field->view(space->connectivity()[idx]);
    reconstruct_to_flux_points(field_in_sol_pts,field_in_flx_pts);
  }

public:
  typedef RealVector field_t;

  field_t& operator[] (const Uint flx_pt)             { return field_in_flx_pts[flx_pt]; }
  const field_t& operator[] (const Uint flx_pt) const { return field_in_flx_pts[flx_pt]; }

public:
  // intrinsic state (not supposed to change)
  Handle< mesh::Space const         > space;
  Handle< sdm::ShapeFunction const > sf;
  Handle< mesh::Field > field;
  ReconstructToFluxPoints reconstruct_to_flux_points;

  // extrinsic state
  std::vector<field_t>      field_in_flx_pts;
};

template <Uint NVAR,Uint NDIM>
struct FluxPointField : ElementCache
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  typedef CacheT< FluxPointField<NVAR,NDIM> > cache_type;
  static std::string type_name() { return "FluxPointField<"+common::to_str(NVAR)+","+common::to_str(NDIM)+">"; }
  FluxPointField (const std::string& name=type_name()) : ElementCache(name) {}

  static void add_options(Cache& cache)
  {
    cache.options().add("field",common::URI());
  }

private:
  virtual void compute_fixed_data()
  {
    field = cache->access_component(options().option("field").template value<common::URI>())->template handle<mesh::Field>();
    space = field->dict().space(*entities).template handle<mesh::Space>();
    sf = space->shape_function().handle<sdm::ShapeFunction>();
    reconstruct_to_flux_points.build_coefficients(sf);
    field_in_flx_pts.resize(sf->nb_flx_pts());
  }

  virtual void compute_variable_data()
  {
    mesh::Field::View field_in_sol_pts = field->view(space->connectivity()[idx]);
    reconstruct_to_flux_points(field_in_sol_pts,field_in_flx_pts);
  }

public:
  typedef Eigen::Matrix<Real, NVAR, 1> field_t;

  field_t& operator[] (const Uint flx_pt)             { return field_in_flx_pts[flx_pt]; }
  const field_t& operator[] (const Uint flx_pt) const { return field_in_flx_pts[flx_pt]; }

public:
  // intrinsic state (not supposed to change)
  Handle< mesh::Space const         > space;
  Handle< sdm::ShapeFunction const > sf;
  Handle< mesh::Field > field;
  ReconstructToFluxPoints reconstruct_to_flux_points;

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

//  void set_field(Handle<Field> FluxPointField)
//  {
//    field = FluxPointField;
//  }

//private:
//  virtual void compute_fixed_data()
//  {
//    space = options().value< Handle<mesh::Dictionary> >("space")->space(entities);
//    sf = space->shape_function().handle<sdm::ShapeFunction>();
//    gradient_reconstruct_to_flux_points.build_coefficients(sf);
//    grad_field_in_flx_pts.resize(sf->nb_flx_pts());
//  }

//  virtual void compute_variable_data()
//  {
//    Field::View grad_field_in_sol_pts = field->view(space->connectivity()[idx]);
//    gradient_reconstruct_to_flux_points(grad_field_in_sol_pts,grad_field_in_flx_pts);
//  }

//public:
//  // intrinsic state (not supposed to change)
//  Handle< mesh::Space const         > space;
//  Handle< sdm::ShapeFunction const > sf;
//  Handle< Field > field;
//  GradientReconstructToFluxPoints gradient_reconstruct_to_flux_points;
//  typedef Eigen::Matrix<Real, NVAR, NDIM> grad_field_t;

//  // extrinsic state
//  std::vector< grad_field_t > grad_field_in_flx_pts;
//};

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_Operations_hpp
