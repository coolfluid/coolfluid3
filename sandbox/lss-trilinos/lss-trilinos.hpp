// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef lss_trilinos_hpp
#define lss_trilinos_hpp

#include <Epetra_MpiComm.h>
#include <Epetra_BlockMap.h>
#include <Epetra_FEVbrMatrix.h>
#include "Teuchos_GlobalMPISession.hpp"
#include "Teuchos_VerboseObject.hpp"
#include "Teuchos_XMLParameterListHelpers.hpp"
#include "Teuchos_CommandLineProcessor.hpp"

#include "lss-interface.hpp"

#include <Common/MPI/CommPattern.hpp>
#include <Common/MPI/debug.hpp>


////////////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////////////////

class Common_API LSSTrilinosVector : public LSSVector {
public:

  /// @name CREATION, DESTRUCTION AND COMPONENT SYSTEM
  //@{

  /// pointer to this type
  typedef boost::shared_ptr<LSSTrilinosVector> Ptr;

  /// const pointer to this type
  typedef boost::shared_ptr<LSSTrilinosVector const> ConstPtr;

  /// name of the type
  static std::string type_name () { return "LSSTrilinosVector"; }

  /// Default constructor
  LSSTrilinosVector(const std::string& name) : LSSVector(name) { }

  /// Destructor.
  virtual ~LSSTrilinosVector()
  {
  /// @todo kill all teuchos::rcp members
  };


  //@} END CREATION, DESTRUCTION AND COMPONENT SYSTEM

  /// @name INDIVIDUAL ACCESS
  //@{

  /// Set value at given location in the matrix
  virtual void set_value(const Uint row, const Real value){};

  /// Add value at given location in the matrix
  virtual void add_value(const Uint row, const Real value){};

  /// Get value at given location in the matrix
  virtual void get_value(const Uint row, Real& value){};

  //@} END INDIVIDUAL ACCESS

  /// @name EFFICCIENT ACCESS
  //@{

  /// Set a list of values to rhs
  virtual void set_rhs_values(const BlockAccumulator& values){};

  /// Add a list of values to rhs
  virtual void add_rhs_values(const BlockAccumulator& values){};

  /// Get a list of values from rhs
  virtual void get_rhs_values(BlockAccumulator& values){};

  /// Set a list of values to sol
  virtual void set_sol_values(const BlockAccumulator& values){};

  /// Add a list of values to sol
  virtual void add_sol_values(const BlockAccumulator& values){};

  /// Get a list of values from sol
  virtual void get_sol_values(BlockAccumulator& values){};

  /// Reset Vector
  virtual void reset(Real reset_to=0.){};

  //@} END EFFICCIENT ACCESS

  /// @name MISCELLANEOUS
  //@{

  /// Print this vector to screen
  virtual void print_to_screen(){};

  /// Print this vector to file
  virtual void print_to_file(const char* fileName){};

  //@} END MISCELLANEOUS

};

////////////////////////////////////////////////////////////////////////////////////////////

class Common_API LSSTrilinosMatrix : public LSSMatrix {
public:


  //@} END INDIVIDUAL ACCESS

  /// @name EFFICCIENT ACCESS
  //@{

  /// Set a list of values
  virtual void set_values(const BlockAccumulator& values)
  {
//    int idxs[]={0,1,2,10};
//    const int elemsize=3;
//    const int numblocks=4;
//    CF::RealMatrix val(12,12); val <<
//      11., 12., 13., 21., 22., 23., 31., 32., 33., 41., 42., 43.,
//      14., 15., 16., 24., 25., 26., 34., 35., 36., 44., 45., 46.,
//      17., 18., 19., 27., 28., 29., 37., 38., 39., 47., 48., 49.,
//      51., 52., 53., 61., 62., 63., 71., 72., 73., 81., 82., 83.,
//      54., 55., 56., 64., 65., 66., 74., 75., 76., 84., 85., 86.,
//      57., 58., 59., 67., 68., 69., 77., 78., 79., 87., 88., 89.,
//      91., 92., 93.,101.,102.,103.,111.,112.,113.,121.,122.,123.,
//      94., 95., 96.,104.,105.,106.,114.,115.,116.,124.,125.,126.,
//      97., 98., 99.,107.,108.,109.,117.,118.,119.,127.,128.,129.,
//     131.,132.,133.,141.,142.,143.,151.,152.,153.,161.,162.,163.,
//     134.,135.,136.,144.,145.,146.,154.,155.,156.,164.,165.,166.,
//     137.,138.,139.,147.,148.,149.,157.,158.,159.,167.,168.,169.;

    // issue1: its stupid to submit submit each block separately, cant submit a single blockrow at once!!!
    // issue2: so it seems that on trilinos side there is another "blockaccumulator"
    // issue3: investigate performance if matrix is View mode, the copy due to issue2 could be circumvented (then epetra only stores pointers to values)
    // issue4: investigate prformance of extracting a blockrowview and fill manually, all blocks at once in the current blockrow
    const int elemsize=m_matrix->Map().ElementSize();
    const int numblocks=values.indices.size();
    int* idxs=(int*)&values.indices[0];

    for (int irow=0; irow<numblocks; irow++)
    {
      std::cout << "# " << std::flush;
      std::cout << m_matrix->BeginReplaceMyValues(idxs[irow],numblocks,idxs);
      for (int icol=0; icol<numblocks; icol++)
        std::cout << m_matrix->SubmitBlockEntry((double*)values.mat.data()+irow*elemsize+icol*elemsize*elemsize*numblocks,numblocks*elemsize,elemsize,elemsize);
      std::cout << m_matrix->EndSubmitEntries();
      std::cout << "\n" << std::flush;
    }

  };

  /// Add a list of values
  /// local indices
  /// eigen, templatization on top level
  virtual void add_values(const BlockAccumulator& values){};

  /// Add a list of values
  virtual void get_values(BlockAccumulator& values){};

  /// Set a row, diagonal and off-diagonals values separately (dirichlet-type boundaries)
  virtual void set_row(const Uint blockrow, const Uint blockeqn, Real diagval, Real offdiagval)
  {
    int elemsize=m_matrix->Map().ElementSize(); // assuming constant elemsize, verified by using proper blockmap constructor
    Epetra_SerialDenseMatrix **val;
    int* colindices;
    int blockrowsize;
    int diagonalblock=-1;
    m_matrix->ExtractMyBlockRowView((int)blockrow,elemsize,blockrowsize,colindices,val);
    for (int i=0; i<blockrowsize; i++)
    {
      if (colindices[i]==blockrow) diagonalblock=i;
      for (int j=0; j<elemsize; j++)
        val[i][0](blockeqn,j)=offdiagval;
    }
    val[diagonalblock][0](blockeqn,blockeqn)=diagval;
  };

  /// Get a column and replace it to zero (dirichlet-type boundaries, when trying to preserve symmetry)
  /// Note that sparsity info is lost, values will contain zeros where no matrix entry is present
  virtual void get_column_and_replace_to_zero(const Uint col, LSSVector& values){};

  /// Add one line to another and tie to it via dirichlet-style (applying periodicity)
  virtual void tie_row_pairs (const Uint colto, const Uint colfrom){};

  /// Set the diagonal
  virtual void set_diagonal(const LSSVector& diag){};

  /// Add to the diagonal
  virtual void add_diagonal(const LSSVector& diag){};

  /// Get the diagonal
  virtual void get_diagonal(LSSVector& diag){};

  /// Reset Matrix
  virtual void reset(Real reset_to=0.)
  {
    m_matrix->PutScalar(reset_to);
  };

  //@} END EFFICCIENT ACCESS

  /// @name MISCELLANEOUS
  //@{

  /// Print this matrix
  virtual void print_to_screen(){
    PEProcessSortedExecute(1,
      int sumentries=0;
      int maxcol=0;
      for (int i=0; i<m_matrix->NumMyRows(); i++) {
        int rowentries;
        m_matrix->NumMyRowEntries(i,rowentries);
        double *vals=new double[rowentries];
        int *idxs=new int[rowentries];
        m_matrix->ExtractMyRowCopy(i,rowentries,rowentries,vals,idxs);
        sumentries+=rowentries;
        for (int j=0; j<rowentries; j++) {
          maxcol=maxcol<idxs[j]?idxs[j]:maxcol;
          std::cout << idxs[j] << " " << -i << " " << vals[j] << "\n" << std::flush;
        }
        delete[] vals;
        delete[] idxs;
      }
      std::cout << "# number of rows:    " << m_matrix->NumMyRows() << "\n";
      std::cout << "# number of cols:    " << maxcol+1 << "\n";
      std::cout << "# number of entries: " << sumentries << "\n";
    );
  };

  /// Print this matrix to a file
  virtual void print_to_file(const char* fileName){};

  //@} END MISCELLANEOUS

public: /// @todo for testing purposes and direct access of trilinos own debug, switch to private when done
//private:

  /// mpi universe of epetra
  Epetra_MpiComm m_comm;

  /// the actual matrix wrapped into teuchos's smart pointer
  Teuchos::RCP<Epetra_FEVbrMatrix> m_matrix;

}; // end of class LSSTrilinosMatrix

////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Common
} // namespace CF

#endif // lss_trilinos_hpp
