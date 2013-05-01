// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string.hpp>

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/StringConversion.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"

#include "math/VariablesDescriptor.hpp"
#include "math/VectorialFunction.hpp"

#include "mesh/AInterpolator.hpp"
#include "mesh/LoadMesh.hpp"
#include "mesh/Field.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Space.hpp"
#include "mesh/Entities.hpp"
#include "mesh/Connectivity.hpp"

#include "mesh/actions/ComputeFieldGradient.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

  using namespace common;
  using namespace math;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ComputeFieldGradient, MeshTransformer, mesh::actions::LibActions> ComputeFieldGradient_Builder;

//////////////////////////////////////////////////////////////////////////////

ComputeFieldGradient::ComputeFieldGradient( const std::string& name )
: MeshTransformer(name)
{
  properties()["brief"] = std::string("ComputeFieldGradient mesh");
  std::string desc;
  desc = "  Usage: ComputeFieldGradient \n\n";

  properties()["description"] = desc;

  options().add("field",m_field).link_to(&m_field)
      .description("Field to take gradient of")
      .mark_basic();

  options().add("field_gradient",m_field_gradient).link_to(&m_field_gradient)
      .description("Output: gradient of option \"field\"")
      .mark_basic();

  options().add("normal",m_normal).link_to(&m_normal);
}

/////////////////////////////////////////////////////////////////////////////

void ComputeFieldGradient::execute()
{
  // Check correct configuration
  if (is_null(m_field)) throw SetupError(FromHere(), "Option field not set in "+uri().string());
  if (is_null(m_field_gradient)) throw SetupError(FromHere(), "Option field_gradient not set in "+uri().string());

  const Uint ndim = m_field->coordinates().row_size();

  if (m_field_gradient->row_size() != ndim*m_field->row_size())
  {
    throw SetupError(FromHere(), "Field "+m_field_gradient->uri().string()+" must have row-size of "+to_str(ndim*m_field->row_size())+". Currently it is "+to_str(m_field_gradient->row_size()));
  }

  RealMatrix n(ndim,ndim);

  if (m_normal.size() == 0)
  {
    n.setZero();
    for (Uint d=0; d<ndim; ++d)
    {
      n(d,d) = 1.;
    }
  }
  else
  {
    if (ndim == 1)
    {
      n << m_normal[XX];
    }
    else if (ndim == 2)
    {
      n <<  // rows are n, s
           m_normal[XX], m_normal[YY],
           m_normal[YY], -m_normal[XX];
    }
    else if (ndim == 3)
    {
      throw NotImplemented(FromHere(), "3d normal not implemented yet");
    }
  }
  // Dereference the handles
  const Field& field = *m_field;
  Field& grad = *m_field_gradient;

  std::vector<Uint> shared_nodes;
  if (grad.continuous())
  {
    shared_nodes.resize(grad.size(),0);
    grad = 0.;
  }

  boost_foreach(const Handle<Space>& grad_space_handle, grad.spaces())
  {
    Space& grad_space = *grad_space_handle;

    if (grad_space.shape_function().dimensionality() == ndim) // if volume element
    {
      if (field.dict().defined_for_entities( grad_space.support().handle<Entities>() ) == false )
        throw SetupError(FromHere(), "Field "+field.uri().string()+" is not defined for elements "+grad_space.support().uri().string());
      Space& field_space = grad_space.support().space(field.dict());


      // Compute interpolation matrices to compute the gradient of
      // field_space in coordinates of grad_space
      std::vector<RealMatrix> gradient_matrix_per_point (grad_space.shape_function().nb_nodes());
      for (Uint grad_pt=0; grad_pt<gradient_matrix_per_point.size(); ++grad_pt)
      {
        gradient_matrix_per_point[grad_pt].resize(ndim,field_space.shape_function().nb_nodes());
        field_space.shape_function().compute_gradient(grad_space.shape_function().local_coordinates().row(grad_pt),
                                                      gradient_matrix_per_point[grad_pt]);
      }

      RealMatrix field_element_values( field_space.shape_function().nb_nodes() , field.row_size() );
      RealMatrix grad_values (ndim,field.row_size());

      RealMatrix jacobian(field_space.shape_function().dimensionality(),ndim);
      RealMatrix cell_coords;
      Entities& entities = field_space.support();
      entities.geometry_space().allocate_coordinates(cell_coords);
      // Compute the actual gradients for each element
      for (Uint e=0; e<grad_space.size(); ++e)
      {
        entities.geometry_space().put_coordinates(cell_coords,e);

        // Assemble field values in a matrix
        for (Uint n=0; n<field_space.shape_function().nb_nodes(); ++n)
        {
          Uint p = field_space.connectivity()[e][n];
          for (Uint v=0; v<field.row_size(); ++v)
          {
            field_element_values(n,v) = field[p][v];
          }
        }

        for (Uint grad_pt=0; grad_pt<grad_space.shape_function().nb_nodes(); ++grad_pt)
        {
          // Compute jacobian of transformation to local coordinates in grad_pt
          entities.element_type().compute_jacobian(grad_space.shape_function().local_coordinates().row(grad_pt),
                                                   cell_coords,
                                                   jacobian);
          // Compute gradient
          grad_values.noalias() = n * jacobian.inverse() * gradient_matrix_per_point[grad_pt] * field_element_values;

          Uint p = grad_space.connectivity()[e][grad_pt];

          if (grad.continuous()) // just sum up, and divide by shared_nodes[p] later to average
          {
            shared_nodes[p] += 1;
            for (Uint d=0; d<ndim; ++d)
            {
              for (Uint v=0; v<field.row_size(); ++v)
              {
                grad[p][v+d*field.row_size()] += grad_values(d,v);
              }
            }
          }
          else // if discontinuous
          {
            for (Uint d=0; d<ndim; ++d)
            {
              for (Uint v=0; v<field.row_size(); ++v)
              {
                grad[p][v+d*field.row_size()] = grad_values(d,v);
              }
            }
          }
        }
      }
    }
  }

  // Divide by number of times the node was visited, to get the average
  if (grad.continuous())
  {
    for (Uint n=0; n<grad.size(); ++n)
    {
      if (shared_nodes[n] > 0)
      {
        for (Uint v=0; v<grad.row_size(); ++v)
        {
          grad[n][v] /= shared_nodes[n];
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3
