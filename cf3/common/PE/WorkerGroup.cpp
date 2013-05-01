// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/LibCommon.hpp"
#include "common/Signal.hpp"

#include "common/XML/FileOperations.hpp"

#include "common/PE/WorkerGroup.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
namespace PE {

////////////////////////////////////////////////////////////////////////////////

ComponentBuilder< WorkerGroup, Component, LibCommon > WorkerGroup_Builder;

////////////////////////////////////////////////////////////////////////////////

WorkerGroup::WorkerGroup( const std::string & name )
  : Component(name),
    m_comm(MPI_COMM_NULL)
{
  regist_signal( "solve" )
    .description("Runs a fake simulation")
    .pretty_name("Solve").connect( boost::bind(&WorkerGroup::signal_solve, this, _1));
}

////////////////////////////////////////////////////////////////////////////////

WorkerGroup::~WorkerGroup()
{

}

////////////////////////////////////////////////////////////////////////////////

void WorkerGroup::set_communicator( Communicator comm )
{
  cf3_assert( comm != MPI_COMM_NULL );

  m_comm = comm;
}

////////////////////////////////////////////////////////////////////////////////

Communicator WorkerGroup::communicator() const
{
  cf3_assert( m_comm != MPI_COMM_NULL );

  return m_comm;
}

////////////////////////////////////////////////////////////////////////////////

void WorkerGroup::signal_solve( common::SignalArgs & args)
{
  std::string str;
  char * buffer;
  int remote_size;
  SignalFrame frame("solve", uri(), "//Worker");

  to_string( *frame.xml_doc, str);

  buffer = new char[ str.length() + 1 ];
  std::strcpy( buffer, str.c_str() );

  MPI_Comm_remote_size(m_comm, &remote_size);

  for(int i = 0 ; i < remote_size ; ++i)
    MPI_Send( buffer, str.length() + 1, MPI_CHAR, i, 0, m_comm );
}

////////////////////////////////////////////////////////////////////////////////

int WorkerGroup::nbworkers () const
{
  int size;

  // use macro?
  MPI_Comm_remote_size(m_comm, &size);

  return size;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace PE
} // namespace common
} // namespace cf3

