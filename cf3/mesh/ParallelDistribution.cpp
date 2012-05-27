// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "common/OptionList.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/PE/Comm.hpp"

#include "mesh/ParallelDistribution.hpp"

#include "common/OptionList.hpp"

#include "common/OptionList.hpp"

namespace cf3 {
namespace mesh {

  using namespace common;

common::ComponentBuilder < ParallelDistribution, Component, LibMesh > ParallelDistribution_Builder;

//////////////////////////////////////////////////////////////////////////////

ParallelDistribution::ParallelDistribution ( const std::string& name ) :
    Component(name),
    m_nb_obj(0),
    m_base(0),
    m_nb_parts(PE::Comm::instance().size())
{
  options().add("nb_obj", m_nb_obj)
      .description("Total number of objects")
      .pretty_name("Number of Objects")
      .mark_basic();

  options().add("nb_parts", m_nb_parts)
      .description("Total number of partitions (e.g. number of processors)")
      .pretty_name("Number of Partitions");

  options().add("base", m_base)
      .description("Start index for global numbering")
      .pretty_name("Base");

  options()["nb_parts"].link_to( &m_nb_parts );
  options()["nb_obj"].link_to( &m_nb_obj );
  options()["base"].link_to( &m_base );
}

//////////////////////////////////////////////////////////////////////////////

Uint ParallelDistribution::part_of_obj(const Uint obj) const
{
  return std::min(m_nb_parts-1, (obj - m_base) / part_size() );
}

//////////////////////////////////////////////////////////////////////////////

Uint ParallelDistribution::proc_of_part(const Uint part) const
{
  return std::min(PE::Comm::instance().size()-1, part / (m_nb_parts/PE::Comm::instance().size()));
}

//////////////////////////////////////////////////////////////////////////////

Uint ParallelDistribution::part_size() const
{
  return m_nb_obj/m_nb_parts;
}

//////////////////////////////////////////////////////////////////////////////

Uint ParallelDistribution::proc_of_obj(const Uint obj) const
{
  return proc_of_part( part_of_obj(obj) );
}

//////////////////////////////////////////////////////////////////////////////

bool ParallelDistribution::rank_owns(const Uint obj) const
{
  return (proc_of_obj(obj) == PE::Comm::instance().rank());
}

//////////////////////////////////////////////////////////////////////////////

bool ParallelDistribution::part_owns(const Uint part, const Uint obj) const
{
  return (part_of_obj(obj) == part);
}

//////////////////////////////////////////////////////////////////////////////

Uint ParallelDistribution::nb_objects_in_part(const Uint part) const
{
  return end_idx_in_part(part) - start_idx_in_part(part);
}

//////////////////////////////////////////////////////////////////////////////

Uint ParallelDistribution::nb_objects_in_proc(const Uint proc) const
{
  return end_idx_in_proc(proc) - start_idx_in_proc(proc);
}

//////////////////////////////////////////////////////////////////////////////

Uint ParallelDistribution::start_idx_in_part(const Uint part) const
{
  return m_nb_obj/m_nb_parts*part;
}

//////////////////////////////////////////////////////////////////////////////

Uint ParallelDistribution::end_idx_in_part(const Uint part) const
{
  if (part == m_nb_parts - 1)
    return m_nb_obj;
  else
    return start_idx_in_part(part+1);
}

//////////////////////////////////////////////////////////////////////////////

Uint ParallelDistribution::start_idx_in_proc(const Uint proc) const
{
  Uint part_begin = m_nb_parts/PE::Comm::instance().size()*proc;
  return start_idx_in_part(part_begin);
}

//////////////////////////////////////////////////////////////////////////////

Uint ParallelDistribution::end_idx_in_proc(const Uint proc) const
{
  Uint part_end = (proc == PE::Comm::instance().size()-1) ? m_nb_parts : m_nb_parts/PE::Comm::instance().size()*(proc+1);
  return end_idx_in_part(part_end);
}

//////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
