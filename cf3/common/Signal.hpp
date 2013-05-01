// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_Signal_hpp
#define cf3_common_Signal_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/signals2/signal.hpp>
#include <boost/signals2/connection.hpp>

#include "common/Exception.hpp"

#include "common/XML/SignalFrame.hpp" // try forward declaration

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

  class ConnectionManager;

////////////////////////////////////////////////////////////////////////////////

/// Exception thrown when errors detected while handling signals in SignalHandler
/// @author Tiago Quintino
class Common_API SignalError : public common::Exception {
public:

  /// Constructor
  SignalError (const common::CodeLocation& where, const std::string& what);

  virtual ~SignalError() throw();

}; // SignalError

////////////////////////////////////////////////////////////////////////////////

/// Class that harbours the types handled by the SignalHandler
///
/// @author Tiago Quintino
class Common_API Signal : public boost::noncopyable {

public: // typedefs

  /// Signal underlying type
  typedef boost::signals2::signal< void ( XML::SignalFrame& ) > signal_type;
  /// Signal connection type
  typedef boost::signals2::connection connection_type;
  /// Signal slot type
  typedef signal_type::slot_type slot_type;

public: // functions

  /// constuctor initializes signal with its name
  Signal( const std::string& name );

  /// destructor
  virtual ~Signal();

  /// @name MUTATORS
  //@{

  /// sets the description of this signal
  Signal& description( const std::string& desc );
  /// sets the pretty name of this signal
  Signal& pretty_name( const std::string& name );

  /// sets if it is read only signal
  Signal& read_only( bool is );
  /// sets if it is read only signal
  Signal& hidden( bool is );

  /// connects to a subscribing signature
  Signal& signature(const Signal::slot_type& subscriber);

  /// connects to a subscribing slot
  Signal& connect(const Signal::slot_type& subscriber);

  /// connects to a subscribing slot
  /// and saves the connection on a ConnectionManager
  Signal& connect(const Signal::slot_type& subscriber, ConnectionManager* mng );

  //@} END MUTATORS

  /// @name ACCESSORS
  //@{

  /// access the signal
  signal_type* signal()    { return m_signal.get(); }
  /// access the signature
  signal_type* signature() { return m_signature.get(); }

  /// gets the name of this signal
  std::string name() const;
  /// gets the description of this signal
  std::string description() const;
  /// gets the pretty name of this signal
  std::string pretty_name() const;

  /// gets if it is read only signal
  bool is_read_only() const;
  /// gets if it is read only signal
  bool is_hidden() const;

  //@} END ACCESSORS

private: // data

  /// the boost signal object
  boost::scoped_ptr< signal_type > m_signal;
  /// pointer to another signal that returns the signature of this signal
  boost::scoped_ptr< signal_type > m_signature;
  /// signal name
  std::string m_name;
  /// signal description
  std::string m_description;
  /// signal readable name (used by the GUI). For exemple, if key is
  /// "set_options", readable should be "Set options".
  std::string m_pretty_name;
  /// if @c true, the signal is considered as read-only and might be called
  /// during another signal execution. Default value is @c false.
  bool m_is_read_only;
  /// if @c true, the signal is hidden from the user's view, but still callable
  bool m_is_hidden;

}; // Signal

/// Automatic handling of connection closure using scoped_connection
/// @author Tiago Quintino
class Connection : public boost::noncopyable {

  boost::signals2::connection m_connection; ///< actual connection

  std::string m_name; ///< connection name

public:

  Connection( const std::string& cname ); ///< constructor with empty connection

  ~Connection(); ///< destructor closes connection

  std::string name() const { return m_name; }

  Connection* connect( const Signal::connection_type& conn );

  void disconnect();

};

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_Signal_hpp
