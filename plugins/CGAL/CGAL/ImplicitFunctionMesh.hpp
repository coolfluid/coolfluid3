// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CGAL_ImplicitFunctionMesh_hpp
#define CF_Mesh_CGAL_ImplicitFunctionMesh_hpp

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

#include "mesh/Mesh.hpp"

#include "CGAL/LibCGAL.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace CGAL {

////////////////////////////////////////////////////////////////////////////////

/// CGAL kernel and related types
typedef ::CGAL::Exact_predicates_inexact_constructions_kernel KernelT;
typedef KernelT::FT CGReal; // Floating point number type (exact in CGAL)
typedef KernelT::Point_3 Point; // 3D point coordinates

////////////////////////////////////////////////////////////////////////////////

/// Base class for implicit functions defining the mesh bounds.
/// This is a virtual functor on purpose, since making it template would expose
/// CGAL too much outside of the library, and delegate the responsibility for linking
/// to the CGAL libs to anything that uses these functions.
struct CGAL_API ImplicitFunction {
  /// Construct using the given radius and center for the domain bounding sphere
  ImplicitFunction(const Real X, const Real Y, const Real Z, const Real R) : x(X), y(Y), z(Z), radius(R) {}

  /// virtual destructor
  virtual ~ImplicitFunction() {}

  /// Negative values for the given point coordinates p are inside the domain
  virtual CGReal operator()(const Point& p) const = 0;

  /// Center of the sphere bounding the domain. Functor must evaluate to a negative value here
  Real x,y,z;

  /// Radius of the sphere bounding the domain.
  Real radius;
};

////////////////////////////////////////////////////////////////////////////////

/// Sphere around the origin with radius r
struct CGAL_API SphereFunction : public ImplicitFunction {
  /// constructor
  SphereFunction(const Real r) : ImplicitFunction(0., 0, 0., r*1.1), m_radius(r) {}
  /// virtual destructor
  virtual ~SphereFunction() {}
  virtual CGReal operator()(const Point& p) const;
private:
  const Real m_radius;
};

////////////////////////////////////////////////////////////////////////////////

/// Parameters for mesh generation, as defined in the CGAL manual
struct CGAL_API MeshParameters {
/// Default parameters based on the sphere example
MeshParameters() : facet_angle(30),
                   facet_size(0.1),
                   facet_distance(0.025),
                   cell_radius_edge(2),
                   cell_size(0.1)
{}
  Real facet_angle;
  Real facet_size;
  Real facet_distance;
  Real cell_radius_edge;
  Real cell_size;
};

////////////////////////////////////////////////////////////////////////////////

/// Using the given implicit function delimiting a domain,
CGAL_API void create_mesh(const ImplicitFunction& function, mesh::Mesh& mesh, const MeshParameters parameters=MeshParameters());

////////////////////////////////////////////////////////////////////////////////

} //  CGAL
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif /* CF_Mesh_CGAL_ImplicitFunctionMesh_hpp */
