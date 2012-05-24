// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_core_SSHTunnel_hpp
#define cf3_ui_core_SSHTunnel_hpp

//////////////////////////////////////////////////////////////////////////////

#include "ui/core/LibCore.hpp"

#include <QString>
#include <string>
#include <QProcess>

class QProcess;

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace core {

class Core_API SSHTunnel : public QProcess {
  Q_OBJECT
public:
  /// @brief normal tunnel creation
  SSHTunnel(QObject* parent,quint16 local_port,quint16 distant_port,QString gateway_host
            ,QString gateway_user,QString distant_host,QString local_host,QString local_user);
  /// @brief reverse tunnel creation
  SSHTunnel(QObject* parent,QString local_host,QString gateway_host,QString distant_host
            ,quint16 local_port,quint16 distant_port,QString local_user,QString gateway_user
            ,QString distant_user);
  ~SSHTunnel();
  static SSHTunnel* simple_tunnel_popup(QWidget *parent);
  static SSHTunnel* reverse_tunnel_popup(QWidget *parent);
  static std::string get_local_signature();
private slots:
  void process_sent_output();
  void process_sent_error();
  void process_end(int status);
private:
  bool ssh_tunnel_is_open;
  static std::string local_signature;
};

} // Core
} // ui
} // cf3

//////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_core_SSHTunnel_hpp
