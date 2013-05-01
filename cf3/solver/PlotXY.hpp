// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_PlotXY_hpp
#define cf3_solver_PlotXY_hpp

//////////////////////////////////////////////////////////////////////////////

#include "common/Table.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {

//////////////////////////////////////////////////////////////////////////////

/// Component to maintain convergence history
/// @author Gil Wertz
/// @author Quentin Gasper
class PlotXY :
    public common::Component
{
public: // typedefs

  /// pointers
  
  

public:

    PlotXY(const std::string& name);

    virtual ~PlotXY();

    /// Get the class name
    static std::string type_name () { return "PlotXY"; }

    void set_data (const common::URI & uri);

    void convergence_history( common::SignalArgs & args );

  private: // data

    std::vector<Real> m_x_axis;

    std::vector<Real> m_y_axis;

    int m_num_it;

    Handle< common::Table<Real> > m_data;

}; // PlotXY

//////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

//////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_PlotXY_hpp
