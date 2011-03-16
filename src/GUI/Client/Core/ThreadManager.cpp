// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "GUI/Client/Core/NetworkThread.hpp"

#include "GUI/Client/Core/ThreadManager.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientCore {

////////////////////////////////////////////////////////////////////////////////

ThreadManager & ThreadManager::instance()
{
  static ThreadManager instance;
  return instance;
}

////////////////////////////////////////////////////////////////////////////////

ThreadManager::ThreadManager() :
    m_networkThread(new NetworkThread())
{

}

////////////////////////////////////////////////////////////////////////////////

ThreadManager::~ThreadManager()
{
  if(m_networkThread->isRunning())
    m_networkThread->exit(0);

  delete m_networkThread;
}

////////////////////////////////////////////////////////////////////////////////

NetworkThread & ThreadManager::network()
{
  return *m_networkThread;
}

////////////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF
