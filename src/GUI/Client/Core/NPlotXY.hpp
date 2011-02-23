// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_Core_NPlotXY_hpp
#define CF_GUI_Client_Core_NPlotXY_hpp

#include <boost/signals2/signal.hpp>

#include "GUI/Client/Core/CNode.hpp"

class QString;

namespace CF {
namespace GUI {
namespace ClientCore {


class CHistoryNotifier{
public:

  /// signal for the set_xy_data of graph.
  /*
  typedef boost::signals2::signal< void ( std::vector<double>&, std::vector<double>& ) >  sig_type;
  */
  typedef boost::signals2::signal< void ( std::vector< std::vector<double> > & fcts,
                                          std::vector<QString> & fct_label) >  sig_type;

  /// implementation of instance that return this ( staticly ).
  static CHistoryNotifier & instance(){
     static CHistoryNotifier inst; //create static instance
     return inst; //return the static instance
  }

  sig_type notify_history;

private:
  /// Empty constructor.
  CHistoryNotifier(){}

  /// Destructor.
  ~CHistoryNotifier(){}
};

class NPlotXY :
    public QObject,
    public CNode
{
  Q_OBJECT

public: //typedefs

  typedef boost::shared_ptr<NPlotXY> Ptr;
  typedef boost::shared_ptr<NPlotXY const> ConstPtr;

public:

  NPlotXY(const QString & name);

  virtual QString toolTip() const;

  void convergence_history ( Common::Signal::arg_t& node );

}; //  XYPlot

} // ClientCore
} // GUI
} // CF


#endif // CF_GUI_Client_Core_NPlotXY_hpp
