// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "Common/OptionT.hpp"
#include "Common/MPI/PE.hpp"

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
    m_nb_parts(mpi::PE::instance().size())
{
  m_properties.add_option<OptionT <Uint> >("nb_obj","Number of Objects","Total number of objects",m_nb_obj)->mark_basic();
  m_properties.add_option<OptionT <Uint> >("nb_parts","Number of Partitions","Total number of partitions (e.g. number of processors)",m_nb_parts);
  m_properties.add_option<OptionT <Uint> >("base","Base","Start index for global numbering",m_base);

  m_properties["nb_parts"].as_option().link_to( &m_nb_parts );
  m_properties["nb_obj"].as_option().link_to( &m_nb_obj );
  m_properties["base"].as_option().link_to( &m_base );
}

//////////////////////////////////////////////////////////////////////////////

Uint CHash::part_of_obj(const Uint obj) const
{
  return std::min(m_nb_parts-1, (obj - m_base) / part_size() );
}

//////////////////////////////////////////////////////////////////////////////

Uint CHash::proc_of_part(const Uint part) const
{
  return std::min(mpi::PE::instance().size()-1, part / (m_nb_parts/mpi::PE::instance().size()));
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
  return (proc_of_obj(obj) == mpi::PE::instance().rank());
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
	Uint part_begin = m_nb_parts/mpi::PE::instance().size()*proc;
	return start_idx_in_part(part_begin);
}

//////////////////////////////////////////////////////////////////////////////

Uint CHash::end_idx_in_proc(const Uint proc) const
{
  Uint part_end = (proc == mpi::PE::instance().size()-1) ? m_nb_parts : m_nb_parts/mpi::PE::instance().size()*(proc+1);
  return end_idx_in_part(part_end);
}

//////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
