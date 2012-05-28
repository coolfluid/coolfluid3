// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Builder.hpp"

#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"
#include "common/PE/Comm.hpp"
#include "common/Foreach.hpp"
#include "common/StringConversion.hpp"
#include "common/Log.hpp"

#include "mesh/MergedParallelDistribution.hpp"
#include "mesh/ParallelDistribution.hpp"

namespace cf3 {
namespace mesh {

  using namespace common;

common::ComponentBuilder < MergedParallelDistribution, Component, LibMesh > MergedParallelDistribution_Builder;

//////////////////////////////////////////////////////////////////////////////

MergedParallelDistribution::MergedParallelDistribution ( const std::string& name ) :
    Component(name),
    m_nb_obj(),
    m_base(0),
    m_nb_parts(PE::Comm::instance().size())
{
  options().add("nb_obj", m_nb_obj)
      .description("Total number of objects of each subhash. Subhashes will "
                        "be created upon configuration with names hash_0 hash_1, ...")
      .pretty_name("Number of Objects");

  options().add("nb_parts", m_nb_parts)
      .description("Total number of partitions (e.g. number of processors)")
      .pretty_name("Number of Partitions");

  options().add("base", m_base)
      .description("Start index for global numbering")
      .pretty_name("Base");

  options()["nb_parts"].attach_trigger( boost::bind ( &MergedParallelDistribution::config_nb_parts , this) );
  options()["base"].link_to( &m_base );
  options()["nb_obj"].attach_trigger( boost::bind ( &MergedParallelDistribution::config_nb_obj , this) );
}

//////////////////////////////////////////////////////////////////////////////

void MergedParallelDistribution::config_nb_obj ()
{
  m_nb_obj = options().value< std::vector<Uint> >("nb_obj");
  boost_foreach(Handle< ParallelDistribution > hash, m_subhash)
    remove_component(hash->name());
  m_subhash.resize(0);
  Uint i=0;
  boost_foreach(Uint nb_obj, m_nb_obj)
  {
    Handle<ParallelDistribution> hash = create_component<ParallelDistribution>("hash_"+to_str(i));
    m_subhash.push_back(hash);
    hash->options().set("nb_obj", nb_obj);
    hash->options().set("nb_parts", m_nb_parts);
    ++i;
  }
}

//////////////////////////////////////////////////////////////////////////////

void MergedParallelDistribution::config_nb_parts ()
{
  m_nb_parts = options().value<Uint>("nb_parts");
  if (m_subhash.size())
  {
    boost_foreach(Handle< ParallelDistribution > hash, m_subhash)
      hash->options().set("nb_parts", m_nb_parts);
  }
}

//////////////////////////////////////////////////////////////////////////////

Uint MergedParallelDistribution::part_of_obj(const Uint obj) const
{
  return std::min(m_nb_parts-1, (obj - m_base) / part_size() );
}

//////////////////////////////////////////////////////////////////////////////

Uint MergedParallelDistribution::proc_of_part(const Uint part) const
{
  return std::min(PE::Comm::instance().size()-1, part / (m_nb_parts/PE::Comm::instance().size()));
}

//////////////////////////////////////////////////////////////////////////////

Uint MergedParallelDistribution::part_size() const
{
  Uint psize = 0;
  boost_foreach (Handle< ParallelDistribution > hash, m_subhash)
    psize += hash->part_size();
  return psize;
}

//////////////////////////////////////////////////////////////////////////////

Uint MergedParallelDistribution::proc_of_obj(const Uint obj) const
{
  return proc_of_part( part_of_obj(obj) );
}

//////////////////////////////////////////////////////////////////////////////

bool MergedParallelDistribution::rank_owns(const Uint obj) const
{
  if (proc_of_obj(obj) == PE::Comm::instance().rank())
    return true;
  else
    return false;
}

//////////////////////////////////////////////////////////////////////////////

bool MergedParallelDistribution::part_owns(const Uint part, const Uint obj) const
{
  if (part_of_obj(obj) == part)
    return true;
  else
    return false;
}

//////////////////////////////////////////////////////////////////////////////

Uint MergedParallelDistribution::nb_objects_in_part(const Uint part) const
{
  return end_idx_in_part(part) - start_idx_in_part(part);
}

//////////////////////////////////////////////////////////////////////////////

Uint MergedParallelDistribution::nb_objects_in_proc(const Uint proc) const
{
  Uint part_begin = m_nb_parts/PE::Comm::instance().size()*proc;
  Uint part_end = (proc == PE::Comm::instance().size()-1) ? m_nb_parts : m_nb_parts/PE::Comm::instance().size()*(proc+1);
  Uint nb_obj = 0;
  for (Uint part = part_begin ; part < part_end; ++part)
    nb_obj += nb_objects_in_part(part);
  return nb_obj;
}

//////////////////////////////////////////////////////////////////////////////

Uint MergedParallelDistribution::start_idx_in_part(const Uint part) const
{
  Uint start_idx = 0;
  boost_foreach (Handle< ParallelDistribution > hash, m_subhash)
    start_idx += hash->start_idx_in_part(part);
  return start_idx ;
}

//////////////////////////////////////////////////////////////////////////////

Uint MergedParallelDistribution::end_idx_in_part(const Uint part) const
{
  Uint end_idx = 0;
  boost_foreach (Handle< ParallelDistribution > hash, m_subhash)
    end_idx += hash->end_idx_in_part(part);
  return end_idx;
}

//////////////////////////////////////////////////////////////////////////////

Uint MergedParallelDistribution::start_idx_in_proc(const Uint proc) const
{
  Uint part_begin = m_nb_parts/PE::Comm::instance().size()*proc;
  return start_idx_in_part(part_begin);
}

//////////////////////////////////////////////////////////////////////////////

Uint MergedParallelDistribution::subhash_of_obj(const Uint obj) const
{
  Uint part = part_of_obj(obj);
  Uint start_idx_of_obj_part = start_idx_in_part( part );
  Uint offset = obj - start_idx_of_obj_part;

  Uint psize = 0;
  Uint i=0;
  boost_foreach(Handle< ParallelDistribution > hash, m_subhash)
  {
    psize += hash->nb_objects_in_part(part);
    if (offset < psize)
      return i;
    ++i;
  }
  cf3_assert_desc("Should not be here", false);
  return 0;
}

//////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
