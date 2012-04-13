// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Math_LSS_EmptyLSSVector_hpp
#define cf3_Math_LSS_EmptyLSSVector_hpp

////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/utility.hpp>

#include "math/LSS/LibLSS.hpp"
#include "common/PE/CommPattern.hpp"
#include "math/LSS/Vector.hpp"
#include "math/VariablesDescriptor.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
  @file EmptyLSSVector.hpp implementation of LSS::EmptyLSSVector
  @author Tamas Banyai

  EmptyLSSVector is intended to use for testing purposes only.
  It acts like a fully operational linear solver, but it does not solve and allocate any memory.

  // @todo turn it into a testing suite and throws for everything incorrect
**/

////////////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace math {
namespace LSS {

////////////////////////////////////////////////////////////////////////////////////////////

class LSS_API EmptyLSSVector : public LSS::Vector {
public:

  /// @name CREATION, DESTRUCTION AND COMPONENT SYSTEM
  //@{

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
  void create(common::PE::CommPattern& cp, Uint neq)
  {
    destroy();
    m_neq=neq;
    m_blockrow_size=cp.gid()->size();
    m_is_created=true;
  }
  
  void create_blocked(common::PE::CommPattern& cp, const VariablesDescriptor& vars)
  {
    destroy();
    m_neq=vars.size();
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
  void set_value(const Uint irow, const Real value) { cf3_assert(m_is_created); }

  /// Add value at given location in the matrix
  void add_value(const Uint irow, const Real value) { cf3_assert(m_is_created); }

  /// Get value at given location in the matrix
  void get_value(const Uint irow, Real& value) { cf3_assert(m_is_created); value=0.; }

  /// Set value at given location in the matrix
  void set_value(const Uint iblockrow, const Uint ieq, const Real value) { cf3_assert(m_is_created); }

  /// Add value at given location in the matrix
  void add_value(const Uint iblockrow, const Uint ieq, const Real value) { cf3_assert(m_is_created); }

  /// Get value at given location in the matrix
  void get_value(const Uint iblockrow, const Uint ieq, Real& value) { cf3_assert(m_is_created); value=0.; }

  //@} END INDIVIDUAL ACCESS

  /// @name EFFICCIENT ACCESS
  //@{

  /// Set a list of values to rhs
  void set_rhs_values(const BlockAccumulator& values) { cf3_assert(m_is_created); }

  /// Add a list of values to rhs
  void add_rhs_values(const BlockAccumulator& values) { cf3_assert(m_is_created); }

  /// Get a list of values from rhs
  void get_rhs_values(BlockAccumulator& values) { cf3_assert(m_is_created); values.rhs.setConstant(0.); }

  /// Set a list of values to sol
  void set_sol_values(const BlockAccumulator& values) { cf3_assert(m_is_created); }

  /// Add a list of values to sol
  void add_sol_values(const BlockAccumulator& values) { cf3_assert(m_is_created); }

  /// Get a list of values from sol
  void get_sol_values(BlockAccumulator& values) { cf3_assert(m_is_created); values.sol.setConstant(0.); }

  /// Reset Vector
  void reset(Real reset_to=0.) { cf3_assert(m_is_created); }

  /// Copies the contents out of the LSS::Vector to table.
  void get( boost::multi_array<Real, 2>& data)
  {
    cf3_assert(m_is_created);
    cf3_assert(data.shape()[0]==m_blockrow_size);
    cf3_assert(data.shape()[1]==m_neq);
    for (boost::multi_array_types::index i = 0; i < data.shape()[0]; ++i)
      for (boost::multi_array_types::index j = 0; j < data.shape()[1]; ++j)
        data[i][j]=0.;
  }

  /// Copies the contents of the table into the LSS::Vector.
  void set( boost::multi_array<Real, 2>& data)
  {
    cf3_assert(m_is_created);
    cf3_assert(data.shape()[0]==m_blockrow_size);
    cf3_assert(data.shape()[1]==m_neq);
  }

  //@} END EFFICCIENT ACCESS

  /// @name MISCELLANEOUS
  //@{

  /// Print to wherever
  void print(common::LogStream& stream) { cf3_assert(m_is_created); stream << "EmptyLSSVector::print of '" << name() << "'.\n"; }

  /// Print to wherever
  void print(std::ostream& stream) { cf3_assert(m_is_created); stream << "EmptyLSSVector::print of '" << name() << "'.\n"; }

  /// Print to file given by filename
  void print(const std::string& filename, std::ios_base::openmode mode = std::ios_base::out ) { cf3_assert(m_is_created); }
  
  void print_native(std::ostream& stream) {}

  /// Accessor to the state of create
  const bool is_created() { return m_is_created; }

  /// Accessor to the number of equations
  const Uint neq() { cf3_assert(m_is_created); return m_neq; }

  /// Accessor to the number of block rows
  const Uint blockrow_size() { cf3_assert(m_is_created); return m_blockrow_size; }

  //@} END MISCELLANEOUS

  /// @name TEST ONLY
  //@{

  /// exports the vector into big linear array
  /// @attention only for debug and utest purposes
  virtual void debug_data(std::vector<Real>& values) { cf3_assert(m_is_created); }

  //@} END TEST ONLY

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
} // namespace math
} // namespace cf3

#endif // cf3_Math_LSS_EmptyLSSVector_hpp
