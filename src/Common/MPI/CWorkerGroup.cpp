// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/LibCommon.hpp"

#include "Common/MPI/CWorkerGroup.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
namespace mpi {

////////////////////////////////////////////////////////////////////////////////

ComponentBuilder< CWorkerGroup, Component, LibCommon > CWorkerGroup_Builder;

////////////////////////////////////////////////////////////////////////////////

CWorkerGroup::CWorkerGroup( const std::string & name )
  : Component(name),
    m_comm(MPI_COMM_NULL)
{

}

////////////////////////////////////////////////////////////////////////////////

CWorkerGroup::~CWorkerGroup()
{

}

////////////////////////////////////////////////////////////////////////////////

void CWorkerGroup::set_communicator( Communicator comm )
{
  cf_assert( comm != MPI_COMM_NULL );

  m_comm = comm;
}

////////////////////////////////////////////////////////////////////////////////

Communicator CWorkerGroup::communicator() const
{
  cf_assert( m_comm != MPI_COMM_NULL );

  return m_comm;
}

////////////////////////////////////////////////////////////////////////////////

int CWorkerGroup::nbworkers () const
{
  int size;

  // use macro?
  MPI_Comm_remote_size(m_comm, &size);

  return size;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace mpi
} // namespace Common
} // namespace CF

