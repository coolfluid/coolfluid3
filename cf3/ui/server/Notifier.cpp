// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "ui/server/Notifier.hpp"

using namespace cf3::common;
using namespace cf3::ui::server;

Notifier::Notifier(NotificationQueue * observedQueue, QObject * parent)
  : QObject(parent),
    m_observedQueue(observedQueue)
{
  cf3_assert( observedQueue != nullptr );
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Notifier::~Notifier()
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Notifier::listenToEvent(const std::string & name, bool notifyOnce)
{
//  m_observedQueue->add_notifier(name, &Notifier::newEvent, this);

  if(notifyOnce)
    m_onceNotifyingEvents[name] = false;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Notifier::begin_notify()
{
  QMap<std::string, bool>::iterator it = m_onceNotifyingEvents.begin();

  for( ; it != m_onceNotifyingEvents.end() ; it++)
    it.value() = false;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Notifier::newEvent(const std::string & name, const URI & raiserPath)
{
//  QMap<std::string, bool>::iterator it = m_onceNotifyingEvents.find(name);

//  if(it == m_onceNotifyingEvents.end() || !it.value())
//  {
//    emit eventOccured(name, raiserPath);

//    if(it != m_onceNotifyingEvents.end())
//      it.value() = true;
//  }
}
