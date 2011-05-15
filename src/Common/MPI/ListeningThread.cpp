// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/date_time.hpp>
#include <boost/thread.hpp>

#include "Common/Assertions.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/Log.hpp"

#include "Common/XML/FileOperations.hpp"

#include "Common/MPI/ListeningThread.hpp"

using namespace boost;
using namespace MPI;
using namespace CF::Common::XML;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
namespace mpi {

////////////////////////////////////////////////////////////////////////////

ListeningThread::ListeningThread(unsigned int waitingTime)
  : m_sleep_duration(waitingTime),
    m_listening(false),
    m_receivingAcksComm(COMM_NULL)
{

}

////////////////////////////////////////////////////////////////////////////

void ListeningThread::add_communicator(const Intercomm & comm)
{
  m_mutex.lock();

  cf_assert( m_comms.find(comm) == m_comms.end() );

  m_comms[comm] = ListeningInfo();

  m_mutex.unlock();
}

////////////////////////////////////////////////////////////////////////////

void ListeningThread::stop_listening()
{
  m_mutex.lock();

  m_listening = false;

  m_mutex.unlock();
}

////////////////////////////////////////////////////////////////////////////

void ListeningThread::start_listening()
{
  std::map<Intercomm, ListeningInfo>::iterator it;

  if( !m_comms.empty() )
  {
    m_listening = true;

    while( m_listening )
    {
      this->init(); // initialize the listening process for comms that need it
      this_thread::sleep( posix_time::milliseconds(m_sleep_duration) );
      this->check_for_data();
    }
  }
}

////////////////////////////////////////////////////////////////////////////

void ListeningThread::set_sleep_duration(Uint time)
{
  m_sleep_duration = time;
}

////////////////////////////////////////////////////////////////////////////

Uint ListeningThread::sleep_duration() const
{
  return m_sleep_duration;
}

////////////////////////////////////////////////////////////////////////////

void ListeningThread::init()
{
  m_mutex.lock();

  std::map<Intercomm, ListeningInfo>::iterator it = m_comms.begin();

  // non-blocking receive on all communicators
  for( ; it != m_comms.end() && m_listening ; ++it )
  {
    ListeningInfo & info = it->second;

    if( info.ready )
    {
      info.request = it->first.Irecv(info.data, ListeningInfo::buffer_size(),
                                     CHAR, ANY_SOURCE, 0);
      info.ready = false;
    }
  }

  m_mutex.unlock();
}

////////////////////////////////////////////////////////////////////////////

void ListeningThread::check_for_data()
{
  m_mutex.lock();

  std::map<Intercomm, ListeningInfo>::iterator it = m_comms.begin();

  for( ; it != m_comms.end() && m_listening ; ++it )
  {
    ListeningInfo & info = it->second;

    // if data arrived
    if( !info.ready && info.request.Test() )
    {
      try
      {
        XmlDoc::Ptr doc = XML::parse_cstring( info.data );

        if( doc->is_valid() )
          new_signal( it->first, doc );
        else
        {
          throw XmlError(FromHere(), std::string("The string [") + info.data +
                         "] could not be parsed.");
        }

        info.ready = true; // ready to do another non-blocking receive
      }
      catch(XmlError & e)
      {
        CFerror << e.what() << CFendl;
      }
    }
  }

  m_mutex.unlock();
}

////////////////////////////////////////////////////////////////////////////

} // mpi
} // Common
} // CF

/////////////////////////////////////////////////////////////////////////////
