// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Math_LSS_EmptyLSSVector_hpp
#define CF_Math_LSS_EmptyLSSVector_hpp

////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/utility.hpp>

#include "Math/LibMath.hpp"
#include "Common/MPI/CommPattern.hpp"
#include "Math/LSS/Vector.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
  @file EmptyLSSVector.hpp implementation of LSS::EmptyLSSVector
  @author Tamas Banyai

  EmptyLSSVector is intended to use for testing purposes only.
  It acts like a fully operational linear solver, but it does not solve and allocate any memory.

  // @todo turn it into a testing suite and throws for everything incorrect
**/

////////////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Math {
namespace LSS {

////////////////////////////////////////////////////////////////////////////////////////////

class Math_API EmptyLSSVector : public LSS::Vector {
public:

  /// @name CREATION, DESTRUCTION AND COMPONENT SYSTEM
  //@{

  /// pointer to this type
  typedef boost::shared_ptr<EmptyLSSVector> Ptr;

  /// const pointer to this type
  typedef boost::shared_ptr<EmptyLSSVector const> ConstPtr;

  /// name of the type
  static std::string type_name () { return "EmptyLSSVector"; }

  /// Accessor to solver type
  const std::string solvertype() { return "EmptyLSS"; }

  /// Default constructor
  EmptyLSSVector(const std::string& name) :
    LSS::Vector(name),
    m_is_created(false),
    m_neq(0),
    m_blockrow_size(0)
  { }

  /// Setup sparsity structure
  void create(const Common::Comm::CommPattern& cp, Uint neq)
  {
    destroy();
    m_neq=neq;
    m_blockrow_size=cp.gid()->size();
    m_is_created=true;
  }

  /// Deallocate underlying data
  void destroy()
  {
    m_is_created=false;
    m_neq=0;
    m_blockrow_size=0;
  }


  //@} END CREATION, DESTRUCTION AND COMPONENT SYSTEM

  /// @name INDIVIDUAL ACCESS
  //@{

  /// Set value at given location in the matrix
  void set_value(const Uint irow, const Real value) { cf_assert(m_is_created); }

  /// Add value at given location in the matrix
  void add_value(const Uint irow, const Real value) { cf_assert(m_is_created); }

  /// Get value at given location in the matrix
  void get_value(const Uint irow, Real& value) { cf_assert(m_is_created); value=0.; }

  /// Set value at given location in the matrix
  void set_value(const Uint iblockrow, const Uint ieq, const Real value) { cf_assert(m_is_created); }

  /// Add value at given location in the matrix
  void add_value(const Uint iblockrow, const Uint ieq, const Real value) { cf_assert(m_is_created); }

  /// Get value at given location in the matrix
  void get_value(const Uint iblockrow, const Uint ieq, Real& value) { cf_assert(m_is_created); value=0.; }

  //@} END INDIVIDUAL ACCESS

  /// @name EFFICCIENT ACCESS
  //@{

  /// Set a list of values to rhs
  void set_rhs_values(const BlockAccumulator& values) { cf_assert(m_is_created); }

  /// Add a list of values to rhs
  void add_rhs_values(const BlockAccumulator& values) { cf_assert(m_is_created); }

  /// Get a list of values from rhs
  void get_rhs_values(BlockAccumulator& values) { cf_assert(m_is_created); values.rhs.setConstant(0.); }

  /// Set a list of values to sol
  void set_sol_values(const BlockAccumulator& values) { cf_assert(m_is_created); }

  /// Add a list of values to sol
  void add_sol_values(const BlockAccumulator& values) { cf_assert(m_is_created); }

  /// Get a list of values from sol
  void get_sol_values(BlockAccumulator& values) { cf_assert(m_is_created); values.sol.setConstant(0.); }

  /// Reset Vector
  void reset(Real reset_to=0.) { cf_assert(m_is_created); }

  //@} END EFFICCIENT ACCESS

  /// @name MISCELLANEOUS
  //@{

  /// Print to wherever
  void print(Common::LogStream& stream) { cf_assert(m_is_created); stream << "EmptyLSSVector::print of '" << name() << "'.\n"; }

  /// Print to wherever
  void print(std::ostream& stream) { cf_assert(m_is_created); stream << "EmptyLSSVector::print of '" << name() << "'.\n"; }

  /// Print to file given by filename
  void print(const std::string& filename) { cf_assert(m_is_created); }

  /// Accessor to the state of create
  const bool is_created() { return m_is_created; }

  /// Accessor to the number of equations
  const Uint neq() { cf_assert(m_is_created); return m_neq; }

  /// Accessor to the number of block rows
  const Uint blockrow_size() { cf_assert(m_is_created); return m_blockrow_size; }

  //@} END MISCELLANEOUS

private:

  /// state of creation
  bool m_is_created;

  /// number of equations
  Uint m_neq;

  /// number of block columns
  Uint m_blockrow_size;

};

////////////////////////////////////////////////////////////////////////////////////////////

} // namespace LSS
} // namespace Math
} // namespace CF

#endif // CF_Math_LSS_EmptyLSSVector_hpp
