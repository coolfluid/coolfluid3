// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include <boost/algorithm/string/erase.hpp>
#include <boost/tuple/tuple.hpp>

#include "Common/Foreach.hpp"
#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionArray.hpp"
#include "Common/CLink.hpp"

#include "Math/MathConsts.hpp"
#include "Mesh/CLinearInterpolator.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CField2.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/ElementData.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CSpace.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

  using namespace Common;
  using namespace Math::MathConsts;
  
////////////////////////////////////////////////////////////////////////////////

CF::Common::ComponentBuilder < CLinearInterpolator, CInterpolator, LibMesh > aHoneyCombInterpolator_Builder;

//////////////////////////////////////////////////////////////////////////////

CLinearInterpolator::CLinearInterpolator( const std::string& name )
  : CInterpolator(name), m_dim(0), m_bounding(2), m_N(3), m_D(3), m_comb_idx(3), m_sufficient_nb_points(0)
{
   

  m_properties.add_option< OptionT<Uint> >
  ( "ApproximateNbElementsPerCell",
    "The approximate amount of elements that are stored in a structured" ,
    1 );

  std::vector<Uint> dummy;
  m_properties.add_option< OptionArrayT<Uint> >
  ( "Divisions",
    "The number of divisions in each direction of the comb. Takes precedence over \"ApproximateNbElementsPerCell\". " ,
    dummy);
    
  m_elements = create_component<CUnifiedData<CElements const> >("elements");

}

//////////////////////////////////////////////////////////////////////////////

void CLinearInterpolator::construct_internal_storage(const CMesh& source)
{
  if (m_source_mesh != source.as_ptr<CMesh>())
  {
    m_source_mesh = source.as_ptr<CMesh>();
    create_bounding_box();
    create_octtree();
  }
}
  
//////////////////////////////////////////////////////////////////////
  
void CLinearInterpolator::interpolate_field_from_to(const CField2& source, CField2& target)
{  
  const CTable<Real>& s_data = source.data();
  CTable<Real>& t_data = target.data();
  
  // Allocations
  CElements::ConstPtr s_elements;
  Uint s_elm_idx;
  RealVector t_node(m_dim); t_node.setZero();
  
  if (source.basis() == CField2::Basis::POINT_BASED && target.basis() == CField2::Basis::POINT_BASED)
  {    
    for (Uint t_node_idx=0; t_node_idx<t_data.size(); ++t_node_idx)
    {
      to_vector(t_node,target.coords(t_node_idx));
      boost::tie(s_elements,s_elm_idx) = find_element(t_node);
      if (is_not_null(s_elements))
      {
        CTable<Uint>::ConstRow s_elm = s_elements->space(source.space_idx()).connectivity_table()[s_elm_idx];
        std::vector<RealVector> s_nodes(s_elm.size(),RealVector(m_dim));
        fill( s_nodes , s_elements->nodes().coordinates() , s_elm );
        
        std::vector<Real> w(s_nodes.size());
        pseudo_laplacian_weighted_linear_interpolation(s_nodes, t_node, w);
        
        for (Uint idata=0; idata<t_data.row_size(); ++idata)
          t_data[t_node_idx][idata] = 0;
        
        for (Uint s_node_idx=0; s_node_idx<s_nodes.size(); ++s_node_idx)
          for (Uint idata=0; idata<t_data.row_size(); ++idata)
            t_data[t_node_idx][idata] += w[s_node_idx]*s_data[s_elm[s_node_idx]][idata];
      }
    }
  }
  else if (source.basis() == CField2::Basis::ELEMENT_BASED && target.basis() == CField2::Basis::POINT_BASED)
  {
    for (Uint t_node_idx=0; t_node_idx<t_data.size(); ++t_node_idx)
    {
      to_vector(t_node,target.coords(t_node_idx));
      if (find_comb_idx(t_node))
      {
        find_pointcloud(m_sufficient_nb_points);
        std::vector<Uint> s_data_idx(m_element_cloud.size());
        std::vector<RealVector> s_centroids(m_element_cloud.size(),RealVector(m_dim));
        Uint cnt(0);
        boost_foreach(const Uint glb_elem_idx, m_element_cloud)
        {
          boost::tie(s_elements,s_elm_idx)=m_elements->data_location(glb_elem_idx);
          CElements const& elements = *s_elements;
          
          RealMatrix elem_coords = elements.get_coordinates(s_elm_idx);
          elements.element_type().compute_centroid(elem_coords,s_centroids[cnt]);
          s_data_idx[cnt] = source.elements_start_idx(elements) + s_elm_idx;            
          ++cnt;
        } 
        
        std::vector<Real> w(s_centroids.size());
        pseudo_laplacian_weighted_linear_interpolation(s_centroids, t_node, w);
        
        for (Uint idata=0; idata<t_data.row_size(); ++idata)
          t_data[t_node_idx][idata] = 0;
        
        for (Uint e=0; e<s_centroids.size(); ++e)
          for (Uint idata=0; idata<t_data.row_size(); ++idata)
            t_data[t_node_idx][idata] += w[e] * s_data[s_data_idx[e]][idata];
      }
      else
      {
        throw ValueNotFound(FromHere(),"could not find node");
      }
    }
  }
  else if (source.basis() == CField2::Basis::POINT_BASED && target.basis() == CField2::Basis::ELEMENT_BASED)
  {
    CFieldView t_view("t_view");
    t_view.set_field(target);
    RealVector t_centroid(m_dim);
    t_centroid.setZero();
    Uint s_elm_idx;
    Uint t_elm_idx;
    RealMatrix elem_coordinates;
    
    boost_foreach( CElements& t_elements, find_components_recursively<CElements>(target.topology()) )
    {
      if (t_view.set_elements(t_elements))
      {
        t_view.allocate_coordinates(elem_coordinates);
        for (Uint t_elm_idx=0; t_elm_idx<t_elements.size(); ++t_elm_idx)
        {
          t_view.put_coordinates(elem_coordinates,t_elm_idx);
          t_view.space().shape_function().compute_centroid(elem_coordinates,t_centroid);
          
          boost::tie(s_elements,s_elm_idx) = find_element(t_centroid);
          if (is_not_null(s_elements))
          {
            CTable<Uint>::ConstRow s_elm = s_elements->space(source.space_idx()).connectivity_table()[s_elm_idx];
            std::vector<RealVector> s_nodes(s_elm.size(),RealVector(m_dim));
            fill( s_nodes , s_elements->nodes().coordinates() , s_elm );

            std::vector<Real> w(s_nodes.size());
            pseudo_laplacian_weighted_linear_interpolation(s_nodes, t_centroid, w);

            for (Uint idata=0; idata<t_data.row_size(); ++idata)
              t_view[t_elm_idx][idata] = 0;

            for (Uint s_node_idx=0; s_node_idx<s_nodes.size(); ++s_node_idx)
              for (Uint idata=0; idata<t_data.row_size(); ++idata)
                t_view[t_elm_idx][idata] += w[s_node_idx]*s_data[s_elm[s_node_idx]][idata];
          }
        }
      }
    }
  }
  else if (source.basis() == CField2::Basis::ELEMENT_BASED && target.basis() == CField2::Basis::ELEMENT_BASED)
  {



    CFieldView t_view("t_view");
    t_view.set_field(target);
    RealVector t_centroid(m_dim);
    t_centroid.setZero();
    Uint s_elm_idx;
    Uint t_elm_idx;
    RealMatrix elem_coordinates;
    
    boost_foreach( CElements& t_elements, find_components_recursively<CElements>(target.topology()) )
    {
      if (t_view.set_elements(t_elements))
      {
        t_view.allocate_coordinates(elem_coordinates);
        for (Uint t_elm_idx=0; t_elm_idx<t_elements.size(); ++t_elm_idx)
        {
          t_view.put_coordinates(elem_coordinates,t_elm_idx);
          t_view.space().shape_function().compute_centroid(elem_coordinates,t_centroid);
          


          if (find_comb_idx(t_centroid))
          {
            find_pointcloud(m_sufficient_nb_points);
            std::vector<Uint> s_data_idx(m_element_cloud.size());
            std::vector<RealVector> s_centroids(m_element_cloud.size(),RealVector(m_dim));
            Uint cnt(0);
            boost_foreach(const Uint glb_elem_idx, m_element_cloud)
            {
              boost::tie(s_elements,s_elm_idx)=m_elements->data_location(glb_elem_idx);
              CElements const& elements = *s_elements;

              RealMatrix elem_coords = elements.get_coordinates(s_elm_idx);
              elements.element_type().compute_centroid(elem_coords,s_centroids[cnt]);
              s_data_idx[cnt] = source.elements_start_idx(elements) + s_elm_idx;            
              ++cnt;
            } 

            std::vector<Real> w(s_centroids.size());
            pseudo_laplacian_weighted_linear_interpolation(s_centroids, t_centroid, w);

            for (Uint idata=0; idata<t_data.row_size(); ++idata)
              t_view[t_elm_idx][idata] = 0;

            for (Uint e=0; e<s_centroids.size(); ++e)
              for (Uint idata=0; idata<t_data.row_size(); ++idata)
                t_view[t_elm_idx][idata] += w[e] * s_data[s_data_idx[e]][idata];
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
    throw ShouldNotBeHere(FromHere(), "CField2::basis() should return NODE_BASED or ELEMENT_BASED");
  }
}

//////////////////////////////////////////////////////////////////////

void CLinearInterpolator::create_bounding_box()
{
  m_dim=0;

  boost_foreach(const CElements& elements, find_components_recursively<CElements>(*m_source_mesh))
  {
    m_dim = std::max(elements.element_type().dimensionality() , m_dim);
  }
  
  // find bounding box coordinates for region 1 and region 2
  m_bounding[MIN].setConstant(Real_max());    
  m_bounding[MAX].setConstant(Real_min());
    
  boost_foreach(CTable<Real>::ConstRow coords, m_source_mesh->nodes().coordinates().array())
  {
    for (Uint d=0; d<m_dim; ++d)
    {
      m_bounding[MIN][d] = std::min(m_bounding[MIN][d],  coords[d]);
      m_bounding[MAX][d] = std::max(m_bounding[MAX][d],  coords[d]);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void CLinearInterpolator::create_octtree()
{
  std::vector<Real> L(3);

  Real V=1;
  for (Uint d=0; d<m_dim; ++d)
  {
    L[d] = m_bounding[MAX][d] - m_bounding[MIN][d];
    V*=L[d];
  }

  m_nb_elems = m_source_mesh->topology().recursive_filtered_elements_count(IsElementsVolume());


  if (property("Divisions").value<std::vector<Uint> >().size() > 0)
  {
    m_N = property("Divisions").value<std::vector<Uint> >();
    for (Uint d=0; d<m_dim; ++d)
      m_D[d] = (L[d])/static_cast<Real>(m_N[d]);
  }
  else
  {
    Real V1 = V/m_nb_elems;
    Real D1 = std::pow(V1,1./m_dim)*property("ApproximateNbElementsPerCell").value<Uint>();

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

  std::vector<CElements::ConstPtr> elements_vector = find_components_recursively_with_filter<CElements>(*m_source_mesh,IsElementsVolume()).as_const_vector();
  m_elements->add_data(elements_vector);
  
  Uint glb_elem_idx=0;
  RealVector centroid(m_dim);
  boost_foreach(CElements::ConstPtr elements_ptr, elements_vector)
  {
    const CElements& elements = *elements_ptr;
    Uint nb_nodes_per_element = elements.connectivity_table().row_size();
    RealMatrix coordinates(nb_nodes_per_element,m_dim);
    
    for (Uint elem_idx=0; elem_idx<elements.size(); ++elem_idx)
    {
      elements.put_coordinates(coordinates,elem_idx);
      elements.element_type().compute_centroid(coordinates,centroid);
      for (Uint d=0; d<m_dim; ++d)
        m_comb_idx[d]=std::min((Uint) std::floor( (centroid[d] - m_bounding[MIN][d])/m_D[d]), m_N[d]-1 );
      m_honeycomb[m_comb_idx[0]][m_comb_idx[1]][m_comb_idx[2]].push_back(glb_elem_idx);
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
          //   CFinfo << Mesh::to_vector(m_honeycomb[i][j][k]).transpose() << CFendl;
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
            //   CFinfo << Mesh::to_vector(m_honeycomb[i][j][k]).transpose() << CFendl;
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
  
bool CLinearInterpolator::find_comb_idx(const RealVector& coordinate)
{
  //CFinfo << "point " << coordinate << CFflush;
  cf_assert(coordinate.size() == static_cast<int>(m_dim));

  for (Uint d=0; d<m_dim; ++d)
  {
    if ( (coordinate[d] - m_bounding[MIN][d])/m_D[d] > m_N[d])
    {
      //CFinfo << "coord["<<d<<"] = " << coordinate[d] <<" is not inside bounding box" << CFendl;
      //CFinfo << (coordinate[d] - m_ranges[d][0])/m_D[d] << " > " << m_N[d] << CFendl;
      return false; // no index found
    }
    m_comb_idx[d] = std::min((Uint) std::floor( (coordinate[d] - m_bounding[MIN][d])/m_D[d]), m_N[d]-1 );
  }
      
  //CFinfo << " should be in box ("<<m_comb_idx[0]<<","<<m_comb_idx[1]<<","<<m_comb_idx[2]<<")" << CFendl;
  return true;
}

//////////////////////////////////////////////////////////////////////////////

void CLinearInterpolator::find_pointcloud(Uint nb_points)
{
  m_element_cloud.resize(0);
  Uint r=1;
  int i(0), j(0), k(0);
  int imin, jmin, kmin;
  int imax, jmax, kmax;
  
  if (m_honeycomb[m_comb_idx[0]][m_comb_idx[1]][m_comb_idx[2]].size() <= nb_points)
    r=0;
  
  while (m_element_cloud.size() < nb_points && m_element_cloud.size() < m_nb_elems)
  {
    ++r;
    m_element_cloud.resize(0);

    switch (m_dim)
    {
      case DIM_3D:
        imin = std::max(int(m_comb_idx[0])-int(r), 0);  imax = std::min(int(m_comb_idx[0])+int(r),int(m_N[0])-1);
        jmin = std::max(int(m_comb_idx[1])-int(r), 0);  jmax = std::min(int(m_comb_idx[1])+int(r),int(m_N[1])-1);
        kmin = std::max(int(m_comb_idx[2])-int(r), 0);  kmax = std::min(int(m_comb_idx[2])+int(r),int(m_N[2])-1);
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
        imin = std::max(int(m_comb_idx[0])-int(r), 0);  imax = std::min(int(m_comb_idx[0])+int(r),int(m_N[0])-1);
        jmin = std::max(int(m_comb_idx[1])-int(r), 0);  jmax = std::min(int(m_comb_idx[1])+int(r),int(m_N[1])-1);
        for (i = imin; i <= imax; ++i)
          for (j = jmin; j <= jmax; ++j)
          {
            boost_foreach(const Uint glb_elem_idx, m_honeycomb[i][j][k])
            m_element_cloud.push_back(glb_elem_idx);
            //CFinfo << "   ("<<i<<","<<j<<","<<k<<")" <<  CFendl;
          }
        break;
      case DIM_1D:
        imin = std::max(int(m_comb_idx[0])-int(r), 0);  imax = std::min(int(m_comb_idx[0])+int(r),int(m_N[0])-1);
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

boost::tuple<CElements::ConstPtr,Uint> CLinearInterpolator::find_element(const RealVector& target_coord)
{
  if (find_comb_idx(target_coord))
  {
    find_pointcloud(1);
    
    CElements::ConstPtr elements;
    Uint elem_idx;
    boost_foreach(const Uint glb_elem_idx, m_element_cloud)
    {
      boost::tie(elements,elem_idx)=m_elements->data_location(glb_elem_idx);
      
      RealMatrix elem_coordinates = elements->get_coordinates(elem_idx);
      if (elements->element_type().is_coord_in_element(target_coord,elem_coordinates))
      {
        return boost::make_tuple(elements,elem_idx);
      }
    }
  }
  return boost::make_tuple(CElements::ConstPtr(), 0u);
}
  
//////////////////////////////////////////////////////////////////////
  
void CLinearInterpolator::pseudo_laplacian_weighted_linear_interpolation(const std::vector<RealVector>& s_points, const RealVector& t_point, std::vector<Real>& weights)
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

} // Mesh
} // CF
