// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/MPI/debug.hpp"

#include "Math/MathConsts.hpp"

#include "Mesh/Manipulations.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CElements.hpp"

namespace CF {
namespace Mesh {

using namespace Common;
using namespace Common::mpi;
using namespace Math::MathConsts;

////////////////////////////////////////////////////////////////////////////////

RemoveNodes::RemoveNodes(CNodes& nodes) :
    is_ghost (nodes.is_ghost().create_buffer()),
    glb_idx (nodes.glb_idx().create_buffer()),
    rank (nodes.rank().create_buffer()),
    coordinates (nodes.coordinates().create_buffer()),
    connected_elements (nodes.glb_elem_connectivity().create_buffer())
{}

////////////////////////////////////////////////////////////////////////////////

void RemoveNodes::operator() (const Uint idx)
{
  Uint val = glb_idx.get_row(idx);

  is_ghost.rm_row(idx);
  glb_idx.rm_row(idx);
  rank.rm_row(idx);
  coordinates.rm_row(idx);
  connected_elements.rm_row(idx);

  std::cout << PERank << "removed node  " << val << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

void RemoveNodes::flush()
{
  is_ghost.flush();
  glb_idx.flush();
  rank.flush();
  coordinates.flush();
  connected_elements.flush();
}

////////////////////////////////////////////////////////////////////////////////

RemoveElements::RemoveElements(CElements& elements) :
    glb_idx (elements.glb_idx().create_buffer()),
    rank (elements.rank().create_buffer()),
    connected_nodes (elements.node_connectivity().create_buffer())
{}

////////////////////////////////////////////////////////////////////////////////

void RemoveElements::operator() (const Uint idx)
{
  Uint val = glb_idx.get_row(idx);

  glb_idx.rm_row(idx);
  rank.rm_row(idx);
  connected_nodes.rm_row(idx);

  std::cout << PERank << "removed element  " << val << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

void RemoveElements::flush()
{
  glb_idx.flush();
  connected_nodes.flush();
  rank.flush();
}

////////////////////////////////////////////////////////////////////////////////

PackUnpackElements::PackUnpackElements(CElements& elements, bool remove_after_pack) :
    m_elements(elements),
    m_remove_after_pack(remove_after_pack),
    m_idx(Uint_max()),
    glb_idx (elements.glb_idx().create_buffer()),
    rank (elements.rank().create_buffer()),
    connected_nodes (elements.node_connectivity().create_buffer())
{}

////////////////////////////////////////////////////////////////////////////////

PackUnpackElements& PackUnpackElements::operator() (const Uint idx)
{
  m_idx=idx;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////

void PackUnpackElements::pack(mpi::Buffer& buf)
{
  cf_assert_desc("Must call using  object(idx).pack(buf), instead of object.pack(buf)" , m_idx != Uint_max());

  Uint val = m_elements.glb_idx()[m_idx];

  buf << m_elements.glb_idx()[m_idx]
      << m_elements.rank()[m_idx];

  boost_foreach(const Uint connected_node, m_elements.node_connectivity()[m_idx])
      buf << connected_node;

  if (m_remove_after_pack)
  {
    glb_idx.rm_row(m_idx);
    rank.rm_row(m_idx);
    connected_nodes.rm_row(m_idx);
  }

  if (m_remove_after_pack)
    std::cout << PERank << "packed and removed element    glb_idx = " << val << std::endl;
  else
    std::cout << PERank << "packed element    glb_idx = " << val << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

void PackUnpackElements::unpack(mpi::Buffer& buf)
{
  Uint glb_idx_data;
  Uint rank_data;
  std::vector<Uint> connected_nodes_data(m_elements.node_connectivity().row_size());

  buf >> glb_idx_data >> rank_data;

  for (Uint n=0; n<connected_nodes_data.size(); ++n)
    buf >> connected_nodes_data[n];

  Uint idx, idx_check;
  idx = glb_idx.add_row(glb_idx_data);
  idx_check = rank.add_row(rank_data);
  cf_assert(idx_check == idx);
  idx_check = connected_nodes.add_row(connected_nodes_data);
  cf_assert(idx_check == idx);

  std::cout << PERank << "unpacked and added element    glb_idx = " << glb_idx_data << "\t    rank = " << rank_data << "\t    connected_nodes = " << connected_nodes_data << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

void PackUnpackElements::flush()
{
  glb_idx.flush();
  connected_nodes.flush();
  rank.flush();
}

////////////////////////////////////////////////////////////////////////////////

PackUnpackNodes::PackUnpackNodes(CNodes& nodes, const bool remove_after_pack) :
  m_nodes(nodes),
  m_remove_after_pack(remove_after_pack),
  m_idx(Uint_max()),
  is_ghost (nodes.is_ghost().create_buffer()),
  glb_idx (nodes.glb_idx().create_buffer()),
  rank (nodes.rank().create_buffer()),
  coordinates (nodes.coordinates().create_buffer()),
  connected_elements (nodes.glb_elem_connectivity().create_buffer())
{}

////////////////////////////////////////////////////////////////////////////////

PackUnpackNodes& PackUnpackNodes::operator() (const Uint idx)
{
  m_idx=idx;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////

void PackUnpackNodes::pack(mpi::Buffer& buf)
{
  cf_assert_desc("Must call using  object(idx).pack(buf), instead of object.pack(buf)" , m_idx != Uint_max());

  Uint val = m_nodes.glb_idx()[m_idx];

  buf << m_nodes.glb_idx()[m_idx]
      << m_nodes.rank()[m_idx]
      << m_nodes.coordinates()[m_idx]
      << m_nodes.glb_elem_connectivity()[m_idx];

  if (m_remove_after_pack)
  {
    glb_idx.rm_row(m_idx);
    rank.rm_row(m_idx);
    coordinates.rm_row(m_idx);
    is_ghost.rm_row(m_idx);
    connected_elements.rm_row(m_idx);
  }

  if (m_remove_after_pack)
    std::cout << PERank << "packed and removed node    glb_idx = " << val << std::endl;
  else
    std::cout << PERank << "packed node    glb_idx = " << val << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

void PackUnpackNodes::unpack(mpi::Buffer& buf)
{
  Uint glb_idx_data;
  Uint rank_data;
  std::vector<Real> coordinates_data;
  std::vector<Uint> connected_elems_data;

  buf >> glb_idx_data >> rank_data >> coordinates_data >> connected_elems_data;

  Uint idx;
  idx = glb_idx.add_row(glb_idx_data);
  cf_always_assert(coordinates.add_row(coordinates_data) == idx);
  cf_always_assert(rank.add_row(rank_data) == idx);
  cf_always_assert(connected_elements.add_row(connected_elems_data) == idx);
  cf_always_assert(is_ghost.add_row(rank_data != PE::instance().rank()));

  std::cout << PERank << "added node    glb_idx = " << glb_idx_data << "\t    rank = " << rank_data << "\t    coords = " << coordinates_data << "\t    connected_elem = " << connected_elems_data << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

void PackUnpackNodes::flush()
{
  is_ghost.flush();
  glb_idx.flush();
  rank.flush();
  coordinates.flush();
  connected_elements.flush();
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
