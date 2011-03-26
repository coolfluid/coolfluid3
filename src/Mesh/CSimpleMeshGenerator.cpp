// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionArray.hpp"
#include "Common/OptionURI.hpp"
#include "Common/FindComponents.hpp"
#include "Common/Foreach.hpp"
#include "Common/CreateComponent.hpp"

#include "Mesh/CSimpleMeshGenerator.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CFaces.hpp"
#include "Mesh/CElements.hpp"

namespace CF {
namespace Mesh {

using namespace Common;
using namespace Common::XML;

////////////////////////////////////////////////////////////////////////////////

ComponentBuilder < CSimpleMeshGenerator, CMeshGenerator, LibMesh > CSimpleMeshGenerator_Builder;

////////////////////////////////////////////////////////////////////////////////

CSimpleMeshGenerator::CSimpleMeshGenerator ( const std::string& name  ) :
  CMeshGenerator ( name )
{
  mark_basic();

  properties().add_option(OptionURI::create("parent","Parent","Parent Component",URI()));
  
  properties().add_option<OptionArrayT<Uint> >("nb_cells","Number of Cells","Vector of number of cells in each direction",m_nb_cells)
    ->link_to(&m_nb_cells)
    ->mark_basic();
  properties().add_option<OptionArrayT<Real> >("lengths","Lengths","Vector of lengths each direction",m_lengths)
    ->link_to(&m_lengths)
    ->mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

CSimpleMeshGenerator::~CSimpleMeshGenerator()
{
}

//////////////////////////////////////////////////////////////////////////////

void CSimpleMeshGenerator::execute()
{
  if (m_nb_cells.size() == 1 && m_lengths.size() == 1)
  {
    create_line(m_lengths[0],m_nb_cells[0]);
  }
  else if (m_nb_cells.size() == 2  && m_lengths.size() == 2)
  {
    create_rectangle(m_lengths[0],m_lengths[1],m_nb_cells[0],m_nb_cells[1]);
  }
  else
  {
    throw SetupError(FromHere(),"Invalid size of the vector number of cells. Only 1D and 2D supported now.");
  }
}

////////////////////////////////////////////////////////////////////////////////

void CSimpleMeshGenerator::create_line(const Real x_len, const Uint x_segments)
{
  CMesh& mesh = *m_mesh.lock();
  CRegion& region = mesh.topology().create_region("fluid");
  CNodes& nodes = mesh.topology().create_nodes(DIM_1D);
  nodes.resize(x_segments+1);
  const Real x_step = x_len / static_cast<Real>(x_segments);
  for(Uint i = 0; i <= x_segments; ++i)
  {
    nodes.coordinates()[i][XX] = static_cast<Real>(i) * x_step;
  }
  
  CCells::Ptr cells = region.create_component<CCells>("Line");
  cells->initialize("CF.Mesh.SF.Line1DLagrangeP1",nodes);
  CTable<Uint>& connectivity = cells->connectivity_table();
  connectivity.resize(x_segments);
  for(Uint i = 0; i < x_segments; ++i)
  {
    CTable<Uint>::Row nodes = connectivity[i];
    nodes[0] = i;
    nodes[1] = i+1;
  }
  
  // Left boundary point
  CFaces::Ptr xneg = mesh.topology().create_region("xneg").create_component<CFaces>("Point");
  xneg->initialize("CF.Mesh.SF.Point1DLagrangeP1", nodes);
  CTable<Uint>& xneg_connectivity = xneg->connectivity_table();
  xneg_connectivity.resize(1);
  xneg_connectivity[0][0] = 0;
  
  // right boundary point
  CFaces::Ptr xpos = mesh.topology().create_region("xpos").create_component<CFaces>("Point");
  xpos->initialize("CF.Mesh.SF.Point1DLagrangeP1", nodes);
  CTable<Uint>& xpos_connectivity = xpos->connectivity_table();
  xpos_connectivity.resize(1);
  xpos_connectivity[0][0] = x_segments;
}

////////////////////////////////////////////////////////////////////////////////

void CSimpleMeshGenerator::create_rectangle(const Real x_len, const Real y_len, const Uint x_segments, const Uint y_segments)
{
  CMesh& mesh = *m_mesh.lock();
  CRegion& region = mesh.topology().create_region("region");
  CNodes& nodes = region.create_nodes(DIM_2D);
  nodes.resize((x_segments+1)*(y_segments+1));
  
  const Real x_step = x_len / static_cast<Real>(x_segments);
  const Real y_step = y_len / static_cast<Real>(y_segments);
  Real y;
  for(Uint j = 0; j <= y_segments; ++j)
  {
    y = static_cast<Real>(j) * y_step;
    for(Uint i = 0; i <= x_segments; ++i)
    {
      CTable<Real>::Row row = nodes.coordinates()[j*(x_segments+1)+i];
      row[XX] = static_cast<Real>(i) * x_step;
      row[YY] = y;
    }
  }
  
  CCells::Ptr cells = region.create_component<CCells>("Quad");
  cells->initialize("CF.Mesh.SF.Quad2DLagrangeP1",nodes);
  CTable<Uint>& connectivity = cells->connectivity_table();
  connectivity.resize((x_segments)*(y_segments));
  for(Uint j = 0; j < y_segments; ++j)
  {
    for(Uint i = 0; i < x_segments; ++i)
    {
      CTable<Uint>::Row nodes = connectivity[j*(x_segments)+i];
      nodes[0] = j * (x_segments+1) + i;
      nodes[1] = nodes[0] + 1;
      nodes[3] = (j+1) * (x_segments+1) + i;
      nodes[2] = nodes[3] + 1;
    }
  }
  
  CFaces::Ptr left = mesh.topology().create_region("left").create_component<CFaces>("Line");
  left->initialize("CF.Mesh.SF.Line2DLagrangeP1", nodes);
  CTable<Uint>& left_connectivity = left->connectivity_table();
  left_connectivity.resize(y_segments);
  for(Uint j = 0; j < y_segments; ++j)
  {
    CTable<Uint>::Row crow = left_connectivity[j];
    crow[0] = j * (x_segments+1);
    crow[1] = (j+1) * (x_segments+1);
  }
  
  CFaces::Ptr right = mesh.topology().create_region("right").create_component<CFaces>("Line");
  right->initialize("CF.Mesh.SF.Line2DLagrangeP1", nodes);
  CTable<Uint>& right_connectivity = right->connectivity_table();
  right_connectivity.resize(y_segments);
  for(Uint j = 0; j < y_segments; ++j)
  {
    CTable<Uint>::Row nodes = right_connectivity[j];
    nodes[1] = j * (x_segments+1) + x_segments;
    nodes[0] = (j+1) * (x_segments+1) + x_segments;
  }
  
  CFaces::Ptr bottom = mesh.topology().create_region("bottom").create_component<CFaces>("Line");
  bottom->initialize("CF.Mesh.SF.Line2DLagrangeP1", nodes);
  CTable<Uint>& bottom_connectivity = bottom->connectivity_table();
  bottom_connectivity.resize(x_segments);
  for(Uint i = 0; i < x_segments; ++i)
  {
    CTable<Uint>::Row nodes = bottom_connectivity[i];
    nodes[0] = i;
    nodes[1] = i+1;
  }
  
  CFaces::Ptr top = mesh.topology().create_region("top").create_component<CFaces>("Line");
  top->initialize("CF.Mesh.SF.Line2DLagrangeP1", nodes);
  CTable<Uint>& top_connectivity = top->connectivity_table();
  top_connectivity.resize(x_segments);
  for(Uint i = 0; i < x_segments; ++i)
  {
    CTable<Uint>::Row nodes = top_connectivity[i];
    nodes[1] = y_segments * (x_segments+1) + i;
    nodes[0] = nodes[1] + 1;
  }
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
