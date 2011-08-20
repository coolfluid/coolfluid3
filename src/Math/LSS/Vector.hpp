// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Math_LSS_Vector_hpp
#define CF_Math_LSS_Vector_hpp

// OBJECTIVE: restrictive and simple to use

////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/utility.hpp>

#include "Common/CommonAPI.hpp"
#include "Common/MPI/PE.hpp"
#include "Common/MPI/CommPattern.hpp"
#include "blockaccumulator.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Math {
namespace LSS {

////////////////////////////////////////////////////////////////////////////////////////////

/// @TODO: properly implement component (type_name,ptr,constptr)
class Common_API LSSVector : public Component {
public:

  /// @name CREATION, DESTRUCTION AND COMPONENT SYSTEM
  //@{

  /// pointer to this type
  typedef boost::shared_ptr<LSSVector> Ptr;

  /// const pointer to this type
  typedef boost::shared_ptr<LSSVector const> ConstPtr;

  /// name of the type
  static std::string type_name () { return "LSSVector"; }

  /// Default constructor
  LSSVector(const std::string& name) : Component(name) { }

  //@} END CREATION, DESTRUCTION AND COMPONENT SYSTEM

  /// @name INDIVIDUAL ACCESS
  //@{

  /// Set value at given location in the matrix
  virtual void set_value(const Uint row, const Real value) = 0;

  /// Add value at given location in the matrix
  virtual void add_value(const Uint row, const Real value) = 0;

  /// Get value at given location in the matrix
  virtual void get_value(const Uint row, const Real& value) = 0;

  //@} END INDIVIDUAL ACCESS

  /// @name EFFICCIENT ACCESS
  //@{

  /// Set a list of values to rhs
  virtual void set_rhs_values(const BlockAccumulator& values) = 0;

  /// Add a list of values to rhs
  virtual void add_rhs_values(const BlockAccumulator& values) = 0;

  /// Get a list of values from rhs
  virtual void get_rhs_values(BlockAccumulator& values) = 0;

  /// Set a list of values to sol
  virtual void set_sol_values(const BlockAccumulator& values) = 0;

  /// Add a list of values to sol
  virtual void add_sol_values(const BlockAccumulator& values) = 0;

  /// Get a list of values from sol
  virtual void get_sol_values(BlockAccumulator& values) = 0;

  /// Reset Vector
  virtual void reset(Real reset_to=0.) = 0;

  //@} END EFFICCIENT ACCESS

  /// @name MISCELLANEOUS
  //@{

  /// Print this vector to screen
  virtual void print_to_screen() = 0;

  /// Print this vector to file
  virtual void print_to_file(const char* fileName) = 0;

  //@} END MISCELLANEOUS

};

////////////////////////////////////////////////////////////////////////////////////////////

} // namespace LSS
} // namespace Math
} // namespace CF

#endif // CF_Math_LSS_Vector_hpp
