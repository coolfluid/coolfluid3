#include <boost/foreach.hpp>
#include <boost/algorithm/string/erase.hpp>
#include "Common/ObjectProvider.hpp"
#include "Common/ComponentPredicates.hpp"
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
: CInterpolator(name)
{
  BUILD_COMPONENT;
}

//////////////////////////////////////////////////////////////////////////////

void CHoneycombInterpolator::interpolate_from_to(const CMesh::Ptr& source, const CMesh::Ptr& target)
{
  m_source = source;
  m_target = target;

  create_honeycomb();
}
  
//////////////////////////////////////////////////////////////////////

void CHoneycombInterpolator::create_honeycomb()
{
  Uint dim = 0;
  std::set<const CArray*> all_coordinates;
  BOOST_FOREACH(CElements& elements, recursive_range_typed<CElements>(*m_source))
  { 
    dim = std::max(elements.element_type().dimensionality() , dim);
    all_coordinates.insert(&elements.coordinates());
  }
  
  std::vector<Real> L(3);
  std::vector<RealVector> ranges(3);
  for (Uint d=0; d<dim; ++d)
  {
    ranges[d].resize(2,0.0);
  }
      
  Real V=1;
  BOOST_FOREACH(const CArray* coordinates , all_coordinates)
  {
    BOOST_FOREACH(const CArray::ConstRow& node, coordinates->array())
    {
      for (Uint d=0; d<dim; ++d)
      {
        ranges[d][0] = std::min(ranges[d][0],  node[d]);
        ranges[d][1] = std::max(ranges[d][1],  node[d]);
      }
    }
  }
  for (Uint d=0; d<dim; ++d)
  {

    L[d] = ranges[d][1] - ranges[d][0];
    V*=L[d];
  }
  
  Uint nb_elems = m_source->get_child_type<CRegion>("regions")->recursive_filtered_elements_count(IsElementsVolume());
  Real V1 = V/nb_elems;
  Real D1 = std::pow(V1,1./dim);
  
  
  std::vector<Uint> N(3);
  std::vector<Real> D(3);
  std::vector<Uint> comb_idx(3);
  for (Uint d=0; d<dim; ++d)
  {
    N[d] = std::ceil(L[d]/D1);
    //N[d] = std::ceil(ranges[d][1]-ranges[d][0])/2.;
    D[d] = (L[d])/static_cast<Real>(N[d]);
  }
  
  for (Uint d=0; d<dim; ++d)
  {
    CFinfo << "range["<<d<<"] :   L = " << L[d] << "    N = " << N[d] << "    D = " << D[d] << CFendl;
  }
  CFinfo << "V = " << V << CFendl;
  
  // initialize the honeycomb
  m_honeycomb.resize(boost::extents[std::max(Uint(1),N[0])][std::max(Uint(1),N[1])][std::max(Uint(1),N[2])]);
  
  
  Uint total_nb_elems=0;
  BOOST_FOREACH(const CElements& elements, recursive_filtered_range_typed<CElements>(*m_source,IsElementsVolume()))
  {
    const CArray& coordinates = elements.coordinates();
    Uint nb_nodes_per_element = elements.connectivity_table().table().shape()[1];
    Uint elem_idx=0;
    BOOST_FOREACH(const CTable::ConstRow& elem, elements.connectivity_table().table())
    {
      RealVector centroid(0.0,dim);
      BOOST_FOREACH(const Uint node_idx, elem)
        centroid += RealVector(coordinates[node_idx]);        

      centroid /= static_cast<Real>(nb_nodes_per_element);
      for (Uint d=0; d<dim; ++d)
        comb_idx[d]=std::min((Uint) std::floor( (centroid[d] - ranges[d][0])/D[d]), N[d]-1 );
      m_honeycomb[comb_idx[0]][comb_idx[1]][comb_idx[2]].push_back(std::make_pair<const CElements*,Uint>(&elements,elem_idx));
      elem_idx++;
    }
    total_nb_elems += elem_idx;
  }
  
  
  Uint total=0;
  
  for (Uint i=0; i<N[0]; ++i)
  for (Uint j=0; j<N[1]; ++j)
  //for (Uint k=0; k<N[2]; ++k)
  {
    Uint k=0;
    CFinfo << "("<<i<<","<<j<<","<<k<<") has " << m_honeycomb[i][j][k].size() << " elems" << CFendl;
    total += m_honeycomb[i][j][k].size();
  }
  CFinfo << "total = " << total << " of " << nb_elems << CFendl;

  
}

//////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
