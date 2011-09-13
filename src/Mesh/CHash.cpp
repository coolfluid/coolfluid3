// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "Common/OptionT.hpp"
#include "Common/PE/Comm.hpp"

#include "Mesh/CHash.hpp"

namespace CF {
namespace Mesh {

  using namespace Common;

Common::ComponentBuilder < CHash, Component, LibMesh > CHash_Builder;

//////////////////////////////////////////////////////////////////////////////

CHash::CHash ( const std::string& name ) :
    Component(name),
    m_nb_obj(0),
    m_base(0),
    m_nb_parts(PE::Comm::instance().size())
{
  m_options.add_option<OptionT <Uint> >("nb_obj", m_nb_obj)
      ->description("Total number of objects")
      ->pretty_name("Number of Objects")
      ->mark_basic();

  m_options.add_option<OptionT <Uint> >("nb_parts", m_nb_parts)
      ->description("Total number of partitions (e.g. number of processors)")
      ->pretty_name("Number of Partitions");

  m_options.add_option<OptionT <Uint> >("base", m_base)
      ->description("Start index for global numbering")
      ->pretty_name("Base");

  m_options["nb_parts"].link_to( &m_nb_parts );
  m_options["nb_obj"].link_to( &m_nb_obj );
  m_options["base"].link_to( &m_base );
}

//////////////////////////////////////////////////////////////////////////////

Uint CHash::part_of_obj(const Uint obj) const
{
  return std::min(m_nb_parts-1, (obj - m_base) / part_size() );
}

//////////////////////////////////////////////////////////////////////////////

Uint CHash::proc_of_part(const Uint part) const
{
  return std::min(PE::Comm::instance().size()-1, part / (m_nb_parts/PE::Comm::instance().size()));
}

//////////////////////////////////////////////////////////////////////////////

Uint CHash::part_size() const
{
  return m_nb_obj/m_nb_parts;
}

//////////////////////////////////////////////////////////////////////////////

Uint CHash::proc_of_obj(const Uint obj) const
{
  return proc_of_part( part_of_obj(obj) );
}

//////////////////////////////////////////////////////////////////////////////

bool CHash::owns(const Uint obj) const
{
  return (proc_of_obj(obj) == PE::Comm::instance().rank());
}

//////////////////////////////////////////////////////////////////////////////

Uint CHash::nb_objects_in_part(const Uint part) const
{
  return end_idx_in_part(part) - start_idx_in_part(part);
}

//////////////////////////////////////////////////////////////////////////////

Uint CHash::nb_objects_in_proc(const Uint proc) const
{
  return end_idx_in_proc(proc) - start_idx_in_proc(proc);
}

//////////////////////////////////////////////////////////////////////////////

Uint CHash::start_idx_in_part(const Uint part) const
{
  return m_nb_obj/m_nb_parts*part;
}

//////////////////////////////////////////////////////////////////////////////

Uint CHash::end_idx_in_part(const Uint part) const
{
  if (part == m_nb_parts - 1)
    return m_nb_obj;
  else
    return start_idx_in_part(part+1);
}

//////////////////////////////////////////////////////////////////////////////

Uint CHash::start_idx_in_proc(const Uint proc) const
{
  Uint part_begin = m_nb_parts/PE::Comm::instance().size()*proc;
  return start_idx_in_part(part_begin);
}

//////////////////////////////////////////////////////////////////////////////

Uint CHash::end_idx_in_proc(const Uint proc) const
{
  Uint part_end = (proc == PE::Comm::instance().size()-1) ? m_nb_parts : m_nb_parts/PE::Comm::instance().size()*(proc+1);
  return end_idx_in_part(part_end);
}

//////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
