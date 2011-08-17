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
  /// @todo kill al lteuchos::rcp members
  };


  //@} END CREATION, DESTRUCTION AND COMPONENT SYSTEM

  /// @name INDIVIDUAL ACCESS
  //@{

  /// Set value at given location in the matrix
  virtual void set_value(const Uint row, const Real value){};

  /// Add value at given location in the matrix
  virtual void add_value(const Uint row, const Real value){};

  /// Get value at given location in the matrix
  virtual void get_value(const Uint row, const Real& value){};

  //@} END INDIVIDUAL ACCESS

  /// @name EFFICCIENT ACCESS
  //@{

  /// Set a list of values
  virtual void set_values(const BlockAccumulator& values){};

  /// Add a list of values
  virtual void add_values(const BlockAccumulator& values){};

  /// Add a list of values
  virtual void get_values(const BlockAccumulator& values){};

  /// Reset Vector
  virtual void reset_to_zero(){};

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

  /// @name CREATION, DESTRUCTION AND COMPONENT SYSTEM
  //@{

  /// pointer to this type
  typedef boost::shared_ptr<LSSTrilinosMatrix> Ptr;

  /// const pointer to this type
  typedef boost::shared_ptr<LSSTrilinosMatrix const> ConstPtr;

  /// name of the type
  static std::string type_name () { return "LSSTrilinosMatrix"; }

  /// Default constructor
  LSSTrilinosMatrix(const std::string& name) :
    LSSMatrix(name),
    m_comm(Comm::PE::instance().communicator())
  { }

  /// Destructor.
  ~LSSTrilinosMatrix()
  {
  /// @todo kill all teuchos::rcp members
  };

  /// Setup sparsity structure
  /// should only work with local numbering (parallel computations, plus rcm could be a totally internal matter of the matrix)
  /// internal mapping should be invisible to outside (needs to reorganize to push ghost nodes)
  /// maybe 2 ctable csr style
  /// local numbering
  /// needs global numbering for communication - ??? commpattern ???
  virtual void create_sparsity(Comm::CommPattern& cp, std::vector<Uint>& node_connectivity, std::vector<Uint>& starting_indices)
  {
    // get gid
    /// @todo don't hardcode the name, rather write an accessor to it in pecommpattern
    Uint *gid=(Uint*)cp.get_child_ptr("gid")->as_ptr<CommWrapper>()->pack();

    // blockmap
    int nmyglobalelements=0;
    int maxrowentries=0;
    std::vector<int> myglobalelements(0);
    std::vector<int> rowelements(0);

    for (int i=0; i<(const int)cp.isUpdatable().size(); i++)
    {
      if (cp.isUpdatable()[i])
      {
        ++nmyglobalelements;
        myglobalelements.push_back((int)gid[i]);
        rowelements.push_back((int)(starting_indices[i+1]-starting_indices[i]));
        maxrowentries=maxrowentries<(starting_indices[i+1]-starting_indices[i])?(starting_indices[i+1]-starting_indices[i]):maxrowentries;
      }
    }
    std::vector<double>dummy_entries(maxrowentries*9,0.);
    std::vector<int>global_columns(maxrowentries);

    Epetra_BlockMap bm(-1,nmyglobalelements,&myglobalelements[0],3,0,m_comm);
    myglobalelements.resize(0);
    myglobalelements.reserve(0);

    // matrix
    m_matrix=Teuchos::rcp(new Epetra_FEVbrMatrix(Copy,bm,3));
    bm.~Epetra_BlockMap();

    for(int i=0; i<(const int)nmyglobalelements; i++)
    {
      for(int j=(const int)starting_indices[i]; j<(const int)starting_indices[i+1]; j++) global_columns[j-starting_indices[i]]=gid[node_connectivity[j]];
      m_matrix->BeginInsertGlobalValues(gid[i],rowelements[i],&global_columns[0]);
      for(int j=(const int)starting_indices[i]; j<(const int)starting_indices[i+1]; j++)
      {
        m_matrix->SubmitBlockEntry(&dummy_entries[0],0,3,3);
      }
      m_matrix->EndSubmitEntries();
    }
    m_matrix->FillComplete();
  }

  //@} END CREATION, DESTRUCTION AND COMPONENT SYSTEM

  /// @name INDIVIDUAL ACCESS
  //@{

  /// Set value at given location in the matrix
  /// Individual access results in degraded performance by a factor of 10-100 times slower, only use it for debug purposes
  virtual void set_value(const Uint row, const Uint col, const Real value)
  {
    int elemsize=m_matrix->Map().ElementSize(); // assuming constant elemsize, verified by using proper blockmap constructor
    int colblock=(int)(col/elemsize);
    int colsub=(int)(col%elemsize);
    int rowblock=(int)(row/elemsize);
    int rowsub=(int)(row%elemsize);
    Epetra_SerialDenseMatrix **val;
    int* colindices;
    int blockrowsize;
    m_matrix->ExtractMyBlockRowView(rowblock,elemsize,blockrowsize,colindices,val);
    for (int i=0; i<blockrowsize; i++)
      if (*colindices++==colblock)
      {
        val[i][0](rowsub,colsub)=value;
        break;
      }
  };

  /// Add value at given location in the matrix
  /// Individual access results in degraded performance by a factor of 10-100 times slower, only use it for debug purposes
  virtual void add_value(const Uint row, const Uint col, const Real value)
  {
    int elemsize=m_matrix->Map().ElementSize(); // assuming constant elemsize, verified by using proper blockmap constructor
    int colblock=(int)(col/elemsize);
    int colsub=(int)(col%elemsize);
    int rowblock=(int)(row/elemsize);
    int rowsub=(int)(row%elemsize);
    Epetra_SerialDenseMatrix **val;
    int* colindices;
    int blockrowsize;
    m_matrix->ExtractMyBlockRowView(rowblock,elemsize,blockrowsize,colindices,val);
    for (int i=0; i<blockrowsize; i++)
      if (*colindices++==colblock)
      {
        val[i][0](rowsub,colsub)+=value;
        break;
      }
  };

  /// Get value at given location in the matrix
  /// Individual access results in degraded performance by a factor of 10-100 times slower, only use it for debug purposes
  virtual void get_value(const Uint row, const Uint col, Real& value)
  {
    int elemsize=m_matrix->Map().ElementSize(); // assuming constant elemsize, verified by using proper blockmap constructor
    int colblock=(int)(col/elemsize);
    int colsub=(int)(col%elemsize);
    int rowblock=(int)(row/elemsize);
    int rowsub=(int)(row%elemsize);
    Epetra_SerialDenseMatrix **val;
    int* colindices;
    int blockrowsize;
    m_matrix->ExtractMyBlockRowView(rowblock,elemsize,blockrowsize,colindices,val);
    for (int i=0; i<blockrowsize; i++)
      if (*colindices++==colblock)
      {
        value=val[i][0](rowsub,colsub);
        break;
      }
  };

  //@} END INDIVIDUAL ACCESS

  /// @name EFFICCIENT ACCESS
  //@{

  /// Set a list of values
  virtual void set_values(const BlockAccumulator& values){};

  /// Add a list of values
  /// local indices
  /// eigen, templatization on top level
  virtual void add_values(const BlockAccumulator& values){};

  /// Add a list of values
  virtual void get_values(BlockAccumulator& values){};

  /// Set a row, diagonal and off-diagonals values separately (dirichlet-type boundaries)
  virtual void set_row(const Uint row, Real diagval, Real offdiagval){};

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
  virtual void reset_to_zero()
  {
    m_matrix->PutScalar(0.);
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
