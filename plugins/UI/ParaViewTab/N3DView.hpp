// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UI_ParaViewTab_N3DVIEW_HPP
#define CF_UI_ParaViewTab_N3DVIEW_HPP

#include <QString>
#include <QObject>

#include "UI/Core/CNode.hpp"

#include <boost/signals2/signal.hpp>

#include "UI/ParaViewTab/LibParaViewTab.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace ParaViewTab {


////////////////////////////////////////////////////////////////////////////////

class ParaViewTab_API N3DView :
    public QObject,
    public UI::Core::CNode
{

    Q_OBJECT

public: //typedefs

  typedef boost::shared_ptr<N3DView> Ptr;
  typedef boost::shared_ptr<N3DView const> ConstPtr;

public:
    N3DView(const std::string & name);

    void reload_client_view();

    void pvserver_launched(QString host,QString port,QString path);

    virtual QString toolTip() const;

    void launch_pvserver( Common::SignalArgs& node );

    void send_server_info_to_client( Common::SignalArgs& node );

protected:

  /// Disables the local signals that need to.
  /// @param localSignals Map of local signals. All values are set to true
  /// by default.
  virtual void disableLocalSignals(QMap<QString, bool> & localSignals) const {}

  virtual void setUpFinished();

};


class Core_API N3DViewNotifier{
public:

  typedef boost::signals2::signal< void ( QString host, QString port) >  sig_type;
  typedef boost::signals2::signal< void ( std::vector<QString>,  std::vector<QString>) >  sig_type2;


  /// implementation of instance that return this ( staticly ).
  static N3DViewNotifier & instance(){
     static N3DViewNotifier inst; // create static instance
     return inst; // return the static instance
  }

  sig_type notify_server_spec;
  sig_type2 notify_path_spec;

private:
  /// Empty constructor.
  N3DViewNotifier(){}

  /// Destructor.
  ~N3DViewNotifier(){}
};

////////////////////////////////////////////////////////////////////////////////


} // Core
} // UI
} // CF

#endif // CF_UI_ParaViewTab_N3DVIEW_HPP
