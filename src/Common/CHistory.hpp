// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_CHistory_hpp
#define CF_Common_CHistory_hpp

//////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

//////////////////////////////////////////////////////////////////////////////

/// Component to maintain convergence history
/// @author Gil Wertz
/// @author Quentin Gasper
class CHistory :
    public Component
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<CHistory> Ptr;
  typedef boost::shared_ptr<CHistory const> ConstPtr;

public:

    CHistory(const std::string& name);

    virtual ~CHistory();

    /// Get the class name
    static std::string type_name () { return "CHistory"; }

    void convergence_history( Signal::arg_t & args );

    /// @param points Number of points to compute.
    void sine (int points);

  private: // data

    std::vector<Real> m_x_axis;

    std::vector<Real> m_y_axis;

    int m_num_it;

}; // CHistory

//////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CHistory_hpp
