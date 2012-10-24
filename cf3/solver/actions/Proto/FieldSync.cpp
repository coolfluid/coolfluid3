// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/PE/Comm.hpp"

#include "FieldSync.hpp"

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

FieldSynchronizer::FieldSynchronizer()
{
}

FieldSynchronizer& FieldSynchronizer::instance()
{
  static FieldSynchronizer instance;
  return instance;
}


void FieldSynchronizer::insert(cf3::mesh::Field& f)
{
  m_fields[f.uri().path()] = f.handle<mesh::Field>();
}

void FieldSynchronizer::synchronize()
{
  if(common::PE::Comm::instance().is_active())
  {
    for(FieldsT::iterator field_it = m_fields.begin(); field_it != m_fields.end(); ++field_it)
    {
      field_it->second->synchronize();
    }
  }

  m_fields.clear();
}

} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3
