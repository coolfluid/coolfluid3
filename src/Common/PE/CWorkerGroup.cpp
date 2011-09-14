// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/LibCommon.hpp"
#include "Common/Signal.hpp"

#include "Common/XML/FileOperations.hpp"

#include "Common/PE/CWorkerGroup.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
namespace PE {

////////////////////////////////////////////////////////////////////////////////

ComponentBuilder< CWorkerGroup, Component, LibCommon > CWorkerGroup_Builder;

////////////////////////////////////////////////////////////////////////////////

CWorkerGroup::CWorkerGroup( const std::string & name )
  : Component(name),
    m_comm(MPI_COMM_NULL)
{
  regist_signal( "solve" )
    ->description("Runs a fake simulation")
    ->pretty_name("Solve")->connect( boost::bind(&CWorkerGroup::signal_solve, this, _1));
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

void CWorkerGroup::signal_solve( Common::SignalArgs & args)
{
  std::string str;
  char * buffer;
  int remote_size;
  SignalFrame frame("solve", uri(), "//Root/Worker");

  to_string( *frame.xml_doc, str);

  buffer = new char[ str.length() + 1 ];
  std::strcpy( buffer, str.c_str() );

  MPI_Comm_remote_size(m_comm, &remote_size);

  for(int i = 0 ; i < remote_size ; ++i)
    MPI_Send( buffer, str.length() + 1, MPI_CHAR, i, 0, m_comm );
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

} // namespace PE
} // namespace Common
} // namespace CF

