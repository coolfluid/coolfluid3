// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_core_NPlotXY_hpp
#define cf3_ui_core_NPlotXY_hpp

////////////////////////////////////////////////////////////////////////////

//header
#include <boost/signals2/signal.hpp>

/// @warning nasty fix !! resolves compilation with Boost 1.48.
/// should be removed Qt's moc tool will be fixed.
#ifndef Q_MOC_RUN
#include "common/BoostArray.hpp"
#endif

#include "ui/core/CNode.hpp"
#include "ui/QwtTab/LibQwtTab.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace QwtTab {

////////////////////////////////////////////////////////////////////////////

class QwtTab_API NPlotXY :
    public QObject,
    public core::CNode
{
  Q_OBJECT

public: //typedefs

  typedef boost::shared_ptr<NPlotXY> Ptr;
  typedef boost::shared_ptr<NPlotXY const> ConstPtr;

  /// @warning nasty fix !! resolves compilation with Boost 1.48.
  /// should be removed Qt's moc tool will be fixed.
#ifndef Q_MOC_RUN
  typedef boost::multi_array<Real, 2> PlotData;
  typedef boost::shared_ptr< PlotData > PlotDataPtr;
#endif

public:

  NPlotXY(const std::string & name);

  virtual QString tool_tip() const;

  void convergence_history ( common::SignalArgs& node );

  void show_hide_plot( common::SignalArgs& node );

  void go_to_plot( common::SignalArgs& node );

protected:

  /// Disables the local signals that need to.
  /// @param localSignals Map of local signals. All values are set to true
  /// by default.
  virtual void disable_local_signals(QMap<QString, bool> & localSignals) const;

  virtual void setup_finished();

}; //  XYPlot

////////////////////////////////////////////////////////////////////////////

} // QwtTab
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_core_NPlotXY_hpp
