// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "Common/OptionT.hpp"
#include "Common/OptionArray.hpp"
#include "Common/MPI/PE.hpp"
#include "Common/Foreach.hpp"
#include "Common/String/Conversion.hpp"
#include "Common/Log.hpp"

#include "Mesh/CMixedHash.hpp"
#include "Mesh/CHash.hpp"

namespace CF {
namespace Mesh {

  using namespace Common;
  using namespace Common::String;
  
Common::ComponentBuilder < CMixedHash, Component, LibMesh > CMixedHash_Builder;

//////////////////////////////////////////////////////////////////////////////

CMixedHash::CMixedHash ( const std::string& name ) : 
    Component(name),
    m_nb_obj(),
    m_base(0),
    m_nb_parts(PE::instance().size())
{
  m_properties.add_option<OptionArrayT <Uint> >("Number of Objects","Total number of objects of each subhash. Subhashes will be created upon configuration with names hash_0 hash_1, ...",m_nb_obj);
  m_properties.add_option<OptionT <Uint> >("Number of Partitions","Total number of partitions (e.g. number of processors)",m_nb_parts);
  m_properties.add_option<OptionT <Uint> >("Base","Start index for global numbering",m_base);
  
  m_properties["Number of Partitions"].as_option().attach_trigger( boost::bind ( &CMixedHash::config_nb_parts , this) );
  m_properties["Base"].as_option().link_to( &m_base );  
  m_properties["Number of Objects"].as_option().attach_trigger( boost::bind ( &CMixedHash::config_nb_obj , this) );
}

//////////////////////////////////////////////////////////////////////////////

void CMixedHash::config_nb_obj ()
{
  property("Number of Objects").put_value(m_nb_obj);
  boost_foreach(CHash::Ptr hash, m_subhash)
    remove_static_component(hash->name());
  m_subhash.resize(0);
  index_foreach(i, Uint nb_obj, m_nb_obj)
  {
    CHash::Ptr hash = allocate_component_type<CHash>("hash_"+to_str(i));
    add_static_component(hash);
    m_subhash.push_back(hash);
    hash->configure_property("Number of Objects", nb_obj);
    hash->configure_property("Number of Partitions", m_nb_parts);
  }
}

//////////////////////////////////////////////////////////////////////////////

void CMixedHash::config_nb_parts ()
{
  property("Number of Partitions").put_value(m_nb_parts);
  if (m_subhash.size())
  {
    boost_foreach(CHash::Ptr hash, m_subhash)
      hash->configure_property("Number of Partitions", m_nb_parts);
  }
}

//////////////////////////////////////////////////////////////////////////////

Uint CMixedHash::part_of_obj(const Uint obj) const
{
  return std::min(m_nb_parts-1, (obj - m_base) / part_size() );
}

//////////////////////////////////////////////////////////////////////////////

Uint CMixedHash::proc_of_part(const Uint part) const
{
  return std::min(PE::instance().size()-1, part / (m_nb_parts/PE::instance().size()));
}

//////////////////////////////////////////////////////////////////////////////

Uint CMixedHash::part_size() const
{
  Uint psize = 0;
  boost_foreach (CHash::ConstPtr hash, m_subhash)
    psize += hash->part_size();
  return psize;
}

//////////////////////////////////////////////////////////////////////////////

Uint CMixedHash::proc_of_obj(const Uint obj) const
{
  return proc_of_part( part_of_obj(obj) );
}

//////////////////////////////////////////////////////////////////////////////

bool CMixedHash::owns(const Uint obj) const
{
  if (proc_of_obj(obj) == PE::instance().rank())
    return true;
  else
    return false;
}
	
//////////////////////////////////////////////////////////////////////////////

Uint CMixedHash::nb_objects_in_part(const Uint part) const
{
  return end_idx_in_part(part) - start_idx_in_part(part);
}

//////////////////////////////////////////////////////////////////////////////

Uint CMixedHash::nb_objects_in_proc(const Uint proc) const
{
	Uint part_begin = m_nb_parts/PE::instance().size()*proc;
	Uint part_end = (proc == PE::instance().size()-1) ? m_nb_parts : m_nb_parts/PE::instance().size()*(proc+1);
	Uint nb_obj = 0;
	for (Uint part = part_begin ; part < part_end; ++part)
		nb_obj += nb_objects_in_part(part);
	return nb_obj;
}

//////////////////////////////////////////////////////////////////////////////

Uint CMixedHash::start_idx_in_part(const Uint part) const
{
  Uint start_idx = 0;
  boost_foreach (CHash::ConstPtr hash, m_subhash)
    start_idx += hash->start_idx_in_part(part);
  return start_idx ;
}

//////////////////////////////////////////////////////////////////////////////

Uint CMixedHash::end_idx_in_part(const Uint part) const
{
  Uint end_idx = 0;
  boost_foreach (CHash::ConstPtr hash, m_subhash)
    end_idx += hash->end_idx_in_part(part);
  return end_idx;
}

//////////////////////////////////////////////////////////////////////////////

Uint CMixedHash::start_idx_in_proc(const Uint proc) const
{
	Uint part_begin = m_nb_parts/PE::instance().size()*proc;
	return start_idx_in_part(part_begin);
}

//////////////////////////////////////////////////////////////////////////////

Uint CMixedHash::subhash_of_obj(const Uint obj) const
{
  Uint part = part_of_obj(obj);
  Uint start_idx_of_obj_part = start_idx_in_part( part );
  Uint offset = obj - start_idx_of_obj_part;
  
  Uint psize = 0;
  index_foreach(i, CHash::ConstPtr hash, m_subhash)
  {
    psize += hash->nb_objects_in_part(part);
    if (offset < psize)
      return i;
  }
}

//////////////////////////////////////////////////////////////////////////////
	
} // Mesh
} // CF
