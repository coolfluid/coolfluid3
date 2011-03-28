// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QMutex>

#include "UI/Core/TreeThread.hpp"
#include "UI/Core/NetworkThread.hpp"

#include "UI/Core/ThreadManager.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Core {

////////////////////////////////////////////////////////////////////////////////

ThreadManager & ThreadManager::instance()
{
  static ThreadManager instance;
  return instance;
}

////////////////////////////////////////////////////////////////////////////////

ThreadManager::ThreadManager() :
    m_networkThread(new NetworkThread()),
    m_treeThread(new TreeThread())
{
}

////////////////////////////////////////////////////////////////////////////////

ThreadManager::~ThreadManager()
{
  delete m_networkThread;
  delete m_treeThread;
}

////////////////////////////////////////////////////////////////////////////////

NetworkThread & ThreadManager::network()
{
  return *m_networkThread;
}

////////////////////////////////////////////////////////////////////////////////

TreeThread & ThreadManager::tree()
{
  if(!m_treeThread->isRunning())
  {
    QMutex mutex;

    mutex.lock();

    m_treeThread->setMutex(&mutex);
    m_treeThread->start();

    mutex.lock();
  }

  return *m_treeThread;
}

////////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // CF
