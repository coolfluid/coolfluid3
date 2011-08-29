// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Math_LSS_TrilinosVector_hpp
#define CF_Math_LSS_TrilinosVector_hpp

////////////////////////////////////////////////////////////////////////////////////////////

/*
#include <boost/utility.hpp>

#include "Math/LibMath.hpp"
#include "Common/MPI/CommPattern.hpp"
#include "Common/Log.hpp"
#include "Math/LSS/BlockAccumulator.hpp"
*/

#include <Epetra_Vector.h>

#include "Math/LSS/Vector.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
  @file Implementation of LSS::vector interface for Trilinos package.
  @author Tamas Banyai

  The chosen tool is epetra vector which has been implemented.
**/

////////////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Math {
namespace LSS {

////////////////////////////////////////////////////////////////////////////////////////////

class Math_API TrilinosVector : public CF::Common::Component {
public:

  /// @name CREATION, DESTRUCTION AND COMPONENT SYSTEM
  //@{

  /// pointer to this type
  typedef boost::shared_ptr<TrilinosVector> Ptr;

  /// const pointer to this type
  typedef boost::shared_ptr<TrilinosVector const> ConstPtr;

  /// name of the type
  static std::string type_name () { return "TrilinosVector"; }

  /// Accessor to solver type
  const std::string solvertype() { return "Trilinos"; }

  /// Default constructor
  TrilinosVector(const std::string& name) : Component(name) { }

  /// Setup sparsity structure
  void create(Uint nblockrows, Uint neq);

  /// Deallocate underlying data
  void destroy();


  //@} END CREATION, DESTRUCTION AND COMPONENT SYSTEM

  /// @name INDIVIDUAL ACCESS
  //@{

  /// Set value at given location in the matrix
  void set_value(const Uint irow, const Real value);

  /// Add value at given location in the matrix
  void add_value(const Uint irow, const Real value);

  /// Get value at given location in the matrix
  void get_value(const Uint irow, Real& value);

  /// Set value at given location in the matrix
  void set_value(const Uint iblockrow, const Uint ieq, const Real value);

  /// Add value at given location in the matrix
  void add_value(const Uint iblockrow, const Uint ieq, const Real value);

  /// Get value at given location in the matrix
  void get_value(const Uint iblockrow, const Uint ieq, Real& value);

  //@} END INDIVIDUAL ACCESS

  /// @name EFFICCIENT ACCESS
  //@{

  /// Set a list of values to rhs
  void set_rhs_values(const BlockAccumulator& values);

  /// Add a list of values to rhs
  void add_rhs_values(const BlockAccumulator& values);

  /// Get a list of values from rhs
  void get_rhs_values(BlockAccumulator& values);

  /// Set a list of values to sol
  void set_sol_values(const BlockAccumulator& values);

  /// Add a list of values to sol
  void add_sol_values(const BlockAccumulator& values);

  /// Get a list of values from sol
  void get_sol_values(BlockAccumulator& values);

  /// Reset Vector
  void reset(Real reset_to=0.);

  //@} END EFFICCIENT ACCESS

  /// @name MISCELLANEOUS
  //@{

  /// Print to wherever
  void print(Common::LogStream& stream);

  /// Print to wherever
  void print(std::ostream& stream);

  /// Print to file given by filename
  void print(const std::string& filename);

  /// Accessor to the state of create
  const bool is_created() { return m_is_created; };

  /// Accessor to the number of equations
  const Uint neq() { return m_neq; };

  /// Accessor to the number of block rows
  const Uint blockrow_size() { return m_blockrow_size; };

  //@} END MISCELLANEOUS

private:

  /// teuchos style smart pointer wrapping an epetra vector
  Teuchos::RCP<Epetra_Vector> m_vector;

  /// number of equations
  Uint m_neq;

  /// number of blocks
  Uint m_blockrow_size;

  /// status of the vector
  bool m_is_created;

};

////////////////////////////////////////////////////////////////////////////////////////////

} // namespace LSS
} // namespace Math
} // namespace CF

#endif // CF_Math_LSS_TrilinosVector_hpp
