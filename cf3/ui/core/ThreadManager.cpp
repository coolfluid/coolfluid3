// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QMutex>

#include "ui/core/TreeThread.hpp"
#include "ui/core/NetworkThread.hpp"

#include "ui/core/ThreadManager.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace core {

////////////////////////////////////////////////////////////////////////////////

ThreadManager & ThreadManager::instance()
{
  static ThreadManager instance;
  return instance;
}

////////////////////////////////////////////////////////////////////////////////

ThreadManager::ThreadManager() :
    m_network_thread(new NetworkThread()),
    m_tree_thread(new TreeThread())
{
}

////////////////////////////////////////////////////////////////////////////////

ThreadManager::~ThreadManager()
{
  delete m_network_thread;
  delete m_tree_thread;
}

////////////////////////////////////////////////////////////////////////////////

NetworkThread & ThreadManager::network()
{
  return *m_network_thread;
}

////////////////////////////////////////////////////////////////////////////////

TreeThread & ThreadManager::tree()
{
  if(!m_tree_thread->isRunning())
  {
    QMutex mutex;

    mutex.lock();

    m_tree_thread->set_mutex(&mutex);
    m_tree_thread->start();

    mutex.lock();
  }

  return *m_tree_thread;
}

////////////////////////////////////////////////////////////////////////////////

} // Core
} // ui
} // cf3
