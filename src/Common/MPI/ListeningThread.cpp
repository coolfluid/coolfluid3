// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <unistd.h>

#include <boost/date_time.hpp>
#include <boost/thread.hpp>

#include "Common/Assertions.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/Log.hpp"

#include "Common/XML/FileOperations.hpp"

#include "Common/MPI/ListeningInfo.hpp"

#include "Common/MPI/ListeningThread.hpp"

using namespace boost;
using namespace CF::Common::XML;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
namespace Comm {

////////////////////////////////////////////////////////////////////////////

ListeningThread::ListeningThread(unsigned int waitingTime)
  : m_sleep_duration(waitingTime),
    m_listening(false)
{

}

////////////////////////////////////////////////////////////////////////////

void ListeningThread::add_communicator( Communicator comm )
{
  m_mutex.lock();

  cf_assert( comm != MPI_COMM_NULL );
  cf_assert( m_comms.find(comm) == m_comms.end() );

  m_comms[comm] = new ListeningInfo();

  m_mutex.unlock();
}

////////////////////////////////////////////////////////////////////////////

void ListeningThread::remove_comunicator( Communicator comm )
{
  m_mutex.lock();

  cf_assert( comm!= MPI_COMM_NULL );

  std::map<Communicator, ListeningInfo*>::iterator it = m_comms.find(comm);

  cf_assert( it != m_comms.end() );

  m_comms.erase(it);

  m_mutex.unlock();
}

////////////////////////////////////////////////////////////////////////////

void ListeningThread::stop_listening()
{
//  m_mutex.lock();

  m_listening = false;

//  m_thread.join();

//  m_mutex.unlock();
}

////////////////////////////////////////////////////////////////////////////

void ListeningThread::start_listening()
{
  if( !m_listening && !m_comms.empty() )
  {
    m_listening = true;

    m_thread = boost::thread(&ListeningThread::run, this);
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

boost::thread & ListeningThread::thread()
{
  return m_thread;
}

////////////////////////////////////////////////////////////////////////////

void ListeningThread::init()
{
  static int count = 0;
  m_mutex.lock();

  std::map<Communicator, ListeningInfo*>::iterator it = m_comms.begin();

  // non-blocking receive on all communicators
  for( ; it != m_comms.end() && m_listening ; ++it )
  {
    ListeningInfo * info = it->second;

    if( info->ready )
    {
      MPI_Request request;
      info->request = request;

      MPI_Irecv(info->data, ListeningInfo::buffer_size(), MPI_CHAR,
                    MPI_ANY_SOURCE, 0, it->first, &info->request);

      info->ready = false;
    }
  }

  m_mutex.unlock();
}

////////////////////////////////////////////////////////////////////////////

void ListeningThread::run()
{
  while( m_listening )
  {
    this->init(); // initialize the listening process for comms that need it
    this_thread::sleep( posix_time::milliseconds(m_sleep_duration) );
    this->check_for_data(); // check if data arrived
  }
}

////////////////////////////////////////////////////////////////////////////

void ListeningThread::check_for_data()
{
  m_mutex.lock();

  ExceptionManager::instance().ExceptionDumps = false;

  std::map<Communicator, ListeningInfo*>::iterator it = m_comms.begin();

  for( ; it != m_comms.end() && m_listening ; ++it )
  {
    ListeningInfo * info = it->second;

    // if the communicator is waiting for data
    if( !info->ready )
    {
      int flag;

      MPI_Test(&info->request, &flag, MPI_STATUS_IGNORE);

      // if data arrived, flag is not zero
      if( flag != 0 )
      {
        try
        {
          XmlDoc::Ptr doc = XML::parse_cstring( info->data );

          new_signal( it->first, doc );

          info->ready = true; // ready to do another non-blocking receive
        }
        catch(XmlError & e)
        {
          CFerror << e.what() << CFendl;
          m_listening = false;
        }
      }
    }
  }

  m_mutex.unlock();
}

////////////////////////////////////////////////////////////////////////////

} // Comm
} // Common
} // CF

/////////////////////////////////////////////////////////////////////////////
