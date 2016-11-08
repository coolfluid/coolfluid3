// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Builder.hpp"

#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "mesh/LagrangeP0/Line.hpp"
#include "mesh/LagrangeP0/Quad.hpp"
#include "mesh/LagrangeP0/Triag.hpp"
#include "mesh/LagrangeP1/ElementTypes.hpp"

#include "Proto/Expression.hpp"

#include "FaceNormals.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
namespace actions {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < FaceNormals, common::Action, LibActions > FaceNormals_Builder;

///////////////////////////////////////////////////////////////////////////////////////

using namespace Proto;

FaceNormals::FaceNormals ( const std::string& name ) :
  ProtoAction(name)
{
  FieldVariable<0, VectorField> n("Normal", "normals", mesh::LagrangeP0::LibLagrangeP0::library_namespace());

  typedef boost::mpl::vector<
    mesh::LagrangeP0::Line,
    mesh::LagrangeP0::Quad,
    mesh::LagrangeP0::Triag,
    mesh::LagrangeP1::Line2D,
    mesh::LagrangeP1::Quad3D,
    mesh::LagrangeP1::Triag3D
  > element_types;
  
  set_expression(elements_expression(element_types(),
    n = normal(gauss_points_1)
  ));
}


////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////////

