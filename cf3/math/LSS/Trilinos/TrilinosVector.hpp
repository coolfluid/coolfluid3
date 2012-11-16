// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Math_LSS_TrilinosVector_hpp
#define cf3_Math_LSS_TrilinosVector_hpp

////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/lexical_cast.hpp>

#include <Epetra_MpiComm.h>
#include <Epetra_Vector.h>
#include <Teuchos_RCP.hpp>

#include "math/LSS/LibLSS.hpp"
#include "math/LSS/BlockAccumulator.hpp"
#include "math/LSS/Vector.hpp"

#include "ThyraMultiVector.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
  @file TrilinosVector.hpp Definition of LSS::vector interface for Trilinos package.
  @author Tamas Banyai

  The chosen tool is epetra vector which has been implemented.
**/

////////////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace math {
namespace LSS {

////////////////////////////////////////////////////////////////////////////////////////////

#define TRILINOS_ASSERT(a) cf3_always_assert((a)==0)
#define TRILINOS_THROW(trilinos_function_call)  { \
  int trilinos_check_error=trilinos_function_call; \
  if ((trilinos_check_error)!=0) \
    throw common::FailedAssertion(FromHere(),"Call to '" + boost::lexical_cast<std::string>(#trilinos_function_call) + "' in Trilinos dependency returned a non-zero (" +  boost::lexical_cast<std::string>(trilinos_check_error) + ") error code."); \
}

////////////////////////////////////////////////////////////////////////////////////////////

class LSS_API TrilinosVector : public LSS::Vector, public ThyraMultiVector {
public:

  /// @name CREATION, DESTRUCTION AND COMPONENT SYSTEM
  //@{

  /// name of the type
  static std::string type_name () { return "TrilinosVector"; }

  /// Accessor to solver type
  const std::string solvertype() { return "Trilinos"; }

  /// Default constructor
  TrilinosVector(const std::string& name);

  /// Setup sparsity structure
  void create(common::PE::CommPattern& cp, Uint neq);
  void create_blocked(common::PE::CommPattern& cp, const VariablesDescriptor& vars);

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

  /// Copies the contents out of the LSS::Vector to table.
  void get( boost::multi_array<Real, 2>& data);

  /// Copies the contents of the table into the LSS::Vector.
  void set( boost::multi_array<Real, 2>& data);

  //@} END EFFICCIENT ACCESS

  /// @name MISCELLANEOUS
  //@{

  /// Print to wherever
  void print(common::LogStream& stream);

  /// Print to wherever
  void print(std::ostream& stream);

  /// Print to file given by filename
  void print(const std::string& filename, std::ios_base::openmode mode = std::ios_base::out );
  
  void print_native(ostream& stream);

  /// Accessor to the state of create
  const bool is_created() { return m_is_created; };

  /// Accessor to the number of equations
  const Uint neq() { return m_neq; };

  /// Accessor to the number of block rows
  const Uint blockrow_size() { return m_blockrow_size; };

  /// Accessor to the trilinos data
  /// @attention this function is not (and should never be) part of the interface itself, only used between trilinoses
  Teuchos::RCP<Epetra_Vector> epetra_vector() { return m_vec; }

  //@} END MISCELLANEOUS

  /// @name TEST ONLY
  //@{

  /// exports the vector into big linear array
  /// @attention only for debug and utest purposes
  void debug_data(std::vector<Real>& values);

  //@} END TEST ONLY
  
  void signal_print_native(common::SignalArgs& args);
  
  Teuchos::RCP< const Thyra::MultiVectorBase< Real > > thyra_vector ( const Teuchos::RCP< const Thyra::VectorSpaceBase< Real > >& space ) const;
  Teuchos::RCP< Thyra::MultiVectorBase< Real > > thyra_vector ( const Teuchos::RCP< const Thyra::VectorSpaceBase< Real > >& space );
  
private:

  /// teuchos style smart pointer wrapping an epetra vector
  Teuchos::RCP<Epetra_Vector> m_vec;

  /// epetra mpi environment
  Epetra_MpiComm m_comm;

  /// number of equations
  Uint m_neq;

  /// number of blocks
  Uint m_blockrow_size;

  /// status of the vector
  bool m_is_created;

  /// mapper array, maps from process local numbering to matrix local numbering (because ghost nodes need to be ordered to the back)
  std::vector<int> m_p2m;

  /// a helper array used in set/add/get_values to avoid frequent new+free combo
  std::vector<int> m_converted_indices;

};

////////////////////////////////////////////////////////////////////////////////////////////

} // namespace LSS
} // namespace math
} // namespace cf3

#endif // cf3_Math_LSS_TrilinosVector_hpp
