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

#include <Common/MPI/PECommPattern.hpp>
#include <Common/MPI/debug.hpp>


////////////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////////////////

class Common_API LSSTrilinosVector : public LSSVector {
public:

  /// @name CREATION AND DESTRUCTION
  //@{

  /// Default constructor without arguments.
  LSSTrilinosVector();

  /// Destructor.
  virtual ~LSSTrilinosVector();

  //@} END CREATION AND DESTRUCTION

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

  /// @name CREATION AND DESTRUCTION
  //@{

  /// Default constructor without arguments.
  LSSTrilinosMatrix():
    m_comm(mpi::PE::instance().communicator())
  {
  };

  /// Destructor.
  ~LSSTrilinosMatrix(){};

  /// Setup sparsity structure
  /// should only work with local numbering (parallel computations, plus rcm could be a totally internal matter of the matrix)
  /// internal mapping should be invisible to outside (needs to reorganize to push ghost nodes)
  /// maybe 2 ctable csr style
  /// local numbering
  /// needs global numbering for communication - ??? commpattern ???
  virtual void create_sparsity(PECommPattern& cp, std::vector<Uint>& node_connectivity, std::vector<Uint>& starting_indices)
  {
    // get gid
    /// @todo don't hardcode the name, rather write an accessor to it in pecommpattern
    Uint *gid=(Uint*)cp.get_child_ptr("gid")->as_ptr<PEObjectWrapper>()->pack();

    // blockmap
    int nmyglobalelements=0;
    int maxrowentries=0;
    std::vector<int> myglobalelements(0);
    std::vector<int> rowelements(0);

int nrowelems=0;

    for (int i=0; i<(const int)cp.isUpdatable().size(); i++)
    {
      if (cp.isUpdatable()[i])
      {
        ++nmyglobalelements;
        myglobalelements.push_back((int)gid[i]);
        rowelements.push_back((int)(starting_indices[i+1]-starting_indices[i]));

nrowelems+=starting_indices[i+1]-starting_indices[i];

        maxrowentries=maxrowentries<(starting_indices[i+1]-starting_indices[i])?(starting_indices[i+1]-starting_indices[i]):maxrowentries;
      }
    }
    std::vector<int> elementsizelist(nmyglobalelements,3);
    std::vector<double>dummy_entries(maxrowentries*9,0.);
    std::vector<int>global_columns(maxrowentries);

PEProcessSortedExecute(-1,std::cout << "# " << Common::mpi::PE::instance().rank() << " nrowelems: " << nrowelems << "\n" << std::flush;);

    Epetra_BlockMap bm(-1,nmyglobalelements,&myglobalelements[0],&elementsizelist[0],0,m_comm);
    myglobalelements.resize(0);
    myglobalelements.reserve(0);

//PEProcessSortedExecute(-1,bm.Print(std::cout););

    // matrix
    m_matrix=Teuchos::rcp(new Epetra_FEVbrMatrix(Copy,bm,&elementsizelist[0]));
    bm.~Epetra_BlockMap();
    elementsizelist.resize(0);
    elementsizelist.reserve(0);

int nsubmitted=0;

    for(int i=0; i<(const int)nmyglobalelements; i++)
    {
      for(int j=(const int)starting_indices[i]; j<(const int)starting_indices[i+1]; j++) global_columns[j-starting_indices[i]]=gid[node_connectivity[j]];
      m_matrix->BeginInsertGlobalValues(gid[i],rowelements[i],&global_columns[0]);
      for(int j=(const int)starting_indices[i]; j<(const int)starting_indices[i+1]; j++)
      {
        m_matrix->SubmitBlockEntry(&dummy_entries[0],0,3,3);
        nsubmitted++;
      }
      m_matrix->EndSubmitEntries();
    }
    m_matrix->FillComplete();

PEProcessSortedExecute(-1,std::cout << "# " << Common::mpi::PE::instance().rank() << " nsubmitted: " << nsubmitted << "\n" << std::flush;);

//PEProcessSortedExecute(-1,m_matrix->Print(std::cout););

  }


  //@} END CREATION AND DESTRUCTION

  /// @name INDIVIDUAL ACCESS
  //@{

  /// Set value at given location in the matrix
  virtual void set_value(const Uint col, const Uint row, const Real value){};

  /// Add value at given location in the matrix
  virtual void add_value(const Uint col, const Uint row, const Real value){};

  /// Get value at given location in the matrix
  virtual void get_value(const Uint col, const Uint row, Real& value){};

  //@} END INDIVIDUAL ACCESS

  /// @name EFFICCIENT ACCESS
  //@{

  /// Set a list of values
  virtual void set_values(const BlockAccumulator& values){};

  /// Add a list of values
  /// local ibdices
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

public: // for testing purposes and direct access of trilinos own debug
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
