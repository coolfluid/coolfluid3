// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include <boost/algorithm/string/erase.hpp>
#include <boost/tuple/tuple.hpp>

#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/Link.hpp"

#include "math/Consts.hpp"

#include "mesh/LinearInterpolator.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Field.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/ElementData.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/UnifiedData.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

  using namespace common;
  using namespace math::Consts;

////////////////////////////////////////////////////////////////////////////////

cf3::common::ComponentBuilder < LinearInterpolator, Interpolator, LibMesh > LinearInterpolator_Builder;

//////////////////////////////////////////////////////////////////////////////

LinearInterpolator::LinearInterpolator( const std::string& name )
  : Interpolator(name), m_dim(0), m_bounding(2), m_N(3), m_D(3), m_point_idx(3), m_sufficient_nb_points(0)
{


  options().add_option( "ApproximateNbElementsPerCell", 1u)
      .description("The approximate amount of elements that are stored in a structured");

  std::vector<Uint> dummy;
  options().add_option( "Divisions", dummy)
      .description("The number of divisions in each direction of the comb. "
                        "Takes precedence over \"ApproximateNbElementsPerCell\". ");

  m_elements = create_component<UnifiedData>("elements");

}

//////////////////////////////////////////////////////////////////////////////

void LinearInterpolator::construct_internal_storage(Mesh& source)
{
  if (m_source_mesh != source.handle<Mesh>())
  {
    m_source_mesh = source.handle<Mesh>();
    create_bounding_box();
    create_octtree();
  }
}

//////////////////////////////////////////////////////////////////////

void LinearInterpolator::interpolate_field_from_to(const Field& source, Field& target)
{
  // Allocations
  Handle< Elements const > s_elements;
  Uint s_elm_idx;
  RealVector t_node(m_dim); t_node.setZero();

  if (source.continuous() && target.continuous())
  {
    const Field& source_coords = source.coordinates();
    const Field& target_coords = target.coordinates();

    for (Uint t_node_idx=0; t_node_idx<target.size(); ++t_node_idx)
    {
      to_vector(t_node,target_coords[t_node_idx]);
      boost::tie(s_elements,s_elm_idx) = find_element(t_node);
      if (is_not_null(s_elements))
      {
        Connectivity::ConstRow s_field_indexes = source.space(*s_elements).connectivity()[s_elm_idx];
        std::vector<RealVector> s_nodes(s_field_indexes.size(),RealVector(m_dim));

        fill( s_nodes , source_coords , s_field_indexes );

        std::vector<Real> w(s_nodes.size());
        pseudo_laplacian_weighted_linear_interpolation(s_nodes, t_node, w);

        for (Uint idata=0; idata<target.row_size(); ++idata)
          target[t_node_idx][idata] = 0;

        for (Uint s_node_idx=0; s_node_idx<s_nodes.size(); ++s_node_idx)
          for (Uint idata=0; idata<target.row_size(); ++idata)
            target[t_node_idx][idata] += w[s_node_idx]*source[s_field_indexes[s_node_idx]][idata];
      }
    }
  }
  else if ( source.discontinuous() && target.continuous() )
  {
    const Field& target_coords = target.coordinates();
    std::vector<Uint> s_field_indexes(0);
    std::vector<RealVector> s_nodes(0);
    Handle< Component > component;
    for (Uint t_node_idx=0; t_node_idx<target.size(); ++t_node_idx)
    {
      to_vector(t_node,target_coords[t_node_idx]);
      if (find_point_in_octtree(t_node,m_point_idx))
      {
        find_pointcloud(m_sufficient_nb_points);
        s_field_indexes.resize(0);
        s_nodes.resize(0);
        boost_foreach(const Uint glb_elem_idx, m_element_cloud)
        {
          boost::tie(component,s_elm_idx)=m_elements->location(glb_elem_idx);
          Elements const& elements = dynamic_cast<Elements const&>(*component);
          RealMatrix space_coords = source.space(elements).compute_coordinates(s_elm_idx);
          boost_foreach ( const Uint state_idx, source.space(elements).connectivity()[s_elm_idx] )
          {
            s_field_indexes.push_back(state_idx);
          }
          for (Uint i=0; i<space_coords.rows(); ++i)
          {
            s_nodes.push_back(space_coords.row(i));
          }
        }
        std::vector<Real> w(s_nodes.size());
        pseudo_laplacian_weighted_linear_interpolation(s_nodes, t_node, w);

        for (Uint idata=0; idata<target.row_size(); ++idata)
          target[t_node_idx][idata] = 0;

        for (Uint e=0; e<s_nodes.size(); ++e)
          for (Uint idata=0; idata<target.row_size(); ++idata)
            target[t_node_idx][idata] += w[e] * source[s_field_indexes[e]][idata];
      }
      else
      {
        std::stringstream ss; ss << "could not find node " << t_node.transpose();
        throw ValueNotFound(FromHere(),ss.str());
      }
    }
  }
  else if ( source.continuous() && target.discontinuous() )
  {
    Uint s_elm_idx;
    RealMatrix elem_coordinates;
    const Field& source_coords = source.coordinates();

    boost_foreach( const Handle<Space>& t_space_handle, target.spaces() )
    {
      const Space& t_space = *t_space_handle;
      t_space.allocate_coordinates(elem_coordinates);
      for (Uint t_elm_idx=0; t_elm_idx<t_space.size(); ++t_elm_idx)
      {
        Connectivity::ConstRow t_field_indexes = t_space.connectivity()[t_elm_idx];
        t_space.put_coordinates(elem_coordinates,t_elm_idx);
        for (Uint t_elm_point_idx=0; t_elm_point_idx<elem_coordinates.rows(); ++t_elm_point_idx)
        {
          const RealVector& t_node = elem_coordinates.row(t_elm_point_idx);
          boost::tie(s_elements,s_elm_idx) = find_element(t_node);
          if (is_not_null(s_elements))
          {
            Connectivity::ConstRow s_field_indexes = source.space(*s_elements).connectivity()[s_elm_idx];
            std::vector<RealVector> s_nodes(s_field_indexes.size(),RealVector(m_dim));

            fill( s_nodes , source_coords , s_field_indexes );

            std::vector<Real> w(s_nodes.size());
            pseudo_laplacian_weighted_linear_interpolation(s_nodes, t_node, w);


            for (Uint idata=0; idata<target.row_size(); ++idata)
              target[t_field_indexes[t_elm_point_idx]][idata] = 0;


            for (Uint s_node_idx=0; s_node_idx<s_nodes.size(); ++s_node_idx)
              for (Uint idata=0; idata<target.row_size(); ++idata)
                target[t_field_indexes[t_elm_point_idx]][idata] += w[s_node_idx]*source[s_field_indexes[s_node_idx]][idata];
          }
        }
      }
    }
  }
  else if ( source.discontinuous() && target.discontinuous() )
  {
    Uint s_elm_idx;
    //Uint t_elm_idx;
    RealMatrix elem_coordinates;
    std::vector<Uint> s_field_indexes(0);
    std::vector<RealVector> s_nodes;
    Handle< Component > component;
    boost_foreach( const Handle<Space>& t_space_handle, target.spaces() )
    {
      const Space& t_space = *t_space_handle;
      t_space.allocate_coordinates(elem_coordinates);
      RealVector t_node(m_dim);  t_node.setZero();
      for (Uint t_elm_idx=0; t_elm_idx<t_space.size(); ++t_elm_idx)
      {
        Connectivity::ConstRow t_field_indexes = t_space.connectivity()[t_elm_idx];
        t_space.put_coordinates(elem_coordinates,t_elm_idx);
        for (Uint t_elm_point_idx=0; t_elm_point_idx<elem_coordinates.rows(); ++t_elm_point_idx)
        {
          for (Uint d=0; d<elem_coordinates.cols(); ++d)
            t_node[d] = elem_coordinates(t_elm_point_idx,d);


          if (find_point_in_octtree(t_node,m_point_idx))
          {
            find_pointcloud(m_sufficient_nb_points);

            s_field_indexes.resize(0);
            s_nodes.resize(0);
            boost_foreach(const Uint glb_elem_idx, m_element_cloud)
            {
              boost::tie(component,s_elm_idx)=m_elements->location(glb_elem_idx);
              Elements const& elements = dynamic_cast<Elements const&>(*component);
              RealMatrix space_coords = source.space(elements).get_coordinates(s_elm_idx);
              boost_foreach ( const Uint state_idx, source.space(elements).connectivity()[s_elm_idx] )
              {
                s_field_indexes.push_back(state_idx);
              }
              for (Uint i=0; i<space_coords.rows(); ++i)
              {
                s_nodes.push_back(space_coords.row(i));
              }
            }

            std::vector<Real> w(s_nodes.size());
            pseudo_laplacian_weighted_linear_interpolation(s_nodes, t_node, w);

            for (Uint idata=0; idata<target.row_size(); ++idata)
              target[t_field_indexes[t_elm_point_idx]][idata] = 0;


            for (Uint s_node_idx=0; s_node_idx<s_nodes.size(); ++s_node_idx)
              for (Uint idata=0; idata<target.row_size(); ++idata)
                target[t_field_indexes[t_elm_point_idx]][idata] += w[s_node_idx]*source[s_field_indexes[s_node_idx]][idata];
          }
          else
          {
            throw ValueNotFound(FromHere(),"could not find node");
          }
        }
      }
    }
  }
  else
  {
    throw ShouldNotBeHere(FromHere(), "Field::basis() should return NODE_BASED or ELEMENT_BASED");
  }
}

//////////////////////////////////////////////////////////////////////

void LinearInterpolator::create_bounding_box()
{
  m_dim=m_source_mesh->dimension();

  // find bounding box coordinates for region 1 and region 2
  m_bounding[MIN].setConstant(real_max());
  m_bounding[MAX].setConstant(real_min());

  boost_foreach(common::Table<Real>::ConstRow coords, m_source_mesh->geometry_fields().coordinates().array())
  {
    for (Uint d=0; d<m_dim; ++d)
    {
      m_bounding[MIN][d] = std::min(m_bounding[MIN][d],  coords[d]);
      m_bounding[MAX][d] = std::max(m_bounding[MAX][d],  coords[d]);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void LinearInterpolator::create_octtree()
{
  std::vector<Real> L(3);

  Real V=1;
  for (Uint d=0; d<m_dim; ++d)
  {
    L[d] = m_bounding[MAX][d] - m_bounding[MIN][d];
    V*=L[d];
  }

  m_nb_elems = m_source_mesh->properties().value<Uint>("nb_cells");

  if (options().option("Divisions").value<std::vector<Uint> >().size() > 0)
  {
    m_N = options().option("Divisions").value<std::vector<Uint> >();
    for (Uint d=0; d<m_dim; ++d)
      m_D[d] = (L[d])/static_cast<Real>(m_N[d]);
  }
  else
  {
    Real V1 = V/m_nb_elems;
    Real D1 = std::pow(V1,1./m_dim)*options().option("ApproximateNbElementsPerCell").value<Uint>();

    for (Uint d=0; d<m_dim; ++d)
    {
      m_N[d] = (Uint) std::ceil(L[d]/D1);
      m_D[d] = (L[d])/static_cast<Real>(m_N[d]);
    }
  }

  CFinfo << "Honeycomb:" << CFendl;
  CFinfo << "----------" << CFendl;
  for (Uint d=0; d<m_dim; ++d)
  {
    CFinfo << "range["<<d<<"] :   L = " << L[d] << "    N = " << m_N[d] << "    D = " << m_D[d] << CFendl;
  }
  CFinfo << "V = " << V << CFendl;

  // initialize the honeycomb
  m_honeycomb.resize(boost::extents[std::max(Uint(1),m_N[0])][std::max(Uint(1),m_N[1])][std::max(Uint(1),m_N[2])]);

  boost_foreach(Elements& elements, find_components_recursively_with_filter<Elements>(*m_source_mesh,IsElementsVolume()))
    m_elements->add(elements);

  Uint glb_elem_idx=0;
  RealVector centroid(m_dim);
  boost_foreach(const Elements& elements, find_components_recursively_with_filter<Elements>(*m_source_mesh,IsElementsVolume()))
  {
    Uint nb_nodes_per_element = elements.geometry_space().connectivity().row_size();
    RealMatrix coordinates;
    elements.geometry_space().allocate_coordinates(coordinates);

    for (Uint elem_idx=0; elem_idx<elements.size(); ++elem_idx)
    {
      elements.geometry_space().put_coordinates(coordinates,elem_idx);
      elements.element_type().compute_centroid(coordinates,centroid);
      for (Uint d=0; d<m_dim; ++d)
        m_point_idx[d]=std::min((Uint) std::floor( (centroid[d] - m_bounding[MIN][d])/m_D[d]), m_N[d]-1 );
      m_honeycomb[m_point_idx[0]][m_point_idx[1]][m_point_idx[2]].push_back(glb_elem_idx);
      ++glb_elem_idx;
    }
  }


  Uint total=0;

  switch (m_dim)
  {
    case DIM_2D:
      for (Uint i=0; i<m_N[0]; ++i)
        for (Uint j=0; j<m_N[1]; ++j)
        {
          Uint k=0;
          // CFinfo << "("<<i<<","<<j<<") has elems ";
          // if (m_honeycomb[i][j][k].size())
          //   CFinfo << mesh::to_vector(m_honeycomb[i][j][k]).transpose() << CFendl;
          // else
          //   CFinfo << CFendl;
          total += m_honeycomb[i][j][k].size();
        }
      break;
    case DIM_3D:
      for (Uint i=0; i<m_N[0]; ++i)
        for (Uint j=0; j<m_N[1]; ++j)
          for (Uint k=0; k<m_N[2]; ++k)
          {
            // CFinfo << "("<<i<<","<<j<<","<<k<<") has elems ";
            // if (m_honeycomb[i][j][k].size())
            //   CFinfo << mesh::to_vector(m_honeycomb[i][j][k]).transpose() << CFendl;
            // else
            //   CFinfo << CFendl;
            total += m_honeycomb[i][j][k].size();
          }
      break;
    default:
      break;
  }

  CFinfo << "total = " << total << " of " << m_nb_elems << CFendl;

  m_sufficient_nb_points = static_cast<Uint>(std::pow(3.,(int)m_dim));

}


//////////////////////////////////////////////////////////////////////////////

bool LinearInterpolator::find_point_in_octtree(const RealVector& coordinate, std::vector<Uint>& point_idx)
{
  //CFinfo << "point " << coordinate << CFflush;
  cf3_assert(coordinate.size() == static_cast<int>(m_dim));

  for (Uint d=0; d<m_dim; ++d)
  {
    if ( (coordinate[d] - m_bounding[MIN][d])/m_D[d] > m_N[d])
    {
      //CFinfo << "coord["<<d<<"] = " << coordinate[d] <<" is not inside bounding box" << CFendl;
      //CFinfo << (coordinate[d] - m_ranges[d][0])/m_D[d] << " > " << m_N[d] << CFendl;
      return false; // no index found
    }
    point_idx[d] = std::min((Uint) std::floor( (coordinate[d] - m_bounding[MIN][d])/m_D[d]), m_N[d]-1 );
  }

  //CFinfo << " should be in box ("<<m_point_idx[0]<<","<<m_point_idx[1]<<","<<m_point_idx[2]<<")" << CFendl;
  return true;
}

//////////////////////////////////////////////////////////////////////////////

void LinearInterpolator::find_pointcloud(Uint nb_points)
{
  m_element_cloud.resize(0);
  Uint r=1;
  int i(0), j(0), k(0);
  int imin, jmin, kmin;
  int imax, jmax, kmax;

  if (m_honeycomb[m_point_idx[0]][m_point_idx[1]][m_point_idx[2]].size() <= nb_points)
    r=0;

  while (m_element_cloud.size() < nb_points && m_element_cloud.size() < m_nb_elems)
  {
    ++r;
    m_element_cloud.resize(0);

    switch (m_dim)
    {
      case DIM_3D:
        imin = std::max(int(m_point_idx[0])-int(r), 0);  imax = std::min(int(m_point_idx[0])+int(r),int(m_N[0])-1);
        jmin = std::max(int(m_point_idx[1])-int(r), 0);  jmax = std::min(int(m_point_idx[1])+int(r),int(m_N[1])-1);
        kmin = std::max(int(m_point_idx[2])-int(r), 0);  kmax = std::min(int(m_point_idx[2])+int(r),int(m_N[2])-1);
        for (i = imin; i <= imax; ++i)
          for (j = jmin; j <= jmax; ++j)
            for (k = kmin; k <= kmax; ++k)
            {
              boost_foreach(const Uint glb_elem_idx, m_honeycomb[i][j][k])
              m_element_cloud.push_back(glb_elem_idx);
              //CFinfo << "   ("<<i<<","<<j<<","<<k<<")" <<  CFendl;
            }
        break;
      case DIM_2D:
        imin = std::max(int(m_point_idx[0])-int(r), 0);  imax = std::min(int(m_point_idx[0])+int(r),int(m_N[0])-1);
        jmin = std::max(int(m_point_idx[1])-int(r), 0);  jmax = std::min(int(m_point_idx[1])+int(r),int(m_N[1])-1);
        for (i = imin; i <= imax; ++i)
          for (j = jmin; j <= jmax; ++j)
          {
            boost_foreach(const Uint glb_elem_idx, m_honeycomb[i][j][k])
            m_element_cloud.push_back(glb_elem_idx);
            //CFinfo << "   ("<<i<<","<<j<<","<<k<<")" <<  CFendl;
          }
        break;
      case DIM_1D:
        imin = std::max(int(m_point_idx[0])-int(r), 0);  imax = std::min(int(m_point_idx[0])+int(r),int(m_N[0])-1);
        for (i = imin; i <= imax; ++i)
        {
          boost_foreach(const Uint glb_elem_idx, m_honeycomb[i][j][k])
          m_element_cloud.push_back(glb_elem_idx);
          //CFinfo << "   ("<<i<<","<<j<<","<<k<<")" <<  CFendl;
        }
        break;
    }
  }

  //CFinfo << m_pointcloud.size() << " points in the pointcloud " << CFendl;

}

//////////////////////////////////////////////////////////////////////////////

boost::tuple<Handle< Elements const >,Uint> LinearInterpolator::find_element(const RealVector& target_coord)
{
  if (find_point_in_octtree(target_coord,m_point_idx))
  {
    find_pointcloud(1);

    Handle< Component > component;
    Uint elem_idx;
    boost_foreach(const Uint glb_elem_idx, m_element_cloud)
    {
      boost::tie(component,elem_idx)=m_elements->location(glb_elem_idx);
      Elements const& elements = dynamic_cast<Elements const&>(*component);
      RealMatrix elem_coordinates = elements.geometry_space().get_coordinates(elem_idx);
      if (elements.element_type().is_coord_in_element(target_coord,elem_coordinates))
      {
        return boost::make_tuple(Handle<Elements const>(elements.handle()),elem_idx);
      }
    }
  }
  return boost::make_tuple(Handle< Elements >(), 0u);
}

//////////////////////////////////////////////////////////////////////

void LinearInterpolator::pseudo_laplacian_weighted_linear_interpolation(const std::vector<RealVector>& s_points, const RealVector& t_point, std::vector<Real>& weights)
{
  switch (m_dim)
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
