// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/PE/debug.hpp"

#include "math/Consts.hpp"

#include "mesh/Manipulations.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/Space.hpp"

namespace cf3 {
namespace mesh {

using namespace common;
using namespace common::PE;
using namespace math::Consts;

////////////////////////////////////////////////////////////////////////////////

RemoveNodes::RemoveNodes(Dictionary& nodes) :
    glb_idx (nodes.glb_idx().create_buffer()),
    rank (nodes.rank().create_buffer()),
    coordinates (nodes.coordinates().create_buffer()),
    connected_elements (nodes.glb_elem_connectivity().create_buffer())
{}

////////////////////////////////////////////////////////////////////////////////

void RemoveNodes::operator() (const Uint idx)
{
  Uint val = glb_idx.get_row(idx);

  glb_idx.rm_row(idx);
  rank.rm_row(idx);
  coordinates.rm_row(idx);
  connected_elements.rm_row(idx);

//  std::cout << PERank << "removed node  " << val << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

void RemoveNodes::flush()
{
  glb_idx.flush();
  rank.flush();
  coordinates.flush();
  connected_elements.flush();
}

////////////////////////////////////////////////////////////////////////////////

RemoveElements::RemoveElements(Elements& elements) :
    glb_idx (elements.glb_idx().create_buffer()),
    rank (elements.rank().create_buffer()),
    connected_nodes (elements.geometry_space().connectivity().create_buffer())
{}

////////////////////////////////////////////////////////////////////////////////

void RemoveElements::operator() (const Uint idx)
{
  Uint val = glb_idx.get_row(idx);

  glb_idx.rm_row(idx);
  rank.rm_row(idx);
  connected_nodes.rm_row(idx);

//  std::cout << PERank << "removed element  " << val << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

void RemoveElements::flush()
{
  glb_idx.flush();
  rank.flush();
  connected_nodes.flush();
}

////////////////////////////////////////////////////////////////////////////////

PackUnpackElements::PackUnpackElements(Elements& elements) :
    m_elements(elements),
    m_remove_after_pack(false),
    m_idx(uint_max()),
    glb_idx (elements.glb_idx().create_buffer()),
    rank (elements.rank().create_buffer()),
    connected_nodes (elements.geometry_space().connectivity().create_buffer())
{}

////////////////////////////////////////////////////////////////////////////////

PackUnpackElements& PackUnpackElements::operator() (const Uint idx, const bool remove_after_pack)
{
  m_idx=idx;
  m_remove_after_pack = remove_after_pack;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////

void PackUnpackElements::remove(const Uint idx)
{
  Uint val = glb_idx.get_row(idx);

  glb_idx.rm_row(idx);
  rank.rm_row(idx);
  connected_nodes.rm_row(idx);

  //std::cout << PERank << "removed element  " << val << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

void PackUnpackElements::pack(PE::Buffer& buf)
{
  cf3_assert_desc("Must call using  object(idx).pack(buf), instead of object.pack(buf)" , m_idx != uint_max());

  Uint val = m_elements.glb_idx()[m_idx];

  buf << m_elements.glb_idx()[m_idx]
      << m_elements.rank()[m_idx];

  boost_foreach(const Uint connected_node, m_elements.geometry_space().connectivity()[m_idx])
      buf << connected_node;

  //std::cout << PERank << "packed element    glb_idx = " << val << std::endl;

  if (m_remove_after_pack)
    remove(m_idx);

}

////////////////////////////////////////////////////////////////////////////////

void PackUnpackElements::unpack(PE::Buffer& buf)
{
  Uint glb_idx_data;
  Uint rank_data;
  std::vector<Uint> connected_nodes_data(m_elements.geometry_space().connectivity().row_size());

  buf >> glb_idx_data >> rank_data;

  for (Uint n=0; n<connected_nodes_data.size(); ++n)
    buf >> connected_nodes_data[n];

  Uint idx;
  idx = glb_idx.add_row(glb_idx_data);
  cf3_always_assert(rank.add_row(rank_data) == idx);
  cf3_always_assert(connected_nodes.add_row(connected_nodes_data) == idx);

  // std::cout << PERank << "unpacked and added element    glb_idx = " << glb_idx_data << "\t    rank = " << rank_data << "\t    connected_nodes = " << connected_nodes_data << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

void PackUnpackElements::flush()
{
  glb_idx.flush();
  connected_nodes.flush();
  rank.flush();
}

////////////////////////////////////////////////////////////////////////////////

PackUnpackNodes::PackUnpackNodes(Dictionary& nodes) :
  m_nodes(nodes),
  m_remove_after_pack(false),
  m_idx(uint_max()),
  glb_idx (nodes.glb_idx().create_buffer(100)),
  rank (nodes.rank().create_buffer(100)),
  coordinates (nodes.coordinates().create_buffer(100)),
  connected_elements (nodes.glb_elem_connectivity().create_buffer(100))
{}

////////////////////////////////////////////////////////////////////////////////

PackUnpackNodes& PackUnpackNodes::operator() (const Uint idx, const bool remove_after_pack)
{
  m_idx=idx;
  m_remove_after_pack = remove_after_pack;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////

void PackUnpackNodes::remove(const Uint idx)
{
  Uint val = glb_idx.get_row(idx);

  cf3_assert(idx < m_nodes.size());

  glb_idx.rm_row(idx);
  rank.rm_row(idx);
  coordinates.rm_row(idx);
  connected_elements.rm_row(idx);

  m_idx = uint_max();
}

////////////////////////////////////////////////////////////////////////////////

void PackUnpackNodes::pack(PE::Buffer& buf)
{
  cf3_assert_desc("Must call using  object(idx).pack(buf), instead of object.pack(buf)" , m_idx != uint_max());

  cf3_assert_desc("["+to_str(m_idx)+">="+to_str(m_nodes.size())+"]",m_idx < m_nodes.size());
  cf3_assert(m_idx < m_nodes.glb_idx().size());
  cf3_assert(m_idx < m_nodes.rank().size());
  cf3_assert(m_idx < m_nodes.coordinates().size());
  cf3_assert(m_idx < m_nodes.glb_elem_connectivity().size());


  Uint val = m_nodes.glb_idx()[m_idx];

  buf << m_nodes.glb_idx()[m_idx];

  buf << m_nodes.rank()[m_idx];

  buf << m_nodes.coordinates()[m_idx];

  buf << m_nodes.glb_elem_connectivity()[m_idx];

//  std::cout << PERank << "packed node    glb_idx = " << val << std::endl;

  if (m_remove_after_pack)
    remove(m_idx);

  m_idx = uint_max();
}

////////////////////////////////////////////////////////////////////////////////

void PackUnpackNodes::unpack(PE::Buffer& buf)
{
  Uint glb_idx_data;
  Uint rank_data;
  std::vector<Real> coordinates_data;
  std::vector<Uint> connected_elems_data;

  buf >> glb_idx_data >> rank_data >> coordinates_data >> connected_elems_data;

  Uint idx;
             idx = glb_idx.add_row(glb_idx_data);
  cf3_always_assert(rank.add_row(rank_data) == idx);
  cf3_always_assert(coordinates.add_row(coordinates_data) == idx);
  cf3_always_assert(connected_elements.add_row(connected_elems_data) == idx);

//  std::cout << PERank << "added node    glb_idx = " << glb_idx_data << "\t    rank = " << rank_data << "\t    coords = " << to_str(coordinates_data) << "\t    connected_elem = " << to_str(connected_elems_data) << std::endl;
  m_idx = uint_max();
}

////////////////////////////////////////////////////////////////////////////////

void PackUnpackNodes::flush()
{
  glb_idx.flush();
  rank.flush();
  coordinates.flush();
  connected_elements.flush();
  m_nodes.resize(m_nodes.coordinates().size());
  m_idx = uint_max();
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
