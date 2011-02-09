// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CField2.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CList.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CEntities.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/CSpace.hpp"

namespace CF {
namespace Mesh {

Common::ComponentBuilder < CFieldView, CFieldView, LibMesh >  CFieldView_Builder;
Common::ComponentBuilder < CScalarFieldView, CFieldView, LibMesh >  CScalarFieldView_Builder;
Common::ComponentBuilder < CMultiStateFieldView, CFieldView, LibMesh >  CMultiStateFieldView_Builder;

////////////////////////////////////////////////////////////////////////////////

CFieldView::CFieldView ( const std::string& name ) : 
  Common::Component (name),
    m_size(0),
    m_start_idx(0),
    m_end_idx(0),
    m_stride(0)
{
}

////////////////////////////////////////////////////////////////////////////////

Uint CFieldView::initialize(CField2& field, CEntities::Ptr elements)
{
  cf_assert(is_not_null(elements));
  set_field(field);  
  set_elements(elements);
  return m_end_idx;
}

////////////////////////////////////////////////////////////////////////////////

CTable<Real>::Row CFieldView::operator[](const Uint idx)
{
  cf_assert( idx < m_size );
  Uint data_idx = m_start_idx+idx;
  return m_field_data.lock()->array()[data_idx];
}

////////////////////////////////////////////////////////////////////////////////

CTable<Real>::ConstRow CFieldView::operator[](const Uint idx) const
{
  cf_assert( idx < m_size );
  Uint data_idx = m_start_idx+idx;
  return m_field_data.lock()->array()[data_idx];
}

////////////////////////////////////////////////////////////////////////////////

bool CFieldView::set_elements(const CEntities& elements) 
{ 
  cf_assert_desc("Field must be set before elements", is_not_null(m_field.lock()) );
  const CField2& field = *m_field.lock();
  m_elements = elements.as_const()->as_type<CEntities>();
  if (field.exists_for_entities(elements))
  {
    m_space = elements.space(field.space_idx()).as_type<CSpace>();
    m_stride = m_space.lock()->nb_states(); // this is the number of states per element (high order methods)
    m_start_idx = field.elements_start_idx(elements);
    m_end_idx = m_start_idx + m_stride * elements.size();
    m_size = m_end_idx - m_start_idx;
    m_coords_table = elements.nodes().coordinates().as_type<CTable<Real> >();
    return true;
  }
  else
  {
    m_size = 0;
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool CFieldView::set_elements(CEntities::Ptr elements)
{
  cf_assert( is_not_null(m_field.lock()) );
  return set_elements(*elements);
}

////////////////////////////////////////////////////////////////////////////////

void CFieldView::set_field(CField2& field) 
{ 
  m_field = field.as_type<CField2>(); 
  m_field_data = field.data().as_type<CTable<Real> >();
  cf_assert( is_not_null(m_field_data.lock()) );
}

////////////////////////////////////////////////////////////////////////////////

void CFieldView::set_field(CField2::Ptr field) 
{ 
  set_field(*field);
}

////////////////////////////////////////////////////////////////////////////////

void CFieldView::allocate_coordinates(RealMatrix& coords)
{
  cf_assert( !m_space.expired() );
  coords.resize(space().shape_function().nb_nodes(),space().shape_function().dimension());
}
  
////////////////////////////////////////////////////////////////////////////////

void CFieldView::put_coordinates(RealMatrix& coords, const Uint elem_idx) const
{
  cf_assert(elem_idx < space().connectivity_table().size());
  CTable<Uint>::ConstRow elem_nodes = space().connectivity_table()[elem_idx];
  const CTable<Real>::ArrayT& coords_table = m_coords_table.lock()->array();
    
  cf_assert(coords.rows() == elem_nodes.size());
  cf_assert(coords.cols() == coords_table.shape()[1]);
  for(Uint node = 0; node != coords.rows(); ++node)
    for (Uint d=0; d != coords.cols(); ++d)
      coords(node,d) = coords_table[elem_nodes[node]][d];
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CMultiStateFieldView::CMultiStateFieldView ( const std::string& name ) 
  : CFieldView (name) 
{
}

////////////////////////////////////////////////////////////////////////////////
  
CMultiStateFieldView::View CMultiStateFieldView::operator[](const Uint idx)
{
  cf_assert( idx < m_size );    
  Uint data_idx = m_start_idx+idx;
  Range range = Range().start(data_idx).finish(data_idx + m_stride);
  cf_assert( is_not_null(m_field_data.lock()) );
  return m_field_data.lock()->array()[ boost::indices[range][Range()] ];
}

////////////////////////////////////////////////////////////////////////////////

CMultiStateFieldView::ConstView CMultiStateFieldView::operator[](const Uint idx) const
{
  cf_assert( idx < m_size );    
  Uint data_idx = m_start_idx+idx;
  Range range = Range().start(data_idx).finish(data_idx + m_stride);
  cf_assert( is_not_null(m_field_data.lock()) );
  return m_field_data.lock()->array()[ boost::indices[range][Range()] ];
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CScalarFieldView::CScalarFieldView ( const std::string& name )
  : CFieldView (name) 
{
}

////////////////////////////////////////////////////////////////////////////////

Real& CScalarFieldView::operator[](const Uint idx)
{
  cf_assert( idx < m_size );    
  Uint data_idx = m_start_idx+idx;
  cf_assert( is_not_null(m_field_data.lock()) );
  return m_field_data.lock()->array()[data_idx][0];
}

////////////////////////////////////////////////////////////////////////////////

const Real& CScalarFieldView::operator[](const Uint idx) const
{
  cf_assert( idx < m_size );    
  Uint data_idx = m_start_idx+idx;
  cf_assert( is_not_null(m_field_data.lock()) );
  return m_field_data.lock()->array()[data_idx][0];
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
