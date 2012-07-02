// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "common/Builder.hpp"

#include "math/MatrixTypesConversion.hpp"

#include "mesh/PseudoLaplacianLinearInterpolation.hpp"
#include "mesh/Field.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

  using namespace common;

cf3::common::ComponentBuilder < PseudoLaplacianLinearInterpolation, InterpolationFunction, LibMesh > PseudoLaplacianLinearInterpolation_builder;

////////////////////////////////////////////////////////////////////////////////

void PseudoLaplacianLinearInterpolation::compute_interpolation_weights(const RealVector& coordinate, const std::vector<SpaceElem>& stencil,
                                           std::vector<Uint>& source_field_points, std::vector<Real>& source_field_weights)
{
  cf3_assert_desc("Dictionary not configured in "+uri().string(), is_not_null(m_dict) );

  // collect s_points
  std::set<Uint> nodes;
  boost_foreach (const SpaceElem& PseudoLaplacianLinearInterpolation, stencil)
  {
    boost_foreach (const Uint node, PseudoLaplacianLinearInterpolation.nodes())
    {
      nodes.insert(node);
    }
  }
  source_field_points.reserve(nodes.size());
  boost_foreach (const Uint node, nodes)
  {
    source_field_points.push_back(node);
  }
  const Field& coordinates = m_dict->coordinates();
  std::vector<RealVector> s_points(nodes.size(),RealVector(coordinates.row_size()));
  for (Uint p=0; p<s_points.size(); ++p)
  {
    cf3_assert(source_field_points[p] < coordinates.size() );
    math::copy(coordinates[source_field_points[p]] , s_points[p]);
  }

  // allocate weights
  source_field_weights.resize(source_field_points.size());

  // call core algorithm to do all the work
  RealVector coord(s_points[0].size());
  for (Uint d=0; d<coordinate.size(); ++d)
    coord[d] = coordinate[d];
  pseudo_laplacian_weighted_linear_interpolation(coord, s_points, source_field_weights);
}

////////////////////////////////////////////////////////////////////////////////

void PseudoLaplacianLinearInterpolation::pseudo_laplacian_weighted_linear_interpolation(const RealVector& t_point, const std::vector<RealVector>& s_points, std::vector<Real>& weights)
{
  const Uint dim = s_points[0].size();
  cf3_assert(t_point.size() == s_points[0].size());
  switch (dim)
  {
    case DIM_3D:
    {
      Real Ixx(0),Ixy(0),Ixz(0),Iyy(0),Iyz(0),Izz(0), Rx(0),Ry(0),Rz(0), Lx,Ly,Lz, dx,dy,dz;
      RealVector Dx(s_points.size());
      RealVector Dy(s_points.size());
      RealVector Dz(s_points.size());
      for (Uint s_pt_idx=0; s_pt_idx<s_points.size(); ++s_pt_idx)
      {
        dx = s_points[s_pt_idx][XX] - t_point[XX];
        dy = s_points[s_pt_idx][YY] - t_point[YY];
        dz = s_points[s_pt_idx][ZZ] - t_point[ZZ];

        Ixx += dx*dx;
        Ixy += dx*dy;
        Ixz += dx*dz;
        Iyy += dy*dy;
        Iyz += dy*dz;
        Izz += dz*dz;

        Rx += dx;
        Ry += dy;
        Rz += dz;

        Dx[s_pt_idx]=dx;
        Dy[s_pt_idx]=dy;
        Dz[s_pt_idx]=dz;
      }
      Lx =  (-(Iyz*Iyz*Rx) + Iyy*Izz*Rx + Ixz*Iyz*Ry - Ixy*Izz*Ry - Ixz*Iyy*Rz + Ixy*Iyz*Rz)/
            (Ixz*Ixz*Iyy - 2.*Ixy*Ixz*Iyz + Ixy*Ixy*Izz + Ixx*(Iyz*Iyz - Iyy*Izz));
      Ly =  (Ixz*Iyz*Rx - Ixy*Izz*Rx - Ixz*Ixz*Ry + Ixx*Izz*Ry + Ixy*Ixz*Rz - Ixx*Iyz*Rz)/
            (Ixz*Ixz*Iyy - 2.*Ixy*Ixz*Iyz + Ixx*Iyz*Iyz + Ixy*Ixy*Izz - Ixx*Iyy*Izz);
      Lz =  (-(Ixz*Iyy*Rx) + Ixy*Iyz*Rx + Ixy*Ixz*Ry - Ixx*Iyz*Ry - Ixy*Ixy*Rz + Ixx*Iyy*Rz)/
            (Ixz*Ixz*Iyy - 2.*Ixy*Ixz*Iyz + Ixy*Ixy*Izz + Ixx*(Iyz*Iyz - Iyy*Izz));

      Real S(0);
      for (Uint s_pt_idx=0; s_pt_idx<s_points.size(); ++s_pt_idx)
      {
        weights[s_pt_idx] = 1.0 + Lx*Dx[s_pt_idx] + Ly*Dy[s_pt_idx] + Lz*Dz[s_pt_idx];
        S += weights[s_pt_idx];
      }

      for (Uint s_pt_idx=0; s_pt_idx<s_points.size(); ++s_pt_idx)
        weights[s_pt_idx] /= S;
      return;
    }
    case DIM_2D:
    {
      Real Ixx(0),Ixy(0),Iyy(0), Rx(0),Ry(0), Lx,Ly, dx,dy;
      RealVector Dx(s_points.size());
      RealVector Dy(s_points.size());
      for (Uint s_pt_idx=0; s_pt_idx<s_points.size(); ++s_pt_idx)
      {
        dx = s_points[s_pt_idx][XX] - t_point[XX];
        dy = s_points[s_pt_idx][YY] - t_point[YY];

        Ixx += dx*dx;
        Ixy += dx*dy;
        Iyy += dy*dy;

        Rx += dx;
        Ry += dy;

        Dx[s_pt_idx]=dx;
        Dy[s_pt_idx]=dy;
      }
      Lx =  (Ixy*Ry - Iyy*Rx)/(Ixx*Iyy-Ixy*Ixy);
      Ly =  (Ixy*Rx - Ixx*Ry)/(Ixx*Iyy-Ixy*Ixy);

      Real S(0);
      for (Uint s_pt_idx=0; s_pt_idx<s_points.size(); ++s_pt_idx)
      {
        weights[s_pt_idx] = 1.0 + Lx*Dx[s_pt_idx] + Ly*Dy[s_pt_idx] ;
        S += weights[s_pt_idx];
      }
      for (Uint s_pt_idx=0; s_pt_idx<s_points.size(); ++s_pt_idx)
        weights[s_pt_idx] /= S;
      return;
    }
    case DIM_1D:
    {
      Real Ixx(0), Rx(0), Lx, dx;
      RealVector Dx(s_points.size());
      for (Uint s_pt_idx=0; s_pt_idx<s_points.size(); ++s_pt_idx)
      {
        dx = s_points[s_pt_idx][XX] - t_point[XX];

        Ixx += dx*dx;

        Rx += dx;

        Dx[s_pt_idx]=dx;
      }
      Lx =  Rx/Ixx;

      Real S(0);
      for (Uint s_pt_idx=0; s_pt_idx<s_points.size(); ++s_pt_idx)
      {
        weights[s_pt_idx] = 1.0 + Lx*Dx[s_pt_idx];
        S += weights[s_pt_idx];
      }
      for (Uint s_pt_idx=0; s_pt_idx<s_points.size(); ++s_pt_idx)
        weights[s_pt_idx] /= S;
      return;
    }
    default:
      throw ShouldNotBeHere(FromHere(), "");
      break;
  }

}

//////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
