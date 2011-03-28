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
#include "Common/FindComponents.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionArray.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/CLink.hpp"

#include "Math/MathConsts.hpp"
#include "Mesh/COcttree.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CField.hpp"
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

CF::Common::ComponentBuilder < COcttree, Component, LibMesh > COcttree_Builder;

//////////////////////////////////////////////////////////////////////////////

COcttree::COcttree( const std::string& name )
  : Component(name), m_dim(0), m_bounding(2), m_N(3), m_D(3), m_octtree_idx(3)
{
  
  m_properties.add_option(OptionComponent<CMesh>::create("mesh","Mesh","Mesh to create octtree from",&m_mesh))
    ->mark_basic();

  m_properties.add_option< OptionT<Uint> >
  ( "nb_elems_per_cell","Number of Elements per Octtree Cell",
    "The approximate amount of elements that are stored in a structured cell of the octtree" ,
    1 );

  std::vector<Uint> dummy;
  m_properties.add_option< OptionArrayT<Uint> >
  ( "nb_cells","Number of Cells"
    "The number of cells in each direction of the comb. Takes precedence over \"Number of Elements per Octtree Cell\". " ,
    dummy);
    
  m_elements = create_component<CUnifiedData<CElements const> >("elements");
  
}

//////////////////////////////////////////////////////////////////////

void COcttree::create_bounding_box()
{
  m_dim=0;

  if (m_mesh.expired())
    throw SetupError(FromHere(), "Option \"mesh\" has not been configured");

  m_dim = m_mesh.lock()->nodes().coordinates().row_size();
  
  // find bounding box coordinates for region 1 and region 2
  m_bounding[MIN].setConstant(Real_max());    
  m_bounding[MAX].setConstant(Real_min());
    
  boost_foreach(CTable<Real>::ConstRow coords, m_mesh.lock()->nodes().coordinates().array())
  {
    for (Uint d=0; d<m_dim; ++d)
    {
      m_bounding[MIN][d] = std::min(m_bounding[MIN][d],  coords[d]);
      m_bounding[MAX][d] = std::max(m_bounding[MAX][d],  coords[d]);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void COcttree::create_octtree()
{
  create_bounding_box();
  if (m_mesh.expired())
    throw SetupError(FromHere(), "Option \"mesh\" has not been configured");

  std::vector<Real> L(3);

  Real V=1;
  for (Uint d=0; d<m_dim; ++d)
  {
    L[d] = m_bounding[MAX][d] - m_bounding[MIN][d];
    V*=L[d];
  }

  const Uint nb_elems = m_mesh.lock()->topology().recursive_filtered_elements_count(IsElementsVolume());

  if (property("nb_cells").value<std::vector<Uint> >().size() > 0)
  {
    m_N = property("nb_cells").value<std::vector<Uint> >();
    for (Uint d=0; d<m_dim; ++d)
      m_D[d] = (L[d])/static_cast<Real>(m_N[d]);
  }
  else
  {
    Real V1 = V/nb_elems;
    Real D1 = std::pow(V1,1./m_dim)*property("nb_elems_per_cell").value<Uint>();

    for (Uint d=0; d<m_dim; ++d)
    {
      m_N[d] = (Uint) std::ceil(L[d]/D1);
      m_D[d] = (L[d])/static_cast<Real>(m_N[d]);
    }
  }

  CFinfo << "Octtree:" << CFendl;
  CFinfo << "--------" << CFendl;
  for (Uint d=0; d<m_dim; ++d)
  {
    CFinfo << "range["<<d<<"] :   L = " << L[d] << "    N = " << m_N[d] << "    D = " << m_D[d] << CFendl;
  }
  CFinfo << "V = " << V << CFendl;

  // initialize the honeycomb
  m_octtree.resize(boost::extents[std::max(Uint(1),m_N[XX])][std::max(Uint(1),m_N[YY])][std::max(Uint(1),m_N[ZZ])]);

  std::vector<CElements::ConstPtr> elements_vector = find_components_recursively_with_filter<CElements>(*m_mesh.lock(),IsElementsVolume()).as_const_vector();
  m_elements->add_data(elements_vector);
  
  Uint unif_elem_idx=0;
  RealVector centroid(m_dim);
  std::vector<Uint> octtree_idx(3);
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
        octtree_idx[d]=std::min((Uint) std::floor( (centroid[d] - m_bounding[MIN][d])/m_D[d]), m_N[d]-1 );
      m_octtree[octtree_idx[XX]][octtree_idx[YY]][octtree_idx[ZZ]].push_back(unif_elem_idx);
      ++unif_elem_idx;
    }
  }


  // Uint total=0;
  // 
  // switch (m_dim)
  // {
  //   case DIM_2D:
  //     for (Uint i=0; i<m_N[0]; ++i)
  //       for (Uint j=0; j<m_N[1]; ++j)
  //       {
  //         Uint k=0;
  //         // CFinfo << "("<<i<<","<<j<<") has elems ";
  //         // if (m_octtree[i][j][k].size())
  //         //   CFinfo << Mesh::to_vector(m_octtree[i][j][k]).transpose() << CFendl;
  //         // else
  //         //   CFinfo << CFendl;
  //         total += m_octtree[i][j][k].size();
  //       }
  //     break;
  //   case DIM_3D:
  //     for (Uint i=0; i<m_N[0]; ++i)
  //       for (Uint j=0; j<m_N[1]; ++j)
  //         for (Uint k=0; k<m_N[2]; ++k)
  //         {
  //           // CFinfo << "("<<i<<","<<j<<","<<k<<") has elems ";
  //           // if (m_octtree[i][j][k].size())
  //           //   CFinfo << Mesh::to_vector(m_octtree[i][j][k]).transpose() << CFendl;
  //           // else
  //           //   CFinfo << CFendl;
  //           total += m_octtree[i][j][k].size();
  //         }
  //     break;
  //   default:
  //     break;
  // }
  // 
  // CFinfo << "total = " << total << " of " << m_nb_elems << CFendl;
}


//////////////////////////////////////////////////////////////////////////////
  
bool COcttree::find_octtree_cell(const RealVector& coordinate, std::vector<Uint>& octtree_idx)
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
    octtree_idx[d] = std::min((Uint) std::floor( (coordinate[d] - m_bounding[MIN][d])/m_D[d]), m_N[d]-1 );
  }
      
  //CFinfo << " should be in box ("<<m_point_idx[0]<<","<<m_point_idx[1]<<","<<m_point_idx[2]<<")" << CFendl;
  return true;
}

//////////////////////////////////////////////////////////////////////////////

void COcttree::gather_elements_around_idx(const std::vector<Uint>& octtree_idx, const Uint ring, std::vector<Uint>& unified_elems)
{
  int i(0), j(0), k(0);
  int imin, jmin, kmin;
  int imax, jmax, kmax;

  int irmin, jrmin, krmin;
  int irmax, jrmax, krmax;
  
  if (ring == 0)
  {
    boost_foreach(const Uint unif_elem_idx, m_octtree[octtree_idx[XX]][octtree_idx[YY]][octtree_idx[ZZ]])
      unified_elems.push_back(unif_elem_idx);
    return;
  }
  else
  {
    switch (m_dim)
    {
      case DIM_3D:
      irmin = int(octtree_idx[XX])-int(ring);  irmax = int(octtree_idx[XX])+int(ring);
      jrmin = int(octtree_idx[YY])-int(ring);  jrmax = int(octtree_idx[YY])+int(ring);
      krmin = int(octtree_idx[ZZ])-int(ring);  krmax = int(octtree_idx[ZZ])+int(ring);

      imin = std::max(irmin, 0);  imax = std::min(irmax,int(m_N[XX])-1);
      jmin = std::max(jrmin, 0);  jmax = std::min(jrmax,int(m_N[YY])-1);
      kmin = std::max(krmin, 0);  kmax = std::min(krmax,int(m_N[ZZ])-1);

        // imin:
        i = imin;
        for (i = imin; i <= imax; ++i)
        {
          for (j = jmin; j <= jmax; ++j)
          {
            for (k = kmin; k <= kmax; ++k)
            {
              if ( i == irmin || i == irmax || j == jrmin || j == jrmax || k == krmin || k == krmax)
              {              
                boost_foreach(const Uint unif_elem_idx, m_octtree[i][j][k])
                  unified_elems.push_back(unif_elem_idx);
              }
            }
          }
        }

        break;
      case DIM_2D:
      
        irmin = int(octtree_idx[XX])-int(ring);  irmax = int(octtree_idx[XX])+int(ring);
        jrmin = int(octtree_idx[YY])-int(ring);  jrmax = int(octtree_idx[YY])+int(ring);

        imin = std::max(irmin, 0);  imax = std::min(irmax,int(m_N[XX])-1);
        jmin = std::max(jrmin, 0);  jmax = std::min(jrmax,int(m_N[YY])-1);

        for (i = imin; i <= imax; ++i)
        {
          for (j = jmin; j <= jmax; ++j)
          {
            if ( i == irmin || i == irmax || j == jrmin || j == jrmax )
            {
              boost_foreach(const Uint unif_elem_idx, m_octtree[i][j][k])
                unified_elems.push_back(unif_elem_idx);
            }
          }
        }
        
        break;
        
      case DIM_1D:
        irmin = int(octtree_idx[XX])-int(ring);  irmax = int(octtree_idx[XX])+int(ring);
        imin = std::max(int(octtree_idx[XX])-int(ring), 0);  imax = std::min(int(octtree_idx[XX])+int(ring),int(m_N[XX])-1);

        for (i = imin; i <= imax; ++i)
        {
          if ( i == irmin || i == irmax)
          {
            boost_foreach(const Uint unif_elem_idx, m_octtree[i][j][k])
              unified_elems.push_back(unif_elem_idx);
          }
        }
        

        break;
    }
  }

    
  //CFinfo << m_pointcloud.size() << " points in the pointcloud " << CFendl;
 
}
  
//////////////////////////////////////////////////////////////////////////////

boost::tuple<CElements::ConstPtr,Uint> COcttree::find_element(const RealVector& target_coord)
{
  if (find_octtree_cell(target_coord,m_octtree_idx))
  {
    std::vector<Uint> unified_elements(0); unified_elements.reserve(16);

    CElements::ConstPtr elements;
    Uint elem_idx;
    
    gather_elements_around_idx(m_octtree_idx,0,unified_elements);
    
    boost_foreach(const Uint unif_elem_idx, unified_elements)
    {
      boost::tie(elements,elem_idx)=m_elements->data_location(unif_elem_idx);
      
      RealMatrix elem_coordinates = elements->get_coordinates(elem_idx);
      if (elements->element_type().is_coord_in_element(target_coord,elem_coordinates))
      {
        return boost::make_tuple(elements,elem_idx);
      }
    }
    
    // if arrived here, it means no element has been found. Enlarge the search with one more ring, for possible misses.
    unified_elements.resize(0); unified_elements.reserve(16);
    gather_elements_around_idx(m_octtree_idx,1,unified_elements);
    
    boost_foreach(const Uint unif_elem_idx, unified_elements)
    {
      boost::tie(elements,elem_idx)=m_elements->data_location(unif_elem_idx);
      
      RealMatrix elem_coordinates = elements->get_coordinates(elem_idx);
      if (elements->element_type().is_coord_in_element(target_coord,elem_coordinates))
      {
        return boost::make_tuple(elements,elem_idx);
      }
    }
    
  }
  // if arrived here, it means no element has been found. Give up.
  return boost::make_tuple(CElements::ConstPtr(), 0u);
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
