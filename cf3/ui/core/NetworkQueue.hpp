// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_core_NetworkQueue_hpp
#define cf3_ui_core_NetworkQueue_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QMap>

#include "common/SignalDispatcher.hpp"

#include "common/XML/SignalFrame.hpp"

#include "ui/core/CNode.hpp"

template<class T> class QList;
class QFile;
class QTextStream;

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {

namespace Tools { namespace Shell { class Interpreter; } }

namespace ui {
namespace core {

//////////////////////////////////////////////////////////////////////////////

struct Core_API Transaction
{
public:

  QString uuid;

  QList< common::SignalArgs > actions;

  bool from_script;

  Transaction( const QString & trans_uuid ) : uuid(trans_uuid), from_script(false) {}

}; // Transaction

//////////////////////////////////////////////////////////////////////////////

class Core_API NetworkQueue
    : public CNode,
      public common::SignalDispatcher
{
public: // typedefs

  
  

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

  Transaction * send ( common::SignalArgs & args, Priority priority = MEDIUM );

  QString start_transaction();

  void add_to_transaction( const QString & uuid, common::SignalArgs & args );

  void insert_transaction( const QString & uuid, Priority priority = MEDIUM );

  virtual QString tool_tip() const;

  static Handle< NetworkQueue > global();

  inline bool is_running() const { return m_current_index != -1; }

  void start();

  void signal_ack ( common::SignalArgs & args );

  void dispatch_signal( const std::string &target,
                        const common::URI &receiver,
                        common::SignalArgs &args );

  void execute_script( const QString & filename );


protected:

  virtual void disable_local_signals(QMap<QString, bool> &local_signals) const {}

private: // data

  QMap<QString, Transaction * > m_new_transactions;

  QList< Transaction * > m_transactions;

  int m_current_index;

  std::string m_current_frame_id;

  void send_next_frame();

  void send_next_command();

  QFile * m_script_file;

  QTextStream * m_script_stream;

  Tools::Shell::Interpreter * m_interpreter;

}; // NetworkQueue

//////////////////////////////////////////////////////////////////////////////

} // Core
} // ui
} // cf3

//////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_core_NetworkQueue_hpp
