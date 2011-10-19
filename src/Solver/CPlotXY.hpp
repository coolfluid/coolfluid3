// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Solver_CPlotXY_hpp
#define cf3_Solver_CPlotXY_hpp

//////////////////////////////////////////////////////////////////////////////

#include "Mesh/CTable.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Solver {

//////////////////////////////////////////////////////////////////////////////

/// Component to maintain convergence history
/// @author Gil Wertz
/// @author Quentin Gasper
class CPlotXY :
    public common::Component
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<CPlotXY> Ptr;
  typedef boost::shared_ptr<CPlotXY const> ConstPtr;

public:

    CPlotXY(const std::string& name);

    virtual ~CPlotXY();

    /// Get the class name
    static std::string type_name () { return "CPlotXY"; }

    void set_data (const common::URI & uri);

    void convergence_history( common::SignalArgs & args );

  private: // data

    std::vector<Real> m_x_axis;

    std::vector<Real> m_y_axis;

    int m_num_it;

    Mesh::CTable<Real>::Ptr m_data;

}; // CPlotXY

//////////////////////////////////////////////////////////////////////////////

} // Solver
} // cf3

//////////////////////////////////////////////////////////////////////////////

#endif // CF3_Solver_CPlotXY_hpp
