#include <boost/foreach.hpp>
#include <boost/algorithm/string/erase.hpp>
#include "Common/ObjectProvider.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/OptionT.hpp"


#include "Mesh/CHoneycombInterpolator.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

  using namespace Common;
  
////////////////////////////////////////////////////////////////////////////////

CF::Common::ObjectProvider < Mesh::CHoneycombInterpolator,
                             Mesh::CInterpolator,
                             MeshLib,
                             NB_ARGS_1 >
aHoneyCombInterpolatorProvider ( "Honeycomb" );

//////////////////////////////////////////////////////////////////////////////

CHoneycombInterpolator::CHoneycombInterpolator( const CName& name )
  : CInterpolator(name), m_dim(0), m_ranges(3), m_N(3), m_D(3), m_sufficient_nb_points(0)
{
  BUILD_COMPONENT;
}
  
/////////////////////////////////////////////////////////////////////////////

void CHoneycombInterpolator::defineConfigOptions ( CF::Common::OptionList& options )
{
  options.add< OptionT<Uint> >
  ( "ApproximateNbElementsPerCell",
    "The approximate amount of elements that are stored in a structured" ,
    1 );   
}  

//////////////////////////////////////////////////////////////////////////////

void CHoneycombInterpolator::construct_internal_storage(const CMesh::Ptr& source, const CMesh::Ptr& target)
{
  if (m_source_mesh != source)
  {
    m_source_mesh = source;
    create_honeycomb();
  }
  m_target_mesh = target;
  
  find_pointcloud(RealVector(5.5,3));

}
  
/////////////////////////////////////////////////////////////////////////////
  
void CHoneycombInterpolator::interpolate_field_from_to(const CField::Ptr& source, const CField::Ptr& target)
{
  
}
  
//////////////////////////////////////////////////////////////////////

void CHoneycombInterpolator::create_honeycomb()
{
  m_dim=0;
  std::set<const CArray*> all_coordinates;
  BOOST_FOREACH(CElements& elements, recursive_range_typed<CElements>(*m_source_mesh))
  { 
    m_dim = std::max(elements.element_type().dimensionality() , m_dim);
    all_coordinates.insert(&elements.coordinates());
  }
  
  std::vector<Real> L(3);
  for (Uint d=0; d<m_dim; ++d)
  {
    m_ranges[d].resize(2,0.0);
  }
      
  Real V=1;
  BOOST_FOREACH(const CArray* coordinates , all_coordinates)
  {
    BOOST_FOREACH(const CArray::ConstRow& node, coordinates->array())
    {
      for (Uint d=0; d<m_dim; ++d)
      {
        m_ranges[d][0] = std::min(m_ranges[d][0],  node[d]);
        m_ranges[d][1] = std::max(m_ranges[d][1],  node[d]);
      }
    }
  }
  for (Uint d=0; d<m_dim; ++d)
  {
    L[d] = m_ranges[d][1] - m_ranges[d][0];
    V*=L[d];
  }
  
  Uint nb_elems = m_source_mesh->get_child_type<CRegion>("regions")->recursive_filtered_elements_count(IsElementsVolume());
  Real V1 = V/nb_elems;
  Real D1 = std::pow(V1,1./m_dim)*option("ApproximateNbElementsPerCell")->value<Uint>();
  
  std::vector<Uint> comb_idx(3);
  for (Uint d=0; d<m_dim; ++d)
  {
    m_N[d] = (Uint) std::ceil(L[d]/D1);
    //N[d] = std::ceil(ranges[d][1]-ranges[d][0])/2.;
    m_D[d] = (L[d])/static_cast<Real>(m_N[d]);
  }
  
  for (Uint d=0; d<m_dim; ++d)
  {
    CFinfo << "range["<<d<<"] :   L = " << L[d] << "    N = " << m_N[d] << "    D = " << m_D[d] << CFendl;
  }
  CFinfo << "V = " << V << CFendl;
  
  // initialize the honeycomb
  m_honeycomb.resize(boost::extents[std::max(Uint(1),m_N[0])][std::max(Uint(1),m_N[1])][std::max(Uint(1),m_N[2])]);
  
  
  Uint total_nb_elems=0;
  BOOST_FOREACH(const CElements& elements, recursive_filtered_range_typed<CElements>(*m_source_mesh,IsElementsVolume()))
  {
    const CArray& coordinates = elements.coordinates();
    Uint nb_nodes_per_element = elements.connectivity_table().table().shape()[1];
    Uint elem_idx=0;
    BOOST_FOREACH(const CTable::ConstRow& elem, elements.connectivity_table().table())
    {
      RealVector centroid(0.0,m_dim);
      BOOST_FOREACH(const Uint node_idx, elem)
        centroid += RealVector(coordinates[node_idx]);        

      centroid /= static_cast<Real>(nb_nodes_per_element);
      for (Uint d=0; d<m_dim; ++d)
        comb_idx[d]=std::min((Uint) std::floor( (centroid[d] - m_ranges[d][0])/m_D[d]), m_N[d]-1 );
      m_honeycomb[comb_idx[0]][comb_idx[1]][comb_idx[2]].push_back(std::make_pair<const CElements*,Uint>(&elements,elem_idx));
      elem_idx++;
    }
    total_nb_elems += elem_idx;
  }
  
  
  Uint total=0;
 
  switch (m_dim)
  {
    case 2:
      for (Uint i=0; i<m_N[0]; ++i)
        for (Uint j=0; j<m_N[1]; ++j)
        {
          Uint k=0;
          CFinfo << "("<<i<<","<<j<<") has " << m_honeycomb[i][j][k].size() << " elems" << CFendl;
          total += m_honeycomb[i][j][k].size();
        }
      break;
    case 3:
      for (Uint i=0; i<m_N[0]; ++i)
        for (Uint j=0; j<m_N[1]; ++j)
          for (Uint k=0; k<m_N[2]; ++k)
          {
            CFinfo << "("<<i<<","<<j<<","<<k<<") has " << m_honeycomb[i][j][k].size() << " elems" << CFendl;
            total += m_honeycomb[i][j][k].size();
          }
      break;
    default:
      break;
  }

  CFinfo << "total = " << total << " of " << nb_elems << CFendl;
  
  m_sufficient_nb_points = static_cast<Uint>(std::pow(3.,(int)m_dim));

}


//////////////////////////////////////////////////////////////////////////////
  
void CHoneycombInterpolator::find_pointcloud(const RealVector& coordinate)
{
  CFinfo << "point " << coordinate << CFflush;
  m_pointcloud.resize(0);
  Uint r=1;
  cf_assert(coordinate.size() == m_dim);
  
  std::vector<Uint> comb_idx(3);
  std::vector<Uint> running_idx(3);
  
  for (Uint d=0; d<m_dim; ++d)
    comb_idx[d] = std::min((Uint) std::floor( (coordinate[d] - m_ranges[d][0])/m_D[d]), m_N[d]-1 );
  
  CFinfo << " should be in box ("<<comb_idx[0]<<","<<comb_idx[1]<<","<<comb_idx[2]<<")" << CFendl;
  int i(0), j(0), k(0);
  
  if (m_honeycomb[comb_idx[0]][comb_idx[1]][comb_idx[2]].size() <= m_sufficient_nb_points)
    r=0;
  
  Uint nb_elems = m_source_mesh->get_child_type<CRegion>("regions")->recursive_filtered_elements_count(IsElementsVolume());
  CFinfo << "nb_elems = " << nb_elems << CFendl;
  while (m_pointcloud.size() < m_sufficient_nb_points && m_pointcloud.size() < nb_elems)
  {
    ++r;
    CFinfo << "r = " << r << " prev size = " << m_pointcloud.size() <<  CFendl;
    m_pointcloud.resize(0);

    switch (m_dim)
    {
      case 3:
        for (i = std::max(int(comb_idx[0])-int(r), 0); i <= std::min(int(comb_idx[0])+int(r),int(m_N[0])-1); ++i){
          for (j = std::max(int(comb_idx[1])-int(r), 0); j <= std::min(int(comb_idx[1])+int(r),int(m_N[1])-1); ++j){
            for (k = std::max(int(comb_idx[2])-int(r), 0); k <= std::min(int(comb_idx[2])+int(r),int(m_N[2])-1); ++k)
            {
              BOOST_FOREACH(const Point& point, m_honeycomb[i][j][k])
              m_pointcloud.push_back(&point);
              CFinfo << "   ("<<i<<","<<j<<","<<k<<")" <<  CFendl;
            }
          }
        }
        break;
      case 2:
        for (i = std::max(int(comb_idx[0])-int(r), 0); i <= std::min(int(comb_idx[0])+int(r),int(m_N[0])-1); ++i)
          for (j = std::max(int(comb_idx[1])-int(r), 0); j <= std::min(int(comb_idx[1])+int(r),int(m_N[1])-1); ++j)
          {
            BOOST_FOREACH(const Point& point, m_honeycomb[i][j][k])
            m_pointcloud.push_back(&point);
            CFinfo << "   ("<<i<<","<<j<<","<<k<<")" <<  CFendl;
            
          }
        break;
      case 1:
        for (i = std::max(int(comb_idx[0])-int(r), 0); i <= std::min(int(comb_idx[0])+int(r),int(m_N[0])-1); ++i)
        {
          BOOST_FOREACH(const Point& point, m_honeycomb[i][j][k])
          m_pointcloud.push_back(&point);
          CFinfo << "   ("<<i<<","<<j<<","<<k<<")" <<  CFendl;
          
        }
        break;
    }    
  }
    
  CFinfo << m_pointcloud.size() << " points in the pointcloud " << CFendl;
 
}
  
//////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
