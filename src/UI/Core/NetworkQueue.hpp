// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UI_Core_NetworkQueue_hpp
#define CF_UI_Core_NetworkQueue_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QMap>

#include "Common/SignalDispatcher.hpp"

#include "Common/XML/SignalFrame.hpp"

#include "UI/Core/CNode.hpp"

template<class T> class QList;
class QFile;
class QTextStream;

//////////////////////////////////////////////////////////////////////////////

namespace CF {

namespace Tools { namespace Shell { class Interpreter; } }

namespace UI {
namespace Core {

//////////////////////////////////////////////////////////////////////////////

struct Core_API Transaction
{
public:

  QString uuid;

  QList< Common::SignalArgs > actions;

  Transaction( const QString & trans_uuid ) : uuid(trans_uuid) {}

}; // Transaction

//////////////////////////////////////////////////////////////////////////////

class Core_API NetworkQueue
    : public CNode,
      public Common::SignalDispatcher
{
public: // typedefs

  typedef boost::shared_ptr<NetworkQueue> Ptr;
  typedef boost::shared_ptr<NetworkQueue const> ConstPtr;

public: // enums

  enum Priority
  {
    IMMEDIATE,
    HIGH,    // will be the next frame to be sent (put at the beginning of the queue)
    MEDIUM,  // find an explanation...
    LOW      // put at the end of the queue
  }; // Priority

public:

  NetworkQueue();

  ~NetworkQueue();

  static std::string class_name() { return "NetworkQueue"; }

  QString send ( Common::SignalArgs & args, Priority priority = MEDIUM );

  QString start_transaction();

  void add_to_transaction( const QString & uuid, Common::SignalArgs & args );

  void insert_transaction( const QString & uuid, Priority priority = MEDIUM );

  virtual QString toolTip() const;

  static NetworkQueue::Ptr global_queue();

  inline bool isRunning() const { return m_currentIndex != -1; }

  void start();

  void signal_ack ( Common::SignalArgs & args );

  void dispatch_signal( const std::string &target,
                        const Common::URI &receiver,
                        Common::SignalArgs &args );

  void execute_script( const QString & filename );


protected:

  virtual void disableLocalSignals(QMap<QString, bool> &localSignals) const {}

private: // data

  QMap<QString, Transaction * > m_newTransactions;

  QList< Transaction * > m_transactions;

  int m_currentIndex;

  std::string m_currentFrameID;

  void send_next_frame();

  void send_next_command();

  QFile * m_scriptFile;

  QTextStream * m_scriptStream;

  Tools::Shell::Interpreter * m_interpreter;

}; // NetworkQueue

//////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_UI_Core_NetworkQueue_hpp
