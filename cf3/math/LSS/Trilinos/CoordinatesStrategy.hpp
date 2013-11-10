// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Math_LSS_CoordinatesStrategy_hpp
#define cf3_Math_LSS_CoordinatesStrategy_hpp

////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/scoped_ptr.hpp>

#include "math/LSS/SolutionStrategy.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
 *  @file CoordinatesStrategy.hpp Solution strategy abstract base for Trilinos strategies that take coordinates
 *  @author Bart Janssens
 **/
////////////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace math {
namespace LSS {

class LSS_API CoordinatesStrategy : public SolutionStrategy
{
public:
  CoordinatesStrategy(const std::string& name);
  ~CoordinatesStrategy();

  /// name of the type
  static std::string type_name () { return "CoordinatesStrategy"; }

  virtual void set_coordinates(common::PE::CommPattern& cp, const common::Table< Real >& coords, const common::List< Uint >& used_nodes, const std::vector< bool >& periodic_links_active);

protected:
  std::vector<Real> m_x_coords;
  std::vector<Real> m_y_coords;
  std::vector<Real> m_z_coords;
};

} // namespace LSS
} // namespace math
} // namespace cf3

#endif // cf3_Math_LSS_CoordinatesStrategy_hpp
