// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UI_ParaViewTab_N3DVIEW_HPP
#define CF_UI_ParaViewTab_N3DVIEW_HPP

// header
#include "UI/Core/CNode.hpp"
#include "UI/ParaViewTab/LibParaViewTab.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace ParaViewTab {


////////////////////////////////////////////////////////////////////////////////


  /// @brief Class used with C3DView to provide 3D mesh view.
  /// @author Wertz Gil
class ParaViewTab_API N3DView :
    public QObject,
    public UI::Core::CNode
{

    Q_OBJECT

public: //typedefs

  typedef boost::shared_ptr<N3DView> Ptr;
  typedef boost::shared_ptr<N3DView const> ConstPtr;

public: //function

    /// Constructor
    /// @param name Name of the node.
    N3DView(const std::string& name);

    virtual ~N3DView();

    /// toolTip
    virtual QString toolTip() const;

    /// This function send paraview server information to the 3D View.
    /// @param node
    void launch_pvserver( common::SignalArgs& node );

    /// This function send server file information to the 3D View.
    /// @param node
    void send_server_info_to_client( common::SignalArgs& node );

    virtual void aboutToBeRemoved();

protected:

  /// Disables the local signals that need to.
  /// @param localSignals Map of local signals. All values are set to true
  /// by default.
  virtual void disableLocalSignals(QMap<QString, bool> & localSignals) const {}

  void reload_client_view();

  virtual void setUpFinished();

private :

  void go_to_tab( Common::SignalArgs & args );

};

////////////////////////////////////////////////////////////////////////////////


} // ParaViewTab
} // UI
} // CF

#endif // CF_UI_ParaViewTab_N3DVIEW_HPP
