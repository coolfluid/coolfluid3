// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "GUI/Server/MPIListeningInfo.hpp"

using namespace MPI;
using namespace CF::GUI::Server;

MPIListeningInfo::MPIListeningInfo()
{
  m_counter = 0;
  m_ready = true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MPIListeningInfo::setComm(Intercomm comm)
{
  m_comm = comm;
  m_processCount = comm.Get_remote_size();
  m_finished = m_counter == m_processCount;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MPIListeningInfo::incCounter()
{
  if(!m_finished)
  {
    m_counter++;
    m_finished = m_counter == m_processCount;
  }
}

